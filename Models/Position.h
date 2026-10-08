#pragma once

namespace SmartMap {
namespace Models {

class Position {
private:
    double latitude;
    double longitude;

public:
    Position();
    Position(double latitude, double longitude);

    double GetLatitude() const;
    double GetLongitude() const;
    void SetLatitude(double lat);
    void SetLongitude(double lon);
};

} // namespace Models
} // namespace SmartMap

