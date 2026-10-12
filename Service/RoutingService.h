#pragma once
#include <vector>
#include <string>
#include "../Models/Position.h"
#include "DistanceService.h"

namespace SmartMap {
namespace Service {

class RoadRoute {
private:
    double distance; // meters
    double duration; // seconds
    std::vector<std::vector<double>> routeCoordinates; // [[lon, lat], ...]

public:
    RoadRoute(double distance,
              double duration,
              const std::vector<std::vector<double>>& routeCoordinates);

    double GetDistance() const;
    double GetDuration() const;
    const std::vector<std::vector<double>>& GetRouteCoordinates() const;
};

class RoutingService {
private:
    DistanceService distanceService;

    std::string FetchOsrmRouteJson(double startLon, double startLat, double destLon, double destLat);
    bool ParseOsrmResponse(const std::string& json,
                           double& outDistance,
                           double& outDuration,
                           std::vector<std::vector<double>>& outCoordinates);

public:
    RoutingService();

    RoadRoute GetRoute(const Models::Position& start, const Models::Position& destination);
};

} // namespace Service
} // namespace SmartMap

