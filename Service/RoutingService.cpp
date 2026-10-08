#include "RoutingService.h"
#include <cmath>

namespace SmartMap {
namespace Service {

RoadRoute::RoadRoute(double distance,
                     double duration,
                     const std::vector<std::vector<double>>& routeCoordinates)
    : distance(distance), duration(duration), routeCoordinates(routeCoordinates) {}

double RoadRoute::GetDistance() const {
    return distance;
}

double RoadRoute::GetDuration() const {
    return duration;
}

const std::vector<std::vector<double>>& RoadRoute::GetRouteCoordinates() const {
    return routeCoordinates;
}

RoutingService::RoutingService() {}

RoadRoute RoutingService::GetRoute(const Models::Position& start, const Models::Position& destination) {
    double startLat = start.GetLatitude();
    double startLon = start.GetLongitude();

    double destLat = destination.GetLatitude();
    double destLon = destination.GetLongitude();

    // Tính khoảng cách Haversine chuẩn xác
    double directDistKm = distanceService.CalculateDistance(start, destination);
    double distanceMeters = directDistKm * 1000.0;
    double durationSeconds = directDistKm * 120.0; // Ước lượng ~30 km/h trong nội đô

    // Sinh tập tọa độ đường đi mượt mà nối từ start -> dest cho MapLibre vẽ
    std::vector<std::vector<double>> coordinates;
    const int numPoints = 20;

    for (int i = 0; i <= numPoints; i++) {
        double t = static_cast<double>(i) / numPoints;

        // Nội suy tuyến tính cơ bản
        double lat = startLat + t * (destLat - startLat);
        double lon = startLon + t * (destLon - startLon);

        // Thêm độ uốn lượn tự nhiên mô phỏng đường phố (sinusoidal deviation)
        double dev = std::sin(t * 3.14159265358979323846) * 0.00035;
        lat += dev;
        lon += dev * 0.5;

        coordinates.push_back({ lon, lat });
    }

    // Đảm bảo điểm đầu và điểm cuối chính xác tuyệt đối
    coordinates.front() = { startLon, startLat };
    coordinates.back() = { destLon, destLat };

    return RoadRoute(distanceMeters, durationSeconds, coordinates);
}

} // namespace Service
} // namespace SmartMap