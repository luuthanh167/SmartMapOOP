#pragma once
#include "../Models/Position.h"

namespace SmartMap {
namespace Service {

class DistanceService {
private:
    double DegreesToRadians(double degrees) const;

public:
    double CalculateDistance(const Models::Position& a, const Models::Position& b) const;
};

} // namespace Service
} // namespace SmartMap

