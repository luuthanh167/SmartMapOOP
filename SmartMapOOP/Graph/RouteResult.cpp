#include "RouteResult.h"

namespace SmartMap {
namespace Graph {

RouteResult::RouteResult(const std::vector<int>& fullPath,
                         const std::vector<int>& destinationOrder,
                         double totalDistance)
    : fullPath(fullPath),
      destinationOrder(destinationOrder),
      totalDistance(totalDistance) {}

std::vector<int> RouteResult::GetFullPath() const {
    return fullPath;
}

std::vector<int> RouteResult::GetDestinationOrder() const {
    return destinationOrder;
}

double RouteResult::GetTotalDistance() const {
    return totalDistance;
}

} // namespace Graph
} // namespace SmartMap

