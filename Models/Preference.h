#pragma once
#include <vector>
#include "DestinationType.h"

namespace SmartMap {
namespace Models {

class Preference {
private:
    std::vector<DestinationType> types;
    double maxDistance;
    double budget;

public:
    Preference();

    void AddType(DestinationType type);
    void RemoveType(DestinationType type);
    std::vector<DestinationType> GetTypes() const;

    void SetMaxDistance(double maxDistance);
    double GetMaxDistance() const;

    void SetBudget(double budget);
    double GetBudget() const;
};

} // namespace Models
} // namespace SmartMap

