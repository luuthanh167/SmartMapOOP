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
    }
}