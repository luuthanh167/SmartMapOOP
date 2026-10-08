#pragma once

namespace SmartMap {
namespace Graph {

class Edge {
private:
    int to;
    double distance;

public:
    Edge(int to, double distance);

    int GetTo() const;
    double GetDistance() const;
};

} // namespace Graph
} // namespace SmartMap

