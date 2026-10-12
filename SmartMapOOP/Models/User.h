#pragma once
#include <string>
#include <optional>
#include "Position.h"
#include "Preference.h"

namespace SmartMap {
namespace Models {

class User {
private:
    std::string name;
    std::optional<Position> currentPosition;
    Preference preference;

public:
    explicit User(const std::string& name);

    void SetName(const std::string& name);
    std::string GetName() const;

    void SetCurrentPosition(const Position& position);
    std::optional<Position> GetCurrentPosition() const;

    Preference& GetPreference();
    const Preference& GetPreference() const;
};

} // namespace Models
} // namespace SmartMap

