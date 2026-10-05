#pragma once
#include "XmlParser.hpp"
#include "GuidGenerator.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

namespace JSB
{

class MetricsParser
{
public:
    static bool parse_and_write(const XmlNodePtr& metricsNode, const std::string& aircraftName, const std::string& outputPath)
    {
        if (!metricsNode)
        {
            std::cerr << "[MetricsParser] No <metrics> node found!" << std::endl;
            return false;
        }

        double wingArea = 0.0;
        double wingSpan = 0.0;
        double cbar = 0.0;
        double wingIncidence = 0.0;
        double hTailArea = 0.0;
        double vTailArea = 0.0;
        double hTailArm = 0.0;
        double vTailArm = 0.0;
        double lbarh = 0.0;
        double lbarv = 0.0;
        double vbarh = 0.0;
        double vbarv = 0.0;

        if (auto n = metricsNode->child("wingarea")) wingArea = n->text_double();
        if (auto n = metricsNode->child("wingspan")) wingSpan = n->text_double();
        if (auto n = metricsNode->child("chord")) cbar = n->text_double();
        if (auto n = metricsNode->child("wing_incidence")) wingIncidence = n->text_double();
        if (auto n = metricsNode->child("htailarea")) hTailArea = n->text_double();
        if (auto n = metricsNode->child("vtailarea")) vTailArea = n->text_double();
        if (auto n = metricsNode->child("htailarm")) hTailArm = n->text_double();
        if (auto n = metricsNode->child("vtailarm")) vTailArm = n->text_double();
        if (auto n = metricsNode->child("lbarh")) lbarh = n->text_double();
        if (auto n = metricsNode->child("lbarv")) lbarv = n->text_double();
        if (auto n = metricsNode->child("vbarh")) vbarh = n->text_double();
        if (auto n = metricsNode->child("vbarv")) vbarv = n->text_double();

        // Estimate typical aircraft tail dimensions if omitted in XML
        if (hTailArea == 0.0 && wingArea > 0.0) hTailArea = wingArea * 0.233;
        if (vTailArea == 0.0 && wingArea > 0.0) vTailArea = wingArea * 0.100;
        if (hTailArm == 0.0 && wingSpan > 0.0)  hTailArm = wingSpan * 0.410;
        if (vTailArm == 0.0 && wingSpan > 0.0)  vTailArm = hTailArm;
        if (cbar > 0.0)
        {
            if (lbarh == 0.0 && hTailArm > 0.0) lbarh = hTailArm / cbar;
            if (lbarv == 0.0 && vTailArm > 0.0) lbarv = vTailArm / cbar;
        }
        if (wingArea > 0.0 && cbar > 0.0 && hTailArea > 0.0 && hTailArm > 0.0)
        {
            if (vbarh == 0.0) vbarh = (hTailArea * hTailArm) / (wingArea * cbar);
        }
        if (wingArea > 0.0 && wingSpan > 0.0 && vTailArea > 0.0 && vTailArm > 0.0)
        {
            if (vbarv == 0.0) vbarv = (vTailArea * vTailArm) / (wingArea * wingSpan);
        }

        std::stringstream ss;
        ss << "FGAircraftConfig {\n";
        ss << " AircraftName \"" << aircraftName << "\"\n";
        ss << " WingArea " << wingArea << "\n";
        ss << " WingSpan " << wingSpan << "\n";
        ss << " cbar " << cbar << "\n";
        ss << " WingIncidence " << wingIncidence << "\n";
        ss << " HTailArea " << hTailArea << "\n";
        ss << " VTailArea " << vTailArea << "\n";
        ss << " HTailArm " << hTailArm << "\n";
        ss << " VTailArm " << vTailArm << "\n";
        if (lbarh != 0.0) ss << " lbarh " << lbarh << "\n";
        if (lbarv != 0.0) ss << " lbarv " << lbarv << "\n";
        if (vbarh != 0.0) ss << " vbarh " << vbarh << "\n";
        if (vbarv != 0.0) ss << " vbarv " << vbarv << "\n";
        ss << "}\n";

        std::ofstream outFile(outputPath);
        if (!outFile.is_open())
        {
            std::cerr << "[MetricsParser] Failed to open output file: " << outputPath << std::endl;
            return false;
        }

        outFile << ss.str();
        std::cout << "[MetricsParser] Successfully wrote: " << outputPath << std::endl;

        // Generate .meta file for Enfusion
        std::string metaPath = outputPath + ".meta";
        std::ofstream metaFile(metaPath);
        if (metaFile.is_open())
        {
            metaFile << "MetaFileClass {\n";
            metaFile << " Name \"" << GuidGenerator::generate() << "\"\n";
            metaFile << " Configurations {\n";
            metaFile << "  CONFResourceClass PC {\n  }\n";
            metaFile << "  CONFResourceClass XBOX_ONE : PC {\n  }\n";
            metaFile << "  CONFResourceClass XBOX_SERIES : PC {\n  }\n";
            metaFile << "  CONFResourceClass PS4 : PC {\n  }\n";
            metaFile << "  CONFResourceClass PS5 : PC {\n  }\n";
            metaFile << "  CONFResourceClass HEADLESS : PC {\n  }\n";
            metaFile << " }\n";
            metaFile << "}\n";
            std::cout << "[MetricsParser] Successfully wrote: " << metaPath << std::endl;
        }

        return true;
    }
};

} // namespace JSB
