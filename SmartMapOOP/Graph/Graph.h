#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include "Edge.h"
#include "../Models/Location.h"

namespace SmartMap {
namespace Graph {

class Graph {
private:
    std::unordered_map<int, Models::Location> locations;
    std::unordered_map<int, std::vector<Edge>> adjacencyList;

public:
    Graph();

    void AddLocation(const Models::Location& location);
    void AddEdge(int from, int to, double distance);

    std::vector<Edge> GetNeighbors(int locationId) const;
    std::optional<Models::Location> GetLocation(int id) const;
    std::vector<Models::Location> GetLocations() const;
    double GetDistance(int from, int to) const;
};

} // namespace Graph
} // namespace SmartMap

