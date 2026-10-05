#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>

namespace JSB
{

class XmlNode;
using XmlNodePtr = std::shared_ptr<XmlNode>;

class XmlNode
{
public:
    std::string name;
    std::string text;
    std::unordered_map<std::string, std::string> attributes;
    std::vector<XmlNodePtr> children;

    std::string attribute(const std::string& key, const std::string& defVal = "") const
    {
        for (const auto& kv : attributes)
        {
            if (equal_case_insensitive(kv.first, key))
                return kv.second;
        }
        return defVal;
    }

    double attribute_double(const std::string& key, double defVal = 0.0) const
    {
        std::string s = attribute(key);
        if (s.empty()) return defVal;
        try { return std::stod(s); } catch (...) { return defVal; }
    }

    XmlNodePtr child(const std::string& tag) const
    {
        for (const auto& ch : children)
        {
            if (equal_case_insensitive(ch->name, tag))
                return ch;
        }
        return nullptr;
    }

    std::vector<XmlNodePtr> get_children(const std::string& tag = "") const
    {
        if (tag.empty())
            return children;
        std::vector<XmlNodePtr> res;
        for (const auto& ch : children)
        {
            if (equal_case_insensitive(ch->name, tag))
                res.push_back(ch);
        }
        return res;
    }

    std::string text_trimmed() const
    {
        return trim(text);
    }

    double text_double(double defVal = 0.0) const
    {
        std::string t = text_trimmed();
        if (t.empty()) return defVal;
        try { return std::stod(t); } catch (...) { return defVal; }
    }

    int text_int(int defVal = 0) const
    {
        std::string t = text_trimmed();
        if (t.empty()) return defVal;
        try { return std::stoi(t); } catch (...) { return defVal; }
    }

    static std::string trim(const std::string& s)
    {
        size_t start = 0;
        while (start < s.size() && (std::isspace(static_cast<unsigned char>(s[start])) || s[start] == '\r' || s[start] == '\n'))
            ++start;
        size_t end = s.size();
        while (end > start && (std::isspace(static_cast<unsigned char>(s[end - 1])) || s[end - 1] == '\r' || s[end - 1] == '\n'))
            --end;
        return s.substr(start, end - start);
    }

    static bool equal_case_insensitive(const std::string& a, const std::string& b)
    {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
                return false;
        }
        return true;
    }
};

class XmlDocument
{
public:
    XmlNodePtr root;

    bool load_file(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "[XmlDocument] Error: Could not open file: " << filePath << std::endl;
            return false;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return parse(ss.str());
    }

    bool parse(const std::string& xml)
    {
        size_t pos = 0;
        root = std::make_shared<XmlNode>();
        root->name = "__ROOT__";

        std::vector<XmlNodePtr> stack;
        stack.push_back(root);

        while (pos < xml.size())
        {
            // Skip comments <!-- ... -->
            if (pos + 4 <= xml.size() && xml.substr(pos, 4) == "<!--")
            {
                size_t endComment = xml.find("-->", pos + 4);
                if (endComment == std::string::npos) break;
                pos = endComment + 3;
                continue;
            }

            // Skip processing instructions <? ... ?> and <! ... >
            if (pos + 2 <= xml.size() && xml[pos] == '<' && (xml[pos + 1] == '?' || xml[pos + 1] == '!'))
            {
                size_t endTag = xml.find('>', pos + 2);
                if (endTag == std::string::npos) break;
                pos = endTag + 1;
                continue;
            }

            // Tag start
            if (xml[pos] == '<')
            {
                size_t endTag = xml.find('>', pos);
                if (endTag == std::string::npos) break;

                // Closing tag </tag>
                if (pos + 1 < xml.size() && xml[pos + 1] == '/')
                {
                    std::string closeName = XmlNode::trim(xml.substr(pos + 2, endTag - (pos + 2)));
                    if (stack.size() > 1)
                    {
                        if (XmlNode::equal_case_insensitive(stack.back()->name, closeName))
                        {
                            stack.pop_back();
                        }
                        else
                        {
                            // Try to find matching parent tag
                            for (int i = static_cast<int>(stack.size()) - 1; i >= 1; --i)
                            {
                                if (XmlNode::equal_case_insensitive(stack[i]->name, closeName))
                                {
                                    while (stack.size() > static_cast<size_t>(i))
                                        stack.pop_back();
                                    break;
                                }
                            }
                        }
                    }
                    pos = endTag + 1;
                    continue;
                }

                // Opening or self-closing tag
                bool selfClosing = false;
                size_t tagContentEnd = endTag;
                if (tagContentEnd > pos + 1 && xml[tagContentEnd - 1] == '/')
                {
                    selfClosing = true;
                    --tagContentEnd;
                }

                std::string tagContent = xml.substr(pos + 1, tagContentEnd - (pos + 1));
                auto newNode = parse_tag(tagContent);

                if (newNode)
                {
                    stack.back()->children.push_back(newNode);
                    if (!selfClosing)
                    {
                        stack.push_back(newNode);
                    }
                }

                pos = endTag + 1;
                continue;
            }

            // Text between tags
            size_t nextTag = xml.find('<', pos);
            if (nextTag == std::string::npos)
                nextTag = xml.size();

            std::string textContent = xml.substr(pos, nextTag - pos);
            if (!stack.empty())
            {
                stack.back()->text += textContent;
            }
            pos = nextTag;
        }

        // Return first actual child of __ROOT__ as the document root
        if (!root->children.empty())
        {
            root = root->children[0];
            return true;
        }

        return false;
    }

private:
    XmlNodePtr parse_tag(const std::string& content)
    {
        auto node = std::make_shared<XmlNode>();
        size_t p = 0;
        // Skip leading whitespace
        while (p < content.size() && std::isspace(static_cast<unsigned char>(content[p]))) ++p;
        // Read tag name
        size_t nameStart = p;
        while (p < content.size() && !std::isspace(static_cast<unsigned char>(content[p])) && content[p] != '/')
            ++p;
        node->name = content.substr(nameStart, p - nameStart);

        // Read attributes
        while (p < content.size())
        {
            while (p < content.size() && std::isspace(static_cast<unsigned char>(content[p]))) ++p;
            if (p >= content.size() || content[p] == '/') break;

            size_t attrNameStart = p;
            while (p < content.size() && content[p] != '=' && !std::isspace(static_cast<unsigned char>(content[p])))
                ++p;
            std::string attrName = content.substr(attrNameStart, p - attrNameStart);

            while (p < content.size() && (std::isspace(static_cast<unsigned char>(content[p])) || content[p] == '='))
                ++p;

            std::string attrVal;
            if (p < content.size() && (content[p] == '"' || content[p] == '\''))
            {
                char quote = content[p++];
                size_t valStart = p;
                while (p < content.size() && content[p] != quote) ++p;
                attrVal = content.substr(valStart, p - valStart);
                if (p < content.size()) ++p; // skip closing quote
            }
            else
            {
                size_t valStart = p;
                while (p < content.size() && !std::isspace(static_cast<unsigned char>(content[p]))) ++p;
                attrVal = content.substr(valStart, p - valStart);
            }

            if (!attrName.empty())
                node->attributes[attrName] = attrVal;
        }

        return node;
    }
};

} // namespace JSB
