#pragma once
#include <string>
#include <vector>
#include <optional>

namespace SmartMap {
namespace Models {

struct Waypoint {
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
};

struct RouteRequest {
    std::string startName = "";
    std::string destinationName = "";
    std::optional<double> startLat = std::nullopt;
    std::optional<double> startLon = std::nullopt;
    std::optional<double> destLat = std::nullopt;
    std::optional<double> destLon = std::nullopt;
    std::vector<Waypoint> waypoints;
    bool optimizeOrder = true;
};

} // namespace Models
} // namespace SmartMap
