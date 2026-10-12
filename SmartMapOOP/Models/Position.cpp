#include "Position.h"

namespace SmartMap {
namespace Models {

Position::Position() : latitude(0.0), longitude(0.0) {}

Position::Position(double latitude, double longitude)
    : latitude(latitude), longitude(longitude) {}

double Position::GetLatitude() const {
    return latitude;
}

double Position::GetLongitude() const {
    return longitude;
}

void Position::SetLatitude(double lat) {
    latitude = lat;
}

void Position::SetLongitude(double lon) {
    longitude = lon;
}

} // namespace Models
} // namespace SmartMap

