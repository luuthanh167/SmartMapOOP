#pragma once
#include <vector>
#include <string>
#include "../Models/Location.h"
#include "../Models/DestinationType.h"

namespace SmartMap {
namespace Service {

class PlaceService {
public:
    PlaceService();

    // Tìm kiếm hỗ trợ chọn nhiều danh mục / topic cùng một lúc
    std::vector<Models::Location> SearchPlaces(
        double latitude,
        double longitude,
        double radiusMeters,
        const std::vector<Models::DestinationType>& types,
        double budget = 0.0);

    // Tìm kiếm theo 1 danh mục đơn lẻ
    std::vector<Models::Location> SearchPlaces(
        double latitude,
        double longitude,
        double radiusMeters,
        Models::DestinationType type,
        double budget = 0.0);

private:
    std::string GetTag(Models::DestinationType type) const;
};

} // namespace Service
} // namespace SmartMap
