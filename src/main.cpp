#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")
#include "XmlParser.hpp"
#include "MetricsParser.hpp"
#include "FlightControlParser.hpp"
#include "AerodynamicsParser.hpp"

namespace fs = std::filesystem;

void print_usage(const char* progName)
{
    std::cout << "====================================================\n";
    std::cout << " JSBSim to Enfusion Configs XML Parser\n";
    std::cout << "====================================================\n";
    std::cout << "Usage:\n";
    std::cout << "  " << progName << " --xml <target_xml> --out <output_dir> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --xml <path>            Path to input JSBSim XML file\n";
    std::cout << "  --out <dir>             Output directory for Enfusion .conf files\n";
    std::cout << "  --prefix <name>         Output file prefix (default: Aircraft)\n";
    std::cout << "  --aircraft <name>       Aircraft display name\n";
    std::cout << "  --parse-aero <0|1>      Parse <aerodynamics> (default: 1)\n";
    std::cout << "  --parse-fcs <0|1>       Parse <flight_control> (default: 1)\n";
    std::cout << "  --parse-metrics <0|1>   Parse <metrics> (default: 1)\n";
    std::cout << "  --help                  Show this help message\n";
    std::cout << "====================================================\n";
}

bool ensure_directory_exists(const std::string& pathStr)
{
    std::error_code ec;
    if (fs::exists(pathStr, ec))
        return true;

    fs::create_directories(pathStr, ec);
    if (fs::exists(pathStr, ec))
        return true;

    // Fallback: Windows Shell API
    std::wstring wpath(pathStr.begin(), pathStr.end());
    int res = SHCreateDirectoryExW(NULL, wpath.c_str(), NULL);
    return (res == ERROR_SUCCESS || res == ERROR_ALREADY_EXISTS || res == ERROR_FILE_EXISTS || fs::exists(pathStr, ec));
}

int main(int argc, char* argv[])
{
    std::ofstream logFile("C:\\Arma modding\\JSBSimEnfusionUtils\\parser.log", std::ios::app);
    logFile << "====================================================" << std::endl;
    logFile << "[Parser Execution Start]" << std::endl;
    for (int i = 0; i < argc; ++i)
        logFile << "  arg[" << i << "]: " << argv[i] << std::endl;
    logFile.flush();

    if (argc < 2)
    {
        print_usage(argv[0]);
        logFile << "[Error] Insufficient arguments (argc < 2)" << std::endl;
        return 1;
    }

    std::string xmlPath;
    std::string outDir = ".";
    std::string prefix = "Aircraft";
    std::string aircraftName;
    bool parseAero = true;
    bool parseFCS = true;
    bool parseMetrics = true;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--xml" && i + 1 < argc)
        {
            xmlPath = argv[++i];
        }
        else if (arg == "--out" && i + 1 < argc)
        {
            outDir = argv[++i];
        }
        else if (arg == "--prefix" && i + 1 < argc)
        {
            prefix = argv[++i];
        }
        else if (arg == "--aircraft" && i + 1 < argc)
        {
            aircraftName = argv[++i];
        }
        else if (arg == "--parse-aero" && i + 1 < argc)
        {
            std::string val = argv[++i];
            parseAero = (val != "0" && val != "false");
        }
        else if (arg == "--parse-fcs" && i + 1 < argc)
        {
            std::string val = argv[++i];
            parseFCS = (val != "0" && val != "false");
        }
        else if (arg == "--parse-metrics" && i + 1 < argc)
        {
            std::string val = argv[++i];
            parseMetrics = (val != "0" && val != "false");
        }
        else if (arg == "--help" || arg == "-h")
        {
            print_usage(argv[0]);
            return 0;
        }
    }

    logFile << "Parsed Config:" << std::endl;
    logFile << "  xmlPath: " << xmlPath << std::endl;
    logFile << "  outDir: " << outDir << std::endl;
    logFile << "  prefix: " << prefix << std::endl;
    logFile << "  aircraft: " << aircraftName << std::endl;
    logFile << "  parseAero: " << parseAero << ", parseFCS: " << parseFCS << ", parseMetrics: " << parseMetrics << std::endl;
    logFile.flush();

    if (xmlPath.empty())
    {
        std::cerr << "[Error] Missing required parameter: --xml <path>" << std::endl;
        logFile << "[Error] Missing --xml argument" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    std::cout << "====================================================\n";
    std::cout << " JSBSim XML Parser Starting...\n";
    std::cout << " Input XML:    " << xmlPath << "\n";
    std::cout << " Output Dir:   " << outDir << "\n";
    std::cout << " Prefix:       " << prefix << "\n";
    std::cout << "====================================================\n";

    JSB::XmlDocument doc;
    if (!doc.load_file(xmlPath))
    {
        std::cerr << "[Error] Failed to load/parse XML file: " << xmlPath << std::endl;
        logFile << "[Error] Failed to load/parse XML: " << xmlPath << std::endl;
        return 2;
    }

    logFile << "XML Loaded. Root tag: " << (doc.root ? doc.root->name : "<null>") << std::endl;
    logFile.flush();

    // Auto-detect aircraft name if not provided
    if (aircraftName.empty())
    {
        if (doc.root)
        {
            aircraftName = doc.root->attribute("name");
        }
        if (aircraftName.empty())
        {
            aircraftName = fs::path(xmlPath).stem().string();
        }
    }

    std::cout << " Aircraft Name: " << aircraftName << "\n";

    // Ensure output directory exists
    if (!ensure_directory_exists(outDir))
    {
        std::cerr << "[Error] Failed to create output directory: " << outDir << std::endl;
        logFile << "[Error] Failed to create output dir: " << outDir << std::endl;
        return 3;
    }

    int successCount = 0;

    // Detect nodes whether they are the root tag or children of root
    JSB::XmlNodePtr metricsNode = nullptr;
    JSB::XmlNodePtr fcsNode = nullptr;
    JSB::XmlNodePtr aeroNode = nullptr;

    if (doc.root)
    {
        if (JSB::XmlNode::equal_case_insensitive(doc.root->name, "metrics"))
            metricsNode = doc.root;
        else
            metricsNode = doc.root->child("metrics");

        if (JSB::XmlNode::equal_case_insensitive(doc.root->name, "flight_control"))
            fcsNode = doc.root;
        else
            fcsNode = doc.root->child("flight_control");

        if (JSB::XmlNode::equal_case_insensitive(doc.root->name, "aerodynamics"))
            aeroNode = doc.root;
        else
            aeroNode = doc.root->child("aerodynamics");
    }

    logFile << "Found Nodes:" << std::endl;
    logFile << "  metricsNode: " << (metricsNode != nullptr ? "YES" : "NO") << std::endl;
    logFile << "  fcsNode: " << (fcsNode != nullptr ? "YES" : "NO") << std::endl;
    logFile << "  aeroNode: " << (aeroNode != nullptr ? "YES" : "NO") << std::endl;
    logFile.flush();

    // 1. Metrics -> AircraftGeometry
    if (parseMetrics && metricsNode)
    {
        fs::path outMetrics = fs::path(outDir) / (prefix + "_AircraftGeometry.conf");
        if (JSB::MetricsParser::parse_and_write(metricsNode, aircraftName, outMetrics.string()))
        {
            ++successCount;
            logFile << "  -> Generated: " << outMetrics.string() << std::endl;
        }
    }

    // 2. Flight Control -> FlightControls
    if (parseFCS && fcsNode)
    {
        fs::path outFCS = fs::path(outDir) / (prefix + "_FlightControls.conf");
        if (JSB::FlightControlParser::parse_and_write(fcsNode, aircraftName, outFCS.string()))
        {
            ++successCount;
            logFile << "  -> Generated: " << outFCS.string() << std::endl;
        }
    }

    // 3. Aerodynamics -> Aerodynamics
    if (parseAero && aeroNode)
    {
        fs::path outAero = fs::path(outDir) / (prefix + "_Aerodynamics.conf");
        if (JSB::AerodynamicsParser::parse_and_write(aeroNode, aircraftName, outAero.string()))
        {
            ++successCount;
            logFile << "  -> Generated: " << outAero.string() << std::endl;
        }
    }

    logFile << "[Result] Config files generated: " << successCount << std::endl;

    std::cout << "====================================================\n";
    std::cout << " Finished parsing. Config files generated: " << successCount << "\n";
    std::cout << "====================================================\n";

    int exitCode = (successCount > 0) ? 0 : 4;
    logFile << "[Exit Code] " << exitCode << std::endl;
    logFile.flush();
    return exitCode;
}
