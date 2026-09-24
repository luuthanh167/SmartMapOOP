using System;
using System.Collections.Generic;

namespace SmartMap.Graph;

public class RouteOptimizer
{
    private SmartMap.Graph.Graph graph;
    private PathFinder pathFinder;

    public RouteOptimizer(SmartMap.Graph.Graph graph)
    {
        this.graph = graph;
        this.pathFinder = new PathFinder(graph);
    }

    public RouteResult FindOptimalRoute(
        int start,
        List<int> destinations)
    {
        // Kiểm tra dữ liệu đầu vào
        if (destinations == null)
        {
            throw new ArgumentNullException(
                nameof(destinations));
        }

        // Không có destination
        if (destinations.Count == 0)
        {
            return new RouteResult(
                new List<int> { start },
                new List<int>(),
                0
            );
        }

        // Kiểm tra destination trùng nhau
        HashSet<int> uniqueDestinations =
            new HashSet<int>(destinations);

        if (uniqueDestinations.Count != destinations.Count)
        {
            throw new ArgumentException(
                "Destinations must not contain duplicate locations."
            );
        }

        // Nếu chỉ có một destination
        if (destinations.Count == 1)
        {
            double distance;

            List<int> path =
                pathFinder.Dijkstra(
                    start,
                    destinations[0],
                    out distance);

            if (path.Count == 0)
            {
                return new RouteResult(
                    new List<int>(),
                    new List<int>(),
                    double.PositiveInfinity
                );
            }

            return new RouteResult(
                path,
                new List<int>(destinations),
                distance
            );
        }

        //Sinh tất cả thứ tự có thể của các điểm đến
        List<List<int>> permutations =
            GeneratePermutations(destinations);

        double bestDistance =
            double.PositiveInfinity;

        List<int> bestOrder =
            new List<int>();


        //Tính khoảng cách cho từng thứ tự
        foreach (List<int> order in permutations)
        {
            double currentDistance =
                CalculateRouteDistance(
                    start,
                    order);

            // Nếu route này tốt hơn route hiện tại
            if (currentDistance < bestDistance)
            {
                bestDistance = currentDistance;
                bestOrder = new List<int>(order);
            }
        }

        //Không tìm được route
        if (double.IsPositiveInfinity(bestDistance))
        {
            return new RouteResult(
                new List<int>(),
                new List<int>(),
                double.PositiveInfinity
            );
        }
        //Xây dựng đường đi đầy đủ
        List<int> fullPath =
            BuildFullPath(
                start,
                bestOrder);

        return new RouteResult(
            fullPath,
            bestOrder,
            bestDistance
        );
    }

    // =========================================================
    // Generate all possible destination orders
    // =========================================================
    private List<List<int>> GeneratePermutations(
        List<int> destinations)
    {
        List<List<int>> result =
            new List<List<int>>();

        List<int> current =
            new List<int>(destinations);

        GeneratePermutationsRecursive(
            current,
            0,
            result);

        return result;
    }

    // =========================================================
    // Recursive permutation helper
    // =========================================================
    private void GeneratePermutationsRecursive(
        List<int> destinations,
        int index,
        List<List<int>> result)
    {
        // Đã tạo xong một permutation
        if (index == destinations.Count)
        {
            result.Add(
                new List<int>(destinations)
            );

            return;
        }

        for (int i = index;
             i < destinations.Count;
             i++)
        {
            // Swap
            Swap(
                destinations,
                index,
                i);

            // Đệ quy
            GeneratePermutationsRecursive(
                destinations,
                index + 1,
                result);

            // Swap lại để khôi phục trạng thái
            Swap(
                destinations,
                index,
                i);
        }
    }

    // =========================================================
    // Swap two elements
    // =========================================================
    private void Swap(
        List<int> list,
        int first,
        int second)
    {
        int temp = list[first];

        list[first] = list[second];
        list[second] = temp;
    }

    // =========================================================
    // Calculate total distance of one route
    // =========================================================
    private double CalculateRouteDistance(
        int start,
        List<int> route)
    {
        double totalDistance = 0;

        int current = start;

        foreach (int destination in route)
        {
            double distance;

            List<int> path =
                pathFinder.Dijkstra(
                    current,
                    destination,
                    out distance);

            // Không có đường đi
            if (path.Count == 0)
            {
                return double.PositiveInfinity;
            }

            totalDistance += distance;

            current = destination;
        }

        return totalDistance;
    }

    // =========================================================
    // Build complete path
    // =========================================================
    private List<int> BuildFullPath(
        int start,
        List<int> destinationOrder)
    {
        List<int> fullPath =
            new List<int>();

        int current = start;

        // Thêm điểm bắt đầu
        fullPath.Add(start);

        foreach (int destination in destinationOrder)
        {
            double distance;

            List<int> path =
                pathFinder.Dijkstra(
                    current,
                    destination,
                    out distance);

            // Không có đường đi
            if (path.Count == 0)
            {
                return new List<int>();
            }

            for (int i = 1;
                 i < path.Count;
                 i++)
            {
                fullPath.Add(path[i]);
            }

            current = destination;
        }

        return fullPath;
    }
}