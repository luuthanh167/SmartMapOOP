#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace SmartMap {
namespace Models {

enum class DestinationType {
    ALL,
    FOOD,
    COFFEE,
    STUDY,
    SHOPPING,
    RELAX,
    ENTERTAINMENT
};

inline std::string DestinationTypeToString(DestinationType type) {
    switch (type) {
        case DestinationType::ALL: return "ALL";
        case DestinationType::FOOD: return "FOOD";
        case DestinationType::COFFEE: return "COFFEE";
        case DestinationType::STUDY: return "STUDY";
        case DestinationType::SHOPPING: return "SHOPPING";
        case DestinationType::RELAX: return "RELAX";
        case DestinationType::ENTERTAINMENT: return "ENTERTAINMENT";
        default: return "UNKNOWN";
    }
}

inline DestinationType StringToDestinationType(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });

    if (s == "all") return DestinationType::ALL;
    if (s == "food") return DestinationType::FOOD;
    if (s == "coffee") return DestinationType::COFFEE;
    if (s == "shopping") return DestinationType::SHOPPING;
    if (s == "study") return DestinationType::STUDY;
    if (s == "relax") return DestinationType::RELAX;
    if (s == "entertainment") return DestinationType::ENTERTAINMENT;
    return DestinationType::ALL;
}

inline std::vector<DestinationType> ParseMultipleDestinationTypes(const std::string& str) {
    std::vector<DestinationType> types;
    if (str.empty()) {
        types.push_back(DestinationType::ALL);
        return types;
    }

    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Xóa khoảng trắng thừa
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);

        if (!token.empty()) {
            DestinationType t = StringToDestinationType(token);
            if (std::find(types.begin(), types.end(), t) == types.end()) {
                types.push_back(t);
            }
        }
    }

    if (types.empty()) {
        types.push_back(DestinationType::ALL);
    }
    return types;
}

} // namespace Models
} // namespace SmartMap
