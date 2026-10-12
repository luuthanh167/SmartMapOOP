#include "Preference.h"
#include <algorithm>

namespace SmartMap {
namespace Models {

Preference::Preference() : maxDistance(0.0), budget(0.0) {}

void Preference::AddType(DestinationType type) {
    if (std::find(types.begin(), types.end(), type) == types.end()) {
        types.push_back(type);
    }
}

void Preference::RemoveType(DestinationType type) {
    types.erase(std::remove(types.begin(), types.end(), type), types.end());
}

std::vector<DestinationType> Preference::GetTypes() const {
    return types;
}

void Preference::SetMaxDistance(double maxDistance) {
    this->maxDistance = maxDistance;
}

double Preference::GetMaxDistance() const {
    return maxDistance;
}

void Preference::SetBudget(double budget) {
    this->budget = budget;
}

double Preference::GetBudget() const {
    return budget;
}

} // namespace Models
} // namespace SmartMap

