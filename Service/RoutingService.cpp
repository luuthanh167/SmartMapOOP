#include "RoutingService.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define closesocket close
#endif

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

std::string RoutingService::FetchOsrmRouteJson(double startLon, double startLat, double destLon, double destLat) {
    const std::string host = "router.project-osrm.org";
    const std::string portStr = "80";

    std::ostringstream pathStream;
    pathStream << std::fixed << std::setprecision(6);
    pathStream << "/route/v1/driving/" << startLon << "," << startLat << ";" << destLon << "," << destLat
               << "?overview=full&geometries=geojson&steps=false";
    std::string path = pathStream.str();

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0 || !res) {
        return "";
    }

    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(res);
        return "";
    }

    // Thiết lập timeout 4 giây để không bao giờ bị đơ ứng dụng nếu mạng chậm
#ifdef _WIN32
    DWORD timeout = 4000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif

    if (connect(sock, res->ai_addr, static_cast<int>(res->ai_addrlen)) != 0) {
        closesocket(sock);
        freeaddrinfo(res);
        return "";
    }
    freeaddrinfo(res);

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";
    req << "User-Agent: SmartMapOOP/1.0\r\n";
    req << "Connection: close\r\n\r\n";

    std::string reqStr = req.str();
    send(sock, reqStr.c_str(), static_cast<int>(reqStr.size()), 0);

    std::string response;
    char buffer[4096];
    int bytes = 0;
    while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes);
    }
    closesocket(sock);

    size_t headerEnd = response.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        return response.substr(headerEnd + 4);
    }
    return response;
}

bool RoutingService::ParseOsrmResponse(const std::string& json,
                                       double& outDistance,
                                       double& outDuration,
                                       std::vector<std::vector<double>>& outCoordinates) {
    if (json.empty() || json.find("\"code\":\"Ok\"") == std::string::npos) {
        return false;
    }

    // 1. Trích xuất distance
    size_t distPos = json.find("\"distance\":");
    if (distPos != std::string::npos) {
        size_t start = distPos + 11;
        size_t end = json.find_first_of(",}", start);
        if (end != std::string::npos) {
            try {
                outDistance = std::stod(json.substr(start, end - start));
            } catch (...) {
                outDistance = 0.0;
            }
        }
    }

    // 2. Trích xuất duration
    size_t durPos = json.find("\"duration\":");
    if (durPos != std::string::npos) {
        size_t start = durPos + 11;
        size_t end = json.find_first_of(",}", start);
        if (end != std::string::npos) {
            try {
                outDuration = std::stod(json.substr(start, end - start));
            } catch (...) {
                outDuration = 0.0;
            }
        }
    }

    // 3. Trích xuất mảng coordinates: [[lon, lat], [lon, lat], ...]
    size_t coordKey = json.find("\"coordinates\":");
    if (coordKey == std::string::npos) {
        return false;
    }

    size_t arrStart = json.find('[', coordKey);
    if (arrStart == std::string::npos) {
        return false;
    }

    size_t cur = arrStart + 1;
    outCoordinates.clear();

    while (cur < json.size()) {
        size_t ptStart = json.find('[', cur);
        if (ptStart == std::string::npos) break;

        size_t ptEnd = json.find(']', ptStart);
        if (ptEnd == std::string::npos) break;

        std::string ptStr = json.substr(ptStart + 1, ptEnd - ptStart - 1);
        size_t comma = ptStr.find(',');
        if (comma != std::string::npos) {
            try {
                double lon = std::stod(ptStr.substr(0, comma));
                double lat = std::stod(ptStr.substr(comma + 1));
                outCoordinates.push_back({ lon, lat });
            } catch (...) {}
        }

        cur = ptEnd + 1;

        // Nếu gặp dấu đóng ngoặc vuông của mảng coordinates ngoài cùng thì dừng
        size_t nextChar = json.find_first_not_of(" \t\r\n,", cur);
        if (nextChar != std::string::npos && json[nextChar] == ']') {
            break;
        }
    }

    return !outCoordinates.empty();
}

RoadRoute RoutingService::GetRoute(const Models::Position& start, const Models::Position& destination) {
    double startLat = start.GetLatitude();
    double startLon = start.GetLongitude();

    double destLat = destination.GetLatitude();
    double destLon = destination.GetLongitude();

    // 1. Thử gọi OSRM để lấy toàn bộ các khúc cua và tọa độ đường đi thật ngoài đời
    std::string jsonResponse = FetchOsrmRouteJson(startLon, startLat, destLon, destLat);
    double osrmDist = 0.0;
    double osrmDur = 0.0;
    std::vector<std::vector<double>> osrmCoords;

    if (ParseOsrmResponse(jsonResponse, osrmDist, osrmDur, osrmCoords) && osrmCoords.size() >= 2) {
        return RoadRoute(osrmDist, osrmDur, osrmCoords);
    }

    // 2. Dự phòng: Tính khoảng cách Haversine nếu không có kết nối OSRM
    double directDistKm = distanceService.CalculateDistance(start, destination);
    double distanceMeters = directDistKm * 1000.0;
    double durationSeconds = directDistKm * 120.0;

    std::vector<std::vector<double>> fallbackCoords;
    fallbackCoords.push_back({ startLon, startLat });
    fallbackCoords.push_back({ destLon, destLat });

    return RoadRoute(distanceMeters, durationSeconds, fallbackCoords);
}

} // namespace Service
} // namespace SmartMap