#pragma once
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "../Service/PlaceService.h"
#include "../Service/RoutingService.h"
#include "../Service/DistanceService.h"

namespace SmartMap {
namespace Server {

class HttpServer {
private:
    int port;
    std::string webRoot;
    std::atomic<bool> isRunning;

    Service::PlaceService placeService;
    Service::RoutingService routingService;
    Service::DistanceService distanceService;

    std::string FindWebRoot() const;
    std::string GetMimeType(const std::string& path) const;
    std::string UrlDecode(const std::string& src) const;

    void HandleClient(uintptr_t clientSocket);
    void ProcessRequest(const std::string& requestRaw, uintptr_t clientSocket);

    void ServeStaticFile(const std::string& urlPath, uintptr_t clientSocket);
    void HandleGetPlaces(const std::string& queryString, uintptr_t clientSocket);
    void HandlePostRoute(const std::string& body, uintptr_t clientSocket);

    void SendResponse(uintptr_t clientSocket, int statusCode, const std::string& statusText,
                      const std::string& contentType,
                      const std::string& body);

public:
    explicit HttpServer(int port = 5000);
    ~HttpServer();

    void Start();
    void Stop();
};

} // namespace Server
} // namespace SmartMap

