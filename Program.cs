using System;
using System.Collections.Generic;
using SmartMap.Models;
using SmartMap.Graph;

class Program
{
    static void Main()
    {
        // 1. Tạo User
        User user = new User("Thanh");

        // 2. Tạo vị trí hiện tại của User
        Position currentPosition = new Position(0, 0);
        user.SetCurrentPosition(currentPosition);

        // 3. Tạo một vài địa điểm
        Location food = new Location(
            1,
            "Quan Com",
            "123 ABC",
            new Position(1, 1),
            DestinationType.FOOD,
            50000,
            "food001"
        );

        Location coffee = new Location(
            2,
            "Coffee Shop",
            "456 DEF",
            new Position(2, 2),
            DestinationType.COFFEE,
            30000,
            "coffee001"
        );

        Location shopping = new Location(
            3,
            "Shopping Mall",
            "789 GHI",
            new Position(3, 3),
            DestinationType.SHOPPING,
            100000,
            "shop001"
        );

        // 4. Tạo Graph
        Graph graph = new Graph();

        graph.AddLocation(food);
        graph.AddLocation(coffee);
        graph.AddLocation(shopping);

        // 5. Tạo đường đi giữa các địa điểm
        graph.AddEdge(1, 2, 2);
        graph.AddEdge(2, 3, 3);
        graph.AddEdge(1, 3, 5);

        // 6. Tạo RouteOptimizer
        RouteOptimizer optimizer = new RouteOptimizer(graph);

        // 7. Tìm đường từ Food -> Shopping
        List<int> destinations = new List<int> { 3 };

        RouteResult result = optimizer.FindOptimalRoute(1, destinations);

        // 8. In kết quả
        Console.WriteLine("=== SMARTMAP ===");
        Console.WriteLine("User: " + user.GetName());

        Console.WriteLine("Route:");

        foreach (int id in result.GetFullPath())
        {
            Location location = graph.GetLocation(id);
            Console.WriteLine("- " + location.GetName());
        }

        Console.WriteLine("Total distance: " + result.GetTotalDistance());
    }
}