using System.Collections.Generic;

namespace SmartMap.Graph;

public class RouteResult
{
    private List<int> fullPath;
    private List<int> destinationOrder;
    private double totalDistance;

    public RouteResult(
        List<int> fullPath,
        List<int> destinationOrder,
        double totalDistance)
    {
        this.fullPath = fullPath;
        this.destinationOrder = destinationOrder;
        this.totalDistance = totalDistance;
    }

    public List<int> GetFullPath()
    {
        return fullPath;
    }

    public List<int> GetDestinationOrder()
    {
        return destinationOrder;
    }

    public double GetTotalDistance()
    {
        return totalDistance;
    }
}