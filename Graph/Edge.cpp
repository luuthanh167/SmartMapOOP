#include "Edge.h"

namespace SmartMap {
namespace Graph {

Edge::Edge(int to, double distance) : to(to), distance(distance) {}

int Edge::GetTo() const {
    return to;
}

double Edge::GetDistance() const {
    return distance;
}

} // namespace Graph
} // namespace SmartMap

