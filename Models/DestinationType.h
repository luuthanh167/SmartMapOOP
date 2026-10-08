#pragma once
#include <string>

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
    if (str == "all") return DestinationType::ALL;
    if (str == "food") return DestinationType::FOOD;
    if (str == "coffee") return DestinationType::COFFEE;
    if (str == "shopping") return DestinationType::SHOPPING;
    if (str == "study") return DestinationType::STUDY;
    if (str == "relax") return DestinationType::RELAX;
    if (str == "entertainment") return DestinationType::ENTERTAINMENT;
    return DestinationType::ALL;
}

} // namespace Models
} // namespace SmartMap
