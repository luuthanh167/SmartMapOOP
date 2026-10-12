#include "HttpServer.h"
#include "../Graph/Graph.h"
#include "../Graph/RouteOptimizer.h"
#include "../Graph/RouteResult.h"
#include "../Models/Position.h"
#include "../Models/Location.h"
#include "../Models/DestinationType.h"
#include "../Models/RouteRequest.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

namespace fs = std::filesystem;

namespace SmartMap {
namespace Server {

static std::string JsonEscape(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

static std::string ExtractJsonString(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";

    size_t start = json.find('"', pos);
    if (start == std::string::npos) return "";

    size_t end = json.find('"', start + 1);
    if (end == std::string::npos) return "";

    return json.substr(start + 1, end - start - 1);
}

static double ExtractJsonDouble(const std::string& json, const std::string& key, double defaultVal) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return defaultVal;

    pos = json.find(':', pos);
    if (pos == std::string::npos) return defaultVal;

    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
        pos++;
    }

    size_t end = pos;
    while (end < json.size() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-' || json[end] == '+')) {
        end++;
    }

    if (end > pos) {
        try {
            return std::stod(json.substr(pos, end - pos));
        } catch (...) {
            return defaultVal;
        }
    }
    return defaultVal;
}

static bool ExtractJsonBool(const std::string& json, const std::string& key, bool defaultVal) {
    std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return defaultVal;

    pos = json.find(':', pos);
    if (pos == std::string::npos) return defaultVal;

    if (json.find("true", pos) < json.find(',', pos)) return true;
    if (json.find("false", pos) < json.find(',', pos)) return false;
    return defaultVal;
}

static std::vector<Models::Waypoint> ExtractJsonWaypoints(const std::string& json) {
    std::vector<Models::Waypoint> waypoints;
    size_t pos = json.find("\"waypoints\"");
    if (pos == std::string::npos) return waypoints;

    size_t arrStart = json.find('[', pos);
    if (arrStart == std::string::npos) return waypoints;

    size_t arrEnd = json.find(']', arrStart);
    if (arrEnd == std::string::npos) return waypoints;

    std::string arrContent = json.substr(arrStart, arrEnd - arrStart + 1);
    size_t cur = 0;

    while ((cur = arrContent.find('{', cur)) != std::string::npos) {
        size_t objEnd = arrContent.find('}', cur);
        if (objEnd == std::string::npos) break;

        std::string objStr = arrContent.substr(cur, objEnd - cur + 1);

        Models::Waypoint wp;
        wp.name = ExtractJsonString(objStr, "name");
        wp.latitude = ExtractJsonDouble(objStr, "latitude", 0.0);
        wp.longitude = ExtractJsonDouble(objStr, "longitude", 0.0);

        if (wp.latitude != 0.0 || wp.longitude != 0.0) {
            waypoints.push_back(wp);
        }

        cur = objEnd + 1;
    }

    return waypoints;
}

HttpServer::HttpServer(int port) : port(port), isRunning(false) {
    webRoot = FindWebRoot();
}

HttpServer::~HttpServer() {
    Stop();
}

std::string HttpServer::FindWebRoot() const {
    std::vector<std::string> candidates = {
        "wwwroot",
        "./wwwroot",
        "../wwwroot",
        "Front-end",
        "./Front-end",
        "../Front-end",
        "d:/SmartMapOOP/wwwroot",
        "D:/SmartMapOOP/wwwroot"
    };

    for (const auto& path : candidates) {
        if (fs::exists(path) && fs::is_directory(path)) {
            return fs::absolute(path).string();
        }
    }
    return "wwwroot";
}

std::string HttpServer::GetMimeType(const std::string& path) const {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
    if (path.rfind(".png") != std::string::npos) return "image/png";
    if (path.rfind(".jpg") != std::string::npos || path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
    return "text/plain; charset=utf-8";
}

std::string HttpServer::UrlDecode(const std::string& src) const {
    std::string ret;
    ret.reserve(src.length());
    for (size_t i = 0; i < src.length(); i++) {
        if (src[i] == '%') {
            if (i + 2 < src.length()) {
                int hexVal = 0;
                std::istringstream hexStream(src.substr(i + 1, 2));
                if (hexStream >> std::hex >> hexVal) {
                    ret += static_cast<char>(hexVal);
                    i += 2;
                } else {
                    ret += '%';
                }
            } else {
                ret += '%';
            }
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

void HttpServer::SendResponse(uintptr_t clientSocket,
                              int statusCode,
                              const std::string& statusText,
                              const std::string& contentType,
                              const std::string& body) {
    SOCKET client = static_cast<SOCKET>(clientSocket);

    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    std::string respStr = response.str();
    send(client, respStr.c_str(), static_cast<int>(respStr.size()), 0);
}

void HttpServer::ServeStaticFile(const std::string& urlPath, uintptr_t clientSocket) {
    std::string cleanPath = urlPath;
    if (cleanPath == "/" || cleanPath.empty()) {
        cleanPath = "/firstPage.html";
    }

    // Xóa query string nếu có
    size_t qPos = cleanPath.find('?');
    if (qPos != std::string::npos) {
        cleanPath = cleanPath.substr(0, qPos);
    }

    fs::path fullPath = fs::path(webRoot) / cleanPath.substr(cleanPath.front() == '/' ? 1 : 0);

    if (!fs::exists(fullPath) || fs::is_directory(fullPath)) {
        std::string notFound = "<h1>404 Not Found: " + cleanPath + "</h1>";
        SendResponse(clientSocket, 404, "Not Found", "text/html; charset=utf-8", notFound);
        return;
    }

    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        std::string err = "<h1>500 Cannot Read File</h1>";
        SendResponse(clientSocket, 500, "Internal Server Error", "text/html; charset=utf-8", err);
        return;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::string mime = GetMimeType(fullPath.string());
    SendResponse(clientSocket, 200, "OK", mime, content);
}

void HttpServer::HandleGetPlaces(const std::string& queryString, uintptr_t clientSocket) {
    std::string category = "all";
    double lat = 10.8498;
    double lon = 106.7713;
    double distance = 2.0;
    double budget = 0.0;

    std::istringstream qs(queryString);
    std::string param;
    while (std::getline(qs, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string val = UrlDecode(param.substr(eq + 1));

            if (key == "category" || key == "categories") category = val;
            else if (key == "lat") try { lat = std::stod(val); } catch (...) {}
            else if (key == "lon") try { lon = std::stod(val); } catch (...) {}
            else if (key == "distance") try { distance = std::stod(val); } catch (...) {}
            else if (key == "budget") try { budget = std::stod(val); } catch (...) {}
        }
    }

    std::vector<Models::DestinationType> types = Models::ParseMultipleDestinationTypes(category);
    auto places = placeService.SearchPlaces(lat, lon, distance * 1000.0, types, budget);

    std::ostringstream json;
    json << "[\n";
    for (size_t i = 0; i < places.size(); i++) {
        const auto& p = places[i];
        json << "  {\n";
        json << "    \"id\": " << p.GetId() << ",\n";
        json << "    \"name\": \"" << JsonEscape(p.GetName()) << "\",\n";
        json << "    \"address\": \"" << JsonEscape(p.GetAddress()) << "\",\n";
        json << "    \"latitude\": " << std::fixed << std::setprecision(6) << p.GetPosition().GetLatitude() << ",\n";
        json << "    \"longitude\": " << std::fixed << std::setprecision(6) << p.GetPosition().GetLongitude() << ",\n";
        json << "    \"type\": \"" << Models::DestinationTypeToString(p.GetType()) << "\",\n";
        json << "    \"price\": " << std::fixed << std::setprecision(0) << p.GetPrice() << "\n";
        json << "  }" << (i + 1 < places.size() ? ",\n" : "\n");
    }
    json << "]";

    SendResponse(clientSocket, 200, "OK", "application/json; charset=utf-8", json.str());
}

void HttpServer::HandlePostRoute(const std::string& body, uintptr_t clientSocket) {
    // 1. Tọa độ mặc định và tra cứu địa điểm nổi tiếng (giống C# Program.cs)
    std::unordered_map<std::string, std::pair<double, double>> knownLocations = {
        {"Đại học Công nghệ Kỹ thuật TP.HCM", {10.8498, 106.7713}},
        {"Đại học Sư phạm Kỹ thuật TP.HCM",   {10.8506, 106.7719}},
        {"Vincom Plaza Lê Văn Việt",          {10.8458, 106.7778}},
        {"Làng Đại học Quốc gia TP.HCM",      {10.8700, 106.8030}},
        {"Đại học Văn Lang",                  {10.8275, 106.6990}},
        {"Đại học Bách Khoa TP.HCM",          {10.7720, 106.6578}},
        {"Local Food Restaurant",             {10.8512, 106.7725}},
        {"Coffee House",                      {10.8465, 106.7780}},
        {"Entertainment Place",               {10.8580, 106.7800}}
    };

    std::string startName = ExtractJsonString(body, "startName");
    double startLat = ExtractJsonDouble(body, "startLat", -999.0);
    double startLon = ExtractJsonDouble(body, "startLon", -999.0);

    if (startLat == -999.0 || startLon == -999.0) {
        auto it = knownLocations.find(startName);
        if (it != knownLocations.end()) {
            startLat = it->second.first;
            startLon = it->second.second;
        } else {
            startLat = 10.8498;
            startLon = 106.7713;
        }
    }

    std::vector<Models::Waypoint> waypoints = ExtractJsonWaypoints(body);
    bool optimizeOrder = ExtractJsonBool(body, "optimizeOrder", true);

    // Nếu frontend gửi đơn điểm bằng destinationName/destLat/destLon
    if (waypoints.empty()) {
        std::string destName = ExtractJsonString(body, "destinationName");
        double destLat = ExtractJsonDouble(body, "destLat", -999.0);
        double destLon = ExtractJsonDouble(body, "destLon", -999.0);

        if (destLat == -999.0 || destLon == -999.0) {
            auto it = knownLocations.find(destName);
            if (it != knownLocations.end()) {
                destLat = it->second.first;
                destLon = it->second.second;
            } else {
                destLat = 10.8465;
                destLon = 106.7780;
            }
        }
        waypoints.push_back({ destName.empty() ? "Điểm đến" : destName, destLat, destLon });
    }

    Models::Position startPos(startLat, startLon);

    // KẾT HỢP CÁC THUẬT TOÁN ĐỒ THỊ & TỐI ƯU HÓA HOÁN VỊ (TSP)
    std::vector<Models::Waypoint> orderedWaypoints = waypoints;

    if (waypoints.size() > 1 && optimizeOrder) {
        // Xây dựng đồ thị Graph với đỉnh 0 là Start và các đỉnh 1..N là Waypoints
        Graph::Graph graph;
        graph.AddLocation(Models::Location(0, startName, "", startPos, Models::DestinationType::STUDY, 0, "node/0"));

        for (size_t i = 0; i < waypoints.size(); i++) {
            Models::Position wpPos(waypoints[i].latitude, waypoints[i].longitude);
            graph.AddLocation(Models::Location(static_cast<int>(i + 1), waypoints[i].name, "", wpPos, Models::DestinationType::COFFEE, 0, "node/" + std::to_string(i + 1)));
        }

        // Tính khoảng cách giữa tất cả các cặp đỉnh (DistanceService / Haversine)
        std::vector<Models::Position> allPoints;
        allPoints.push_back(startPos);
        for (const auto& wp : waypoints) {
            allPoints.push_back(Models::Position(wp.latitude, wp.longitude));
        }

        for (size_t i = 0; i < allPoints.size(); i++) {
            for (size_t j = i + 1; j < allPoints.size(); j++) {
                double dist = distanceService.CalculateDistance(allPoints[i], allPoints[j]) * 1000.0; // mét
                graph.AddEdge(static_cast<int>(i), static_cast<int>(j), dist);
            }
        }

        // Chạy thuật toán RouteOptimizer (hoán vị đệ quy TSP) để tìm thứ tự ngắn nhất
        Graph::RouteOptimizer optimizer(graph);
        std::vector<int> destIds;
        for (size_t i = 1; i <= waypoints.size(); i++) {
            destIds.push_back(static_cast<int>(i));
        }

        Graph::RouteResult optResult = optimizer.FindOptimalRoute(0, destIds);
        auto bestOrder = optResult.GetDestinationOrder();

        if (bestOrder.size() == waypoints.size()) {
            orderedWaypoints.clear();
            for (int id : bestOrder) {
                orderedWaypoints.push_back(waypoints[id - 1]);
            }
        }
    }

    // Ghép nối từng chặng (Legs) và tính toàn bộ tọa độ đường đi (routeCoordinates)
    double totalDistanceMeters = 0.0;
    double totalDurationSeconds = 0.0;
    std::vector<std::vector<double>> allRouteCoordinates;

    struct LegInfo {
        std::string fromName;
        std::string toName;
        double distanceKm;
        double durationMin;
    };
    std::vector<LegInfo> legs;

    Models::Position currentPos = startPos;
    std::string currentName = startName.empty() ? "Vị trí của bạn" : startName;

    for (const auto& nextWp : orderedWaypoints) {
        Models::Position nextPos(nextWp.latitude, nextWp.longitude);
        Service::RoadRoute legRoute = routingService.GetRoute(currentPos, nextPos);

        double legDist = legRoute.GetDistance();
        double legDur = legRoute.GetDuration();

        totalDistanceMeters += legDist;
        totalDurationSeconds += legDur;

        legs.push_back({
            currentName,
            nextWp.name,
            legDist / 1000.0,
            legDur / 60.0
        });

        const auto& legCoords = legRoute.GetRouteCoordinates();
        if (allRouteCoordinates.empty()) {
            allRouteCoordinates.insert(allRouteCoordinates.end(), legCoords.begin(), legCoords.end());
        } else {
            // Tránh lặp điểm nối giữa 2 chặng
            allRouteCoordinates.insert(allRouteCoordinates.end(), legCoords.begin() + 1, legCoords.end());
        }

        currentPos = nextPos;
        currentName = nextWp.name;
    }

    // Xây dựng JSON phản hồi chuẩn xác cho secondPage.js
    std::ostringstream json;
    json << std::fixed << std::setprecision(4);
    json << "{\n";
    json << "  \"totalDistance\": " << (totalDistanceMeters / 1000.0) << ",\n";
    json << "  \"duration\": " << totalDurationSeconds << ",\n";

    // Destination cuối cùng
    double finalLat = orderedWaypoints.back().latitude;
    double finalLon = orderedWaypoints.back().longitude;
    json << "  \"destination\": { \"latitude\": " << finalLat << ", \"longitude\": " << finalLon << " },\n";

    // OrderedWaypoints
    json << "  \"orderedWaypoints\": [\n";
    for (size_t i = 0; i < orderedWaypoints.size(); i++) {
        const auto& ow = orderedWaypoints[i];
        json << "    { \"name\": \"" << JsonEscape(ow.name) << "\", \"latitude\": " << ow.latitude << ", \"longitude\": " << ow.longitude << " }"
             << (i + 1 < orderedWaypoints.size() ? ",\n" : "\n");
    }
    json << "  ],\n";

    // Legs
    json << "  \"legs\": [\n";
    for (size_t i = 0; i < legs.size(); i++) {
        const auto& leg = legs[i];
        json << "    { \"fromName\": \"" << JsonEscape(leg.fromName)
             << "\", \"toName\": \"" << JsonEscape(leg.toName)
             << "\", \"distanceKm\": " << std::setprecision(2) << leg.distanceKm
             << ", \"durationMin\": " << std::setprecision(1) << leg.durationMin << " }"
             << (i + 1 < legs.size() ? ",\n" : "\n");
    }
    json << "  ],\n";

    // RouteCoordinates
    json << "  \"routeCoordinates\": [\n";
    for (size_t i = 0; i < allRouteCoordinates.size(); i++) {
        json << "    [" << std::setprecision(6) << allRouteCoordinates[i][0] << ", " << allRouteCoordinates[i][1] << "]"
             << (i + 1 < allRouteCoordinates.size() ? ",\n" : "\n");
    }
    json << "  ]\n";
    json << "}";

    SendResponse(clientSocket, 200, "OK", "application/json; charset=utf-8", json.str());
}

void HttpServer::ProcessRequest(const std::string& requestRaw, uintptr_t clientSocket) {
    std::istringstream reqStream(requestRaw);
    std::string method, url, proto;
    reqStream >> method >> url >> proto;

    if (method.empty()) {
        return;
    }

    if (method == "OPTIONS") {
        SendResponse(clientSocket, 200, "OK", "text/plain", "");
        return;
    }

    // Tách path và query string
    std::string path = url;
    std::string queryString = "";
    size_t qPos = url.find('?');
    if (qPos != std::string::npos) {
        path = url.substr(0, qPos);
        queryString = url.substr(qPos + 1);
    }

    if (method == "GET" && path == "/api/places") {
        HandleGetPlaces(queryString, clientSocket);
        return;
    }

    if (method == "POST" && path == "/api/route") {
        // Tìm body sau \r\n\r\n
        size_t bodyPos = requestRaw.find("\r\n\r\n");
        std::string body = "";
        if (bodyPos != std::string::npos) {
            body = requestRaw.substr(bodyPos + 4);
        }
        HandlePostRoute(body, clientSocket);
        return;
    }

    // Mặc định phục vụ file tĩnh (HTML, CSS, JS)
    ServeStaticFile(path, clientSocket);
}

void HttpServer::HandleClient(uintptr_t clientSocket) {
    SOCKET client = static_cast<SOCKET>(clientSocket);
    std::vector<char> buffer(65536);
    int bytesRead = recv(client, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string raw(buffer.data(), bytesRead);
        ProcessRequest(raw, clientSocket);
    }

    closesocket(client);
}

void HttpServer::Start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Lỗi: Không thể khởi tạo Winsock!" << std::endl;
        return;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Lỗi: Không thể tạo socket!" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Lỗi: Không thể bind tới cổng " << port << "!" << std::endl;
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Lỗi: Listen thất bại!" << std::endl;
        closesocket(serverSocket);
        return;
    }

    isRunning = true;

    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << " Web SmartMap (C++) dang chay tai: http://localhost:" << port << "\n";
    std::cout << " Mo trinh duyet va truy cap: http://localhost:" << port << "\n";
    std::cout << " Thu muc Web Root: " << webRoot << "\n";
    std::cout << "=================================================\n\n";

    // Tự động mở trình duyệt web trên Windows
#ifdef _WIN32
    std::string url = "http://localhost:" + std::to_string(port);
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif

    while (isRunning) {
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);

        if (clientSocket == INVALID_SOCKET) {
            if (!isRunning) break;
            continue;
        }

        std::thread([this, clientSocket]() {
            this->HandleClient(static_cast<uintptr_t>(clientSocket));
        }).detach();
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::Stop() {
    isRunning = false;
}

} // namespace Server
} // namespace SmartMap
