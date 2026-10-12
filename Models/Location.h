#pragma once
#include <string>
#include "Position.h"
#include "DestinationType.h"

namespace SmartMap {
namespace Models {

class Location {
private:
    int id;
    std::string name;
    std::string address;
    Position position;
    DestinationType type;
    double price;
    std::string placeId;
    double rating;

public:
    Location();
    Location(int id,
             const std::string& name,
             const std::string& address,
             const Position& position,
             DestinationType type,
             double price,
             const std::string& placeId,
             double rating = 4.5);

    int GetId() const;
    std::string GetName() const;
    std::string GetAddress() const;
    Position GetPosition() const;
    DestinationType GetType() const;
    double GetPrice() const;
    std::string GetPlaceId() const;
    double GetRating() const;
};

} // namespace Models
} // namespace SmartMap

