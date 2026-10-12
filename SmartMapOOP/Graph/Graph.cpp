#include "Graph.h"
#include <limits>

namespace SmartMap {
namespace Graph {

Graph::Graph() {}

void Graph::AddLocation(const Models::Location& location) {
    int id = location.GetId();
    if (locations.find(id) == locations.end()) {
        locations[id] = location;
        adjacencyList[id] = std::vector<Edge>();
    }
}

void Graph::AddEdge(int from, int to, double distance) {
    if (adjacencyList.find(from) == adjacencyList.end() ||
        adjacencyList.find(to) == adjacencyList.end()) {
        return;
    }

    adjacencyList[from].push_back(Edge(to, distance));
    adjacencyList[to].push_back(Edge(from, distance));
}

std::vector<Edge> Graph::GetNeighbors(int locationId) const {
    auto it = adjacencyList.find(locationId);
    if (it != adjacencyList.end()) {
        return it->second;
    }
    return {};
}

std::optional<Models::Location> Graph::GetLocation(int id) const {
    auto it = locations.find(id);
    if (it != locations.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Models::Location> Graph::GetLocations() const {
    std::vector<Models::Location> result;
    result.reserve(locations.size());
    for (const auto& pair : locations) {
        result.push_back(pair.second);
    }
    return result;
}

double Graph::GetDistance(int from, int to) const {
    auto it = adjacencyList.find(from);
    if (it == adjacencyList.end()) {
        return std::numeric_limits<double>::infinity();
    }

    for (const auto& edge : it->second) {
        if (edge.GetTo() == to) {
            return edge.GetDistance();
        }
    }

    return std::numeric_limits<double>::infinity();
}

} // namespace Graph
} // namespace SmartMap

