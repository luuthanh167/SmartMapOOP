#include "Location.h"

namespace SmartMap {
namespace Models {

Location::Location()
    : id(0), name(""), address(""), position(), type(DestinationType::COFFEE), price(0.0), placeId("") {}

Location::Location(int id,
                   const std::string& name,
                   const std::string& address,
                   const Position& position,
                   DestinationType type,
                   double price,
                   const std::string& placeId)
    : id(id),
      name(name),
      address(address),
      position(position),
      type(type),
      price(price),
      placeId(placeId) {}

int Location::GetId() const {
    return id;
}

std::string Location::GetName() const {
    return name;
}

std::string Location::GetAddress() const {
    return address;
}

Position Location::GetPosition() const {
    return position;
}

DestinationType Location::GetType() const {
    return type;
}

double Location::GetPrice() const {
    return price;
}

std::string Location::GetPlaceId() const {
    return placeId;
}

} // namespace Models
} // namespace SmartMap

