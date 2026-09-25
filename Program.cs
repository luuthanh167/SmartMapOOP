using System;
using System.Collections.Generic;
using SmartMap.Models;
using SmartMap.Graph;
using SmartMap.Service;

class Program
{
    static async Task Main()
    {
        // ==========================================
        // 1. Tạo User
        // ==========================================

        User user = new User("Thanh");

        Position currentPosition = new Position(
            10.8498,
            106.7713
        );

        user.SetCurrentPosition(currentPosition);


        // ==========================================
        // 2. Lấy địa điểm thật từ OpenStreetMap
        // ==========================================

        PlaceService placeService = new PlaceService();

        List<Location> places = await placeService.SearchPlaces(
            10.8498,
            106.7713,
            2000,
            DestinationType.COFFEE
        );


        // ==========================================
        // 3. In các địa điểm tìm được
        // ==========================================

        Console.WriteLine("=== REAL PLACES ===");

        foreach (Location location in places)
        {
            Console.WriteLine(
                location.GetId()
                + " - "
                + location.GetName()
                + " - "
                + location.GetPosition().GetLatitude()
                + ", "
                + location.GetPosition().GetLongitude()
            );
        }


        // ==========================================
        // 4. Tạo Graph
        // ==========================================

        Graph graph = new Graph();

        foreach (Location location in places)
        {
            graph.AddLocation(location);
        }


        // ==========================================
        // 5. Tạo Edge bằng khoảng cách
        // ==========================================

        DistanceService distanceService = new DistanceService();

        for (int i = 0; i < places.Count; i++)
        {
            for (int j = i + 1; j < places.Count; j++)
            {
                double distance = distanceService.CalculateDistance(
                    places[i].GetPosition(),
                    places[j].GetPosition()
                );

                graph.AddEdge(
                    places[i].GetId(),
                    places[j].GetId(),
                    distance
                );
            }
        }


        // ==========================================
        // 6. Kiểm tra Graph
        // ==========================================

        Console.WriteLine();
        Console.WriteLine("=== GRAPH ===");

        foreach (Location location in places)
        {
            Console.WriteLine(
                "Location "
                + location.GetId()
                + ": "
                + location.GetName()
            );

            List<Edge> neighbors =
                graph.GetNeighbors(location.GetId());

            foreach (Edge edge in neighbors)
            {
                Console.WriteLine(
                    "  -> Location "
                    + edge.GetTo()
                    + " | Distance: "
                    + edge.GetDistance()
                );
            }
        }
        
        PathFinder pathFinder = new PathFinder(graph);
        // double totalDistance;
        // List <int> path = pathFinder.Dijkstra(1, 9, out totalDistance);
        // Console.WriteLine();
        // Console.WriteLine("===Dijkstra Test===");
        // Console.WriteLine("Path: " + string.Join("->", path));
        // Console.WriteLine("Total distance: " + totalDistance.ToString("F2") + "km");

        // ==========================================
        // 7. User chọn các địa điểm
        // ==========================================

        Console.WriteLine();
        Console.WriteLine("===CHOOSE START LOCATION==="); //chọn địa điểm bắt đầu
        Console.WriteLine("> ");
        string? startInput = Console.ReadLine();
        if (!int.TryParse(startInput, out int startId))
        {
            Console.WriteLine("Invalid start location");
            return;
        }

        Location? startLocation = places.Find(x => x.GetId() == startId);

        if (startLocation == null)
        {
            Console.WriteLine("Start location not found");
            return;
        }
        Console.WriteLine();
        Console.WriteLine("=== CHOOSE DESTINATIONS==="); //chọn những địa điểm muốn đến
        Console.WriteLine("Enter destination IDs separated by spaces:");
        Console.Write("> ");

        string? input = Console.ReadLine();

        if (string.IsNullOrWhiteSpace(input))
        {
            Console.WriteLine("No locations selected.");
            return;
        }

        string[] inputIds = input.Split(
            ' ',
            StringSplitOptions.RemoveEmptyEntries
        );

        List<Location> selectedPlaces = new List<Location>();

        foreach (string idText in inputIds)
        {
            if (int.TryParse(idText, out int id))
            {
                Location? location = places.Find(
                    x => x.GetId() == id
                );

                if (location != null &&
                    !selectedPlaces.Contains(location))
                {
                    selectedPlaces.Add(location);
                }
            }
        }

        // ==========================================
        // 9. In các địa điểm user đã chọn
        // ==========================================
        Console.WriteLine();
        Console.WriteLine("===START LOCATIONS===");
        Console.WriteLine(startLocation.GetId() + " - " + startLocation.GetName());
        Console.WriteLine();
        Console.WriteLine("=== SELECTED DESTINATIONS ===");

        foreach (Location location in selectedPlaces)
        {
            Console.WriteLine(
                location.GetId()
                + " - "
                + location.GetName()
            );
        }
        // // ==========================================
        // // 10. Tìm đường đi ngắn nhất qua các địa điểm
        // // ==========================================

        // if (selectedPlaces.Count >= 2)
        // {
        //     Console.WriteLine();
        //     Console.WriteLine("=== SHORTEST ROUTE ===");

        //     double totalRouteDistance = 0;

        //     int currentLocation =
        //         selectedPlaces[0].GetId();

        //     Console.WriteLine(
        //         "Start: "
        //         + selectedPlaces[0].GetName()
        //     );

        //     for (int i = 1; i < selectedPlaces.Count; i++)
        //     {
        //         int nextLocation =
        //             selectedPlaces[i].GetId();

        //         double distance;

        //         List<int> path =
        //             pathFinder.Dijkstra(
        //                 currentLocation,
        //                 nextLocation,
        //                 out distance
        //             );

        //         if (path.Count == 0)
        //         {
        //             Console.WriteLine(
        //                 "No path found from "
        //                 + currentLocation
        //                 + " to "
        //                 + nextLocation
        //             );

        //             return;
        //         }

        //         Console.WriteLine();
        //         Console.WriteLine(
        //             "From "
        //             + currentLocation
        //             + " to "
        //             + nextLocation
        //         );

        //         Console.Write("Path: ");

        //         foreach (int id in path)
        //         {
        //             Console.Write(id + " ");
        //         }

        //         Console.WriteLine();
        //         Console.WriteLine(
        //             "Distance: "
        //             + distance
        //             + " km"
        //         );

        //         totalRouteDistance += distance;

        //         currentLocation = nextLocation;
        //     }

        //     Console.WriteLine();
        //     Console.WriteLine(
        //         "Total Route Distance: "
        //         + totalRouteDistance
        //         + " km"
        //     );
        // }
        // ==========================================
        // 10. Tối ưu thứ tự các địa điểm (in ra thứ tự đi cùng với khoảng cách tối ưu)
        // ==========================================

        List<int> selectedIds = new List<int>();

        foreach (Location location in selectedPlaces)
        {
            selectedIds.Add(location.GetId());
        }

        RouteOptimizer routeOptimizer =
            new RouteOptimizer(graph);

        RouteResult routeResult =
            routeOptimizer.FindOptimalRoute(
                selectedIds[0],
                selectedIds
            );

        Console.WriteLine();
        Console.WriteLine("=== OPTIMAL ROUTE ===");

        Console.WriteLine("Best Order:");

        foreach (int id in routeResult.GetDestinationOrder())
        {
            Console.Write(id + " ");
        }

        Console.WriteLine();

        Console.WriteLine(
            "Minimum Distance: "
            + routeResult.GetTotalDistance()
            + " km"
        );
    }
}