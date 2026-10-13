#include "PathFinder.h"
#include <queue>
#include <algorithm>
#include <cmath>
#include <limits>

namespace SmartMap {
namespace Graph {

PathFinder::PathFinder(const Graph& graph) : graph(graph) {}

// Dijkstra
std::vector<int> PathFinder::Dijkstra(int start, int target, double& totalDistance) {
    std::vector<int> vertices;
    for (const auto& loc : graph.GetLocations()) {
        vertices.push_back(loc.GetId());
    }

    bool hasStart = (std::find(vertices.begin(), vertices.end(), start) != vertices.end());
    bool hasTarget = (std::find(vertices.begin(), vertices.end(), target) != vertices.end());

    if (!hasStart || !hasTarget) {
        totalDistance = std::numeric_limits<double>::infinity();
        return {};
    }

    if (start == target) {
        totalDistance = 0.0;
        return { start };
    }

    std::unordered_map<int, double> distances;
    std::unordered_map<int, int> parent;
    std::unordered_set<int> visited;

    for (int vertex : vertices) {
        distances[vertex] = std::numeric_limits<double>::infinity();
    }

    distances[start] = 0.0;

    while (visited.size() < vertices.size()) {
        int currentVertex = -1;
        double minDistance = std::numeric_limits<double>::infinity();

        for (int vertex : vertices) {
            if (visited.find(vertex) == visited.end() && distances[vertex] < minDistance) {
                minDistance = distances[vertex];
                currentVertex = vertex;
            }
        }

        if (currentVertex == -1) {
            break;
        }

        visited.insert(currentVertex);

        if (currentVertex == target) {
            break;
        }

        for (const auto& edge : graph.GetNeighbors(currentVertex)) {
            int neighbor = edge.GetTo();
            if (distances.find(neighbor) != distances.end()) {
                double distanceThroughCurrent = distances[currentVertex] + edge.GetDistance();

                if (distanceThroughCurrent < distances[neighbor]) {
                    distances[neighbor] = distanceThroughCurrent;
                    parent[neighbor] = currentVertex;
                }
            }
        }
    }

    auto it = distances.find(target);
    totalDistance = (it != distances.end()) ? it->second : std::numeric_limits<double>::infinity();

    if (std::isinf(totalDistance)) {
        return {};
    }

    return ReconstructPath(parent, start, target);
}

// Supporting method to reconstruct the path from start to target using the parent map
std::vector<int> PathFinder::ReconstructPath(const std::unordered_map<int, int>& parent,
                                             int start,
                                             int target) {
    std::vector<int> path;

    if (start == target) {
        path.push_back(start);
        return path;
    }

    if (parent.find(target) == parent.end()) {
        return path;
    }

    int currentVertex = target;

    while (currentVertex != start) {
        path.push_back(currentVertex);
        auto it = parent.find(currentVertex);
        if (it == parent.end()) {
            return {};
        }
        currentVertex = it->second;
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());

    return path;
}

} // namespace Graph
} // namespace SmartMap

