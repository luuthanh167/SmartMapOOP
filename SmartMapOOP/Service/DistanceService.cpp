#include "DistanceService.h"
#include <cmath>

namespace SmartMap {
namespace Service {

double DistanceService::CalculateDistance(const Models::Position& a, const Models::Position& b) const {
    double lat1 = a.GetLatitude();
    double lon1 = a.GetLongitude();

    double lat2 = b.GetLatitude();
    double lon2 = b.GetLongitude();

    const double earthRadiusKm = 6371.0;

    double dLat = DegreesToRadians(lat2 - lat1);
    double dLon = DegreesToRadians(lon2 - lon1);

    double lat1Rad = DegreesToRadians(lat1);
    double lat2Rad = DegreesToRadians(lat2);

    double h =
        std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
        std::cos(lat1Rad) * std::cos(lat2Rad) *
        std::sin(dLon / 2.0) * std::sin(dLon / 2.0);

    double distance = 2.0 * earthRadiusKm * std::asin(std::sqrt(h));

    return distance;
}

double DistanceService::DegreesToRadians(double degrees) const {
    const double pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
}

} // namespace Service
} // namespace SmartMap

