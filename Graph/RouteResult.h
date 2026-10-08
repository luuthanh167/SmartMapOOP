#pragma once
#include <vector>

namespace SmartMap {
namespace Graph {

class RouteResult {
private:
    std::vector<int> fullPath;
    std::vector<int> destinationOrder;
    double totalDistance;

public:
    RouteResult(const std::vector<int>& fullPath,
                const std::vector<int>& destinationOrder,
                double totalDistance);

    std::vector<int> GetFullPath() const;
    std::vector<int> GetDestinationOrder() const;
    double GetTotalDistance() const;
};

} // namespace Graph
} // namespace SmartMap

