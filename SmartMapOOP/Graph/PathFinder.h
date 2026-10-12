#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "Graph.h"

namespace SmartMap {
namespace Graph {

class PathFinder {
private:
    const Graph& graph;

    bool DFSUtil(int current,
                 int target,
                 std::unordered_set<int>& visited,
                 std::unordered_map<int, int>& parent);

    std::vector<int> ReconstructPath(const std::unordered_map<int, int>& parent,
                                     int start,
                                     int target);

public:
    explicit PathFinder(const Graph& graph);

    // BFS
    std::vector<int> BFS(int start, int target);

    // DFS
    std::vector<int> DFS(int start, int target);

    // Dijkstra
    std::vector<int> Dijkstra(int start, int target, double& totalDistance);
};

} // namespace Graph
} // namespace SmartMap

