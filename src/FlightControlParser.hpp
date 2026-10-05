#pragma once
#include "XmlParser.hpp"
#include "GuidGenerator.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>

namespace JSB
{

class FlightControlParser
{
public:
    static bool parse_and_write(const XmlNodePtr& fcsNode, const std::string& aircraftName, const std::string& outputPath)
    {
        if (!fcsNode)
        {
            std::cerr << "[FlightControlParser] No <flight_control> node found!" << std::endl;
            return false;
        }

        std::string configName = fcsNode->attribute("name");
        if (configName.empty())
            configName = aircraftName + "-FlightControls-Configuration";

        std::stringstream ss;
        ss << "FGFCSConfig {\n";
        ss << " configName \"" << configName << "\"\n";
        ss << " Channels {\n";

        auto channels = fcsNode->get_children("channel");
        for (const auto& ch : channels)
        {
            write_channel(ss, ch);
        }

        ss << " }\n";
        ss << "}\n";

        std::ofstream outFile(outputPath);
        if (!outFile.is_open())
        {
            std::cerr << "[FlightControlParser] Failed to open output file: " << outputPath << std::endl;
            return false;
        }

        outFile << ss.str();
        std::cout << "[FlightControlParser] Successfully wrote: " << outputPath << std::endl;
        return true;
    }

private:
    static void write_channel(std::stringstream& ss, const XmlNodePtr& ch)
    {
        std::string chName = ch->attribute("name", "CHANNEL");
        ss << "  FGFCSChannel \"" << GuidGenerator::generate() << "\" {\n";
        ss << "   name \"" << chName << "\"\n";
        ss << "   FCSComponents {\n";

        for (const auto& comp : ch->children)
        {
            std::string tag = comp->name;
            std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);

            if (tag == "summer")
            {
                write_summer(ss, comp);
            }
            else if (tag == "aerosurface_scale")
            {
                write_aerosurface_scale(ss, comp);
            }
            else if (tag == "pure_gain")
            {
                write_pure_gain(ss, comp);
            }
            else if (tag == "kinematic")
            {
                write_kinematic(ss, comp);
            }
            else if (tag == "switch")
            {
                write_switch(ss, comp);
            }
            else if (tag == "fcs_function")
            {
                write_fcs_function(ss, comp);
            }
        }

        ss << "   }\n";
        ss << "   ExecRate 1\n";
        ss << "   isOn 1\n";
        ss << "  }\n";
    }

    static std::string format_default_output(const std::string& name)
    {
        if (name.empty()) return "";
        std::string prop = name;
        std::transform(prop.begin(), prop.end(), prop.begin(), [](unsigned char c) {
            if (c == ' ' || c == '_') return '-';
            return (char)std::tolower(c);
        });
        if (prop.find("fcs/") != 0)
        {
            prop = "fcs/" + prop;
        }
        return prop;
    }

    static void write_outputs(std::stringstream& ss, const XmlNodePtr& comp)
    {
        auto outputs = comp->get_children("output");
        ss << "     OutputNodes {\n";
        bool hasOutput = false;
        for (const auto& out : outputs)
        {
            std::string text = out->text_trimmed();
            if (!text.empty())
            {
                ss << "      \"" << text << "\"\n";
                hasOutput = true;
            }
        }
        if (!hasOutput)
        {
            std::string defProp = format_default_output(comp->attribute("name"));
            if (!defProp.empty())
            {
                ss << "      \"" << defProp << "\"\n";
            }
        }
        ss << "     }\n";
    }

    static void write_summer(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGSummer \"" << GuidGenerator::generate() << "\" {\n";

        std::string compName = comp->attribute("name");
        if (!compName.empty())
            ss << "     Name \"" << compName << "\"\n";

        // Outputs
        write_outputs(ss, comp);

        // Clip
        bool hasClip = false;
        double clipMin = -1.0, clipMax = 1.0;
        if (auto clipNode = comp->child("clipto"))
        {
            hasClip = true;
            if (auto mn = clipNode->child("min")) clipMin = mn->text_double(-1.0);
            if (auto mx = clipNode->child("max")) clipMax = mx->text_double(1.0);
        }
        else if (auto clipNode2 = comp->child("clip"))
        {
            hasClip = true;
            if (auto mn = clipNode2->child("min")) clipMin = mn->text_double(-1.0);
            if (auto mx = clipNode2->child("max")) clipMax = mx->text_double(1.0);
        }

        if (hasClip)
        {
            ss << "     ClipMin " << clipMin << "\n";
            ss << "     ClipMax " << clipMax << "\n";
            ss << "     clip 1\n";
        }

        // Inputs
        auto inputs = comp->get_children("input");
        ss << "     InputNodes {\n";
        for (const auto& in : inputs)
        {
            ss << "      FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << "       propName \"" << in->text_trimmed() << "\"\n";
            ss << "      }\n";
        }
        ss << "     }\n";

        ss << "    }\n";
    }

    static void write_aerosurface_scale(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGGain \"" << GuidGenerator::generate() << "\" {\n";

        std::string compName = comp->attribute("name");
        if (!compName.empty())
            ss << "     Name \"" << compName << "\"\n";

        // Outputs
        write_outputs(ss, comp);

        // Inputs
        auto inputs = comp->get_children("input");
        ss << "     InputNodes {\n";
        for (const auto& in : inputs)
        {
            ss << "      FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << "       propName \"" << in->text_trimmed() << "\"\n";
            ss << "      }\n";
        }
        ss << "     }\n";

        ss << "     Type \"AEROSURFACE_SCALE\"\n";

        double gain = 1.0;
        if (auto g = comp->child("gain")) gain = g->text_double(1.0);
        ss << "     Gain " << gain << "\n";

        double inMin = -1.0, inMax = 1.0;
        if (auto domain = comp->child("domain"))
        {
            if (auto mn = domain->child("min")) inMin = mn->text_double(-1.0);
            if (auto mx = domain->child("max")) inMax = mx->text_double(1.0);
        }
        ss << "     InMin " << inMin << "\n";
        ss << "     InMax " << inMax << "\n";

        double outMin = -1.0, outMax = 1.0;
        if (auto range = comp->child("range"))
        {
            if (auto mn = range->child("min")) outMin = mn->text_double(-1.0);
            if (auto mx = range->child("max")) outMax = mx->text_double(1.0);
        }
        ss << "     OutMin " << outMin << "\n";
        ss << "     OutMax " << outMax << "\n";

        ss << "     ZeroCentered 1\n";
        ss << "     clip 1\n";

        ss << "    }\n";
    }

    static void write_pure_gain(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGGain \"" << GuidGenerator::generate() << "\" {\n";

        std::string compName = comp->attribute("name");
        if (!compName.empty())
            ss << "     Name \"" << compName << "\"\n";

        write_outputs(ss, comp);

        auto inputs = comp->get_children("input");
        ss << "     InputNodes {\n";
        for (const auto& in : inputs)
        {
            ss << "      FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << "       propName \"" << in->text_trimmed() << "\"\n";
            ss << "      }\n";
        }
        ss << "     }\n";

        ss << "     Type \"PURE_GAIN\"\n";
        double gain = 1.0;
        if (auto g = comp->child("gain")) gain = g->text_double(1.0);
        ss << "     Gain " << gain << "\n";

        ss << "    }\n";
    }

    static void write_kinematic(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGKinemat \"" << GuidGenerator::generate() << "\" {\n";

        std::string compName = comp->attribute("name");
        if (!compName.empty())
        {
            ss << "     Name \"" << compName << "\"\n";
        }

        write_outputs(ss, comp);

        auto inputs = comp->get_children("input");
        ss << "     InputNodes {\n";
        for (const auto& in : inputs)
        {
            ss << "      FGParameterProperty \"" << GuidGenerator::generate() << "\" {\n";
            ss << "       propName \"" << in->text_trimmed() << "\"\n";
            ss << "      }\n";
        }
        ss << "     }\n";

        ss << "     clip 1\n";

        // Parse traverse settings
        auto traverseNode = comp->child("traverse");
        if (traverseNode)
        {
            auto settings = traverseNode->get_children("setting");
            if (!settings.empty())
            {
                ss << "     traverses {\n";
                for (const auto& s : settings)
                {
                    double pos = 0.0;
                    double time = 0.0;
                    if (auto p = s->child("position")) pos = p->text_double(0.0);
                    if (auto t = s->child("time")) time = t->text_double(0.0);

                    ss << "      FGTraverse \"" << GuidGenerator::generate() << "\" {\n";
                    ss << "       pos " << pos << "\n";
                    ss << "       time " << time << "\n";
                    ss << "      }\n";
                }
                ss << "     }\n";
            }
        }

        ss << "     DoScale 1\n";
        ss << "    }\n";
    }

    static void write_switch(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGSwitch \"" << GuidGenerator::generate() << "\" {\n";
        std::string compName = comp->attribute("name");
        if (!compName.empty()) ss << "     Name \"" << compName << "\"\n";
        write_outputs(ss, comp);
        ss << "    }\n";
    }

    static void write_fcs_function(std::stringstream& ss, const XmlNodePtr& comp)
    {
        ss << "    FGFCSFunction \"" << GuidGenerator::generate() << "\" {\n";
        std::string compName = comp->attribute("name");
        if (!compName.empty()) ss << "     Name \"" << compName << "\"\n";
        write_outputs(ss, comp);
        ss << "    }\n";
    }
};

} // namespace JSB

