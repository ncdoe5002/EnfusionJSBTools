#pragma once
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace JSB
{

class GuidGenerator
{
public:
    static std::string generate()
    {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        uint64_t val = dis(gen);
        std::stringstream ss;
        ss << "{" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << val << "}";
        return ss.str();
    }
};

} // namespace JSB
