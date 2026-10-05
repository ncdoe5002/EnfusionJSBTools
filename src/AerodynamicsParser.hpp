#pragma once
#include "XmlParser.hpp"
#include "GuidGenerator.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace JSB
{

class AerodynamicsParser
{
public:
    static bool parse_and_write(const XmlNodePtr& aeroNode, const std::string& aircraftName, const std::string& outputPath)
    {
        if (!aeroNode)
        {
            std::cerr << "[AerodynamicsParser] No <aerodynamics> node found!" << std::endl;
            return false;
        }

        std::stringstream ss;
        ss << "AerodynamicsConfig {\n";

        // Global functions (functions directly under <aerodynamics> not in an <axis>)
        auto directFunctions = aeroNode->get_children("function");
        if (!directFunctions.empty())
        {
            ss << " globalFunctions {\n";
            for (const auto& func : directFunctions)
            {
                write_function(ss, func, "  ");
            }
            ss << " }\n";
        }

        // Axis functions (<axis name="LIFT|DRAG|SIDE|ROLL|PITCH|YAW">)
        auto axes = aeroNode->get_children("axis");
        if (!axes.empty())
        {
            ss << " AeroFunctions {\n";
            for (const auto& axis : axes)
            {
                std::string axisName = axis->attribute("name", "GROUP");
                std::transform(axisName.begin(), axisName.end(), axisName.begin(), ::toupper);

                ss << "  AeroFunctionArray \"" << GuidGenerator::generate() << "\" {\n";
                ss << "   functionGroup \"" << axisName << "\"\n";
                ss << "   functions {\n";

                auto funcs = axis->get_children("function");
                for (const auto& func : funcs)
                {
                    write_function(ss, func, "    ");
                }

                ss << "   }\n";
                ss << "  }\n";
            }
            ss << " }\n";
        }

        ss << "}\n";

        std::ofstream outFile(outputPath);
        if (!outFile.is_open())
        {
            std::cerr << "[AerodynamicsParser] Failed to open output file: " << outputPath << std::endl;
            return false;
        }

        outFile << ss.str();
        std::cout << "[AerodynamicsParser] Successfully wrote: " << outputPath << std::endl;

        // Also generate .meta file
        generate_meta(outputPath);

        return true;
    }

private:
    static void generate_meta(const std::string& confPath)
    {
        std::string metaPath = confPath + ".meta";
        std::ofstream metaFile(metaPath);
        if (metaFile.is_open())
        {
            std::string fileGuid = GuidGenerator::generate();
            metaFile << "MetaFileClass {\n";
            metaFile << " Name \"" << fileGuid << "\"\n";
            metaFile << " Configurations {\n";
            metaFile << "  CONFResourceClass PC {\n  }\n";
            metaFile << "  CONFResourceClass XBOX_ONE : PC {\n  }\n";
            metaFile << "  CONFResourceClass XBOX_SERIES : PC {\n  }\n";
            metaFile << "  CONFResourceClass PS4 : PC {\n  }\n";
            metaFile << "  CONFResourceClass PS5 : PC {\n  }\n";
            metaFile << "  CONFResourceClass HEADLESS : PC {\n  }\n";
            metaFile << " }\n";
            metaFile << "}\n";
        }
    }

    static void write_function(std::stringstream& ss, const XmlNodePtr& funcNode, const std::string& indent)
    {
        std::string funcName = funcNode->attribute("name");
        // Simplify name if it has slash prefixes like aero/force/
        if (funcName.find('/') != std::string::npos)
        {
            size_t lastSlash = funcName.find_last_of('/');
            funcName = funcName.substr(lastSlash + 1);
        }

        ss << indent << "FGFunction \"" << GuidGenerator::generate() << "\" {\n";
        if (!funcName.empty())
            ss << indent << " name \"" << funcName << "\"\n";

        for (const auto& child : funcNode->children)
        {
            std::string tag = child->name;
            std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
            if (tag == "description") continue;

            ss << indent << " parameter ";
            write_parameter_node(ss, child, indent + " ");
            break;
        }

        ss << indent << "}\n";
    }

    static void write_parameter_node(std::stringstream& ss, const XmlNodePtr& node, const std::string& indent)
    {
        std::string tag = node->name;
        std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

        if (tag == "product")
        {
            ss << "FGParamMul \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " arguments {\n";
            for (const auto& ch : node->children)
            {
                if (ch->name == "description") continue;
                ss << indent << "  ";
                write_parameter_node(ss, ch, indent + "  ");
            }
            ss << indent << " }\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "sum")
        {
            ss << "FGParamAdd \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " arguments {\n";
            for (const auto& ch : node->children)
            {
                if (ch->name == "description") continue;
                ss << indent << "  ";
                write_parameter_node(ss, ch, indent + "  ");
            }
            ss << indent << " }\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "difference")
        {
            ss << "FGParamSub \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " arguments {\n";
            for (const auto& ch : node->children)
            {
                if (ch->name == "description") continue;
                ss << indent << "  ";
                write_parameter_node(ss, ch, indent + "  ");
            }
            ss << indent << " }\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "quotient")
        {
            ss << "FGParamDiv \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " arguments {\n";
            for (const auto& ch : node->children)
            {
                if (ch->name == "description") continue;
                ss << indent << "  ";
                write_parameter_node(ss, ch, indent + "  ");
            }
            ss << indent << " }\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "property")
        {
            ss << "FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " propName \"" << node->text_trimmed() << "\"\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "value")
        {
            ss << "FGParameterValue \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " Value " << node->text_double() << "\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
        else if (tag == "table")
        {
            write_table(ss, node, indent);
        }
        else
        {
            ss << "FGParameterValue \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << " Value " << node->text_double() << "\n";
            ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
        }
    }

    static void write_table(std::stringstream& ss, const XmlNodePtr& tableNode, const std::string& indent)
    {
        ss << "FGParameterTable \"" << GuidGenerator::generate() << "\" {\n";

        auto vars = tableNode->get_children("independentVar");
        std::string varX, varY;
        if (!vars.empty()) varX = vars[0]->text_trimmed();
        if (vars.size() > 1) varY = vars[1]->text_trimmed();

        if (!varX.empty())
        {
            ss << indent << " input FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << "  propName \"" << varX << "\"\n";
            ss << indent << " }\n";
        }
        if (!varY.empty())
        {
            ss << indent << " inputY FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << indent << "  propName \"" << varY << "\"\n";
            ss << indent << " }\n";
        }

        auto dataNode = tableNode->child("tableData");
        if (dataNode)
        {
            parse_and_write_table_data(ss, dataNode->text, vars.size(), indent);
        }

        ss << indent.substr(0, indent.size() >= 1 ? indent.size() - 1 : 0) << "}\n";
    }

    static std::vector<double> extract_numbers(const std::string& rawLine)
    {
        std::vector<double> nums;
        std::string cleaned;
        size_t p = 0;
        while (p < rawLine.size())
        {
            if (p + 4 <= rawLine.size() && rawLine.substr(p, 4) == "<!--")
            {
                size_t endC = rawLine.find("-->", p + 4);
                if (endC == std::string::npos) break;
                p = endC + 3;
                continue;
            }
            cleaned += rawLine[p++];
        }

        std::stringstream ss(cleaned);
        std::string token;
        while (ss >> token)
        {
            try
            {
                size_t idx = 0;
                double val = std::stod(token, &idx);
                if (idx > 0)
                    nums.push_back(val);
            }
            catch (...) {}
        }
        return nums;
    }

    static void parse_and_write_table_data(std::stringstream& ss, const std::string& text, size_t dim, const std::string& indent)
    {
        std::stringstream dataStream(text);
        std::vector<std::vector<double>> rows;
        std::string line;
        while (std::getline(dataStream, line))
        {
            auto nums = extract_numbers(line);
            if (!nums.empty())
                rows.push_back(nums);
        }

        ss << indent << " table SCR_LookupTable \"" << GuidGenerator::generate() << "\" {\n";

        if (dim <= 1 || rows.size() <= 1)
        {
            // 1D Table
            std::vector<double> xVals, yVals;
            for (const auto& r : rows)
            {
                if (r.size() >= 2)
                {
                    xVals.push_back(r[0]);
                    yVals.push_back(r[1]);
                }
            }

            ss << indent << "  xAxis {\n";
            for (double x : xVals)
                ss << indent << "   " << x << "\n";
            ss << indent << "  }\n";

            ss << indent << "  values1D {\n";
            for (double y : yVals)
                ss << indent << "   " << y << "\n";
            ss << indent << "  }\n";
        }
        else
        {
            // 2D Table
            const auto& colHeaders = rows[0];
            ss << indent << "  xAxis {\n";
            for (size_t i = 1; i < rows.size(); ++i)
            {
                if (!rows[i].empty())
                    ss << indent << "   " << rows[i][0] << "\n";
            }
            ss << indent << "  }\n";

            ss << indent << "  yAxis {\n";
            for (double c : colHeaders)
                ss << indent << "   " << c << "\n";
            ss << indent << "  }\n";

            ss << indent << "  values2D {\n";
            for (size_t i = 1; i < rows.size(); ++i)
            {
                const auto& r = rows[i];
                ss << indent << "   SCR_LookupRow2 \"" << GuidGenerator::generate() << "\" {\n";
                for (size_t colIdx = 1; colIdx < r.size() && colIdx <= 4; ++colIdx)
                {
                    ss << indent << "    r" << colIdx << " " << r[colIdx] << "\n";
                }
                ss << indent << "   }\n";
            }
            ss << indent << "  }\n";
        }

        ss << indent << " }\n";
    }
};

} // namespace JSB
