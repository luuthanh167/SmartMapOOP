#pragma once
#include <vector>
#include <unordered_map>
#include "Graph.h"
#include "PathFinder.h"
#include "RouteResult.h"

namespace SmartMap {
namespace Graph {

class RouteOptimizer {
private:
    const Graph& graph;
    PathFinder pathFinder;

    void SearchTSPBranchAndBound(
        int currentVertex,
        size_t depth,
        double currentDistance,
        std::vector<int>& currentOrder,
        std::vector<bool>& visited,
        const std::vector<int>& uniqueDestinations,
        const std::unordered_map<int, std::unordered_map<int, double>>& distMatrix,
        double& bestDistance,
        std::vector<int>& bestOrder);

    std::vector<int> BuildFullPath(int start, const std::vector<int>& destinationOrder);

public:
    explicit RouteOptimizer(const Graph& graph);

    RouteResult FindOptimalRoute(int start, const std::vector<int>& destinations);
};

} // namespace Graph
} // namespace SmartMap
