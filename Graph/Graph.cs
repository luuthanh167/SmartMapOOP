using System;
using System.Collections.Generic;
using SmartMap.Models;

namespace SmartMap.Graph;

public class Graph
{
    private Dictionary<int, Location> locations;
    private Dictionary<int, List<Edge>> adjacencyList;

    public Graph()
    {
        locations = new Dictionary<int, Location>();
        adjacencyList = new Dictionary<int, List<Edge>>();
    }

    public void AddLocation(Location location)
    {
        int id = location.GetId();

        if (!locations.ContainsKey(id))
        {
            locations[id] = location;
            adjacencyList[id] = new List<Edge>();
        }
    }

    public void AddEdge(int from, int to, double distance)
    {
        if (!adjacencyList.ContainsKey(from) || !adjacencyList.ContainsKey(to))
        {
            return;
        }

        adjacencyList[from].Add(
            new Edge(to, distance)
        );

        adjacencyList[to].Add(
            new Edge(from, distance)
        );
    }

    public List<Edge> GetNeighbors(int locationId)
    {
        if (adjacencyList.TryGetValue(locationId, out var neighbors))
        {
            return neighbors;
        }
        return new List<Edge>();
    }

    public Location? GetLocation(int id)
    {
        return locations.TryGetValue(id, out var loc) ? loc : null;
    }

    public List<Location> GetLocations()
    {
        return new List<Location>(locations.Values);
    }

    public double GetDistance(int from, int to)
    {
        if (!adjacencyList.TryGetValue(from, out var edges))
        {
            return double.PositiveInfinity;
        }

        foreach (var edge in edges)
        {
            if (edge.GetTo() == to)
            {
                return edge.GetDistance();
            }
        }
        return double.PositiveInfinity;
    }
}