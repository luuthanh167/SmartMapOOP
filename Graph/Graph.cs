using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
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
        adjacencyList[from].Add(
            new Edge(to, distance)
        );

        adjacencyList[to].Add(
            new Edge(from, distance)
        );
    }

    public List<Edge> GetNeighbors(int locationId)
    {
        return adjacencyList[locationId];
    }

    public Location GetLocation(int id)
    {
        return locations[id];
    }

    public List<Location> GetLocations()
    {
        return new List<Location>(locations.Values);
    }

    public double GetDistance(int from, int to)
    {
        foreach (var edge in adjacencyList[from])
        {
            if (edge.GetTo() == to)
            {
                return edge.GetDistance();
            }
        }
        return double.PositiveInfinity;
    }
}