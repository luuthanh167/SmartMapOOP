#include "User.h"

namespace SmartMap {
namespace Models {

User::User(const std::string& name) : name(name), preference() {}

void User::SetName(const std::string& name) {
    this->name = name;
}

std::string User::GetName() const {
    return name;
}

void User::SetCurrentPosition(const Position& position) {
    currentPosition = position;
}

std::optional<Position> User::GetCurrentPosition() const {
    return currentPosition;
}

Preference& User::GetPreference() {
    return preference;
}

const Preference& User::GetPreference() const {
    return preference;
}

} // namespace Models
} // namespace SmartMap

