using System;
using System.Collections.Generic;

namespace SmartMap.Graph;

public class PathFinder
{
    private Graph graph;

    public PathFinder(Graph graph)
    {
        this.graph = graph;
    }

    // BFS
    public List<int> BFS(int start, int target)
    {
        if (graph.GetLocation(start) == null || graph.GetLocation(target) == null)
        {
            return new List<int>();
        }

        if (start == target)
        {
            return new List<int> { start };
        }

        Queue<int> queue = new Queue<int>();
        HashSet<int> visited = new HashSet<int>();
        Dictionary<int, int> parent = new Dictionary<int, int>();

        queue.Enqueue(start);
        visited.Add(start);

        while (queue.Count > 0)
        {
            int current = queue.Dequeue();
            if (current == target)
            {
                break;
            }

            foreach (var edge in graph.GetNeighbors(current))
            {
                int neighbor = edge.GetTo();
                if (!visited.Contains(neighbor))
                {
                    visited.Add(neighbor);
                    parent[neighbor] = current;
                    queue.Enqueue(neighbor);
                }
            }
        }

        if (!visited.Contains(target))
        {
            return new List<int>(); // No path found
        }

        // Reconstruct the path
        List<int> path = new List<int>();
        int currentVertex = target;
        while (currentVertex != start)
        {
            path.Add(currentVertex);
            if (!parent.ContainsKey(currentVertex))
            {
                return new List<int>();
            }
            currentVertex = parent[currentVertex];
        }
        path.Add(start);
        path.Reverse();
        return path;
    }

    // DFS
    public List<int> DFS(int start, int target)
    {
        if (graph.GetLocation(start) == null || graph.GetLocation(target) == null)
        {
            return new List<int>();
        }

        if (start == target)
        {
            return new List<int> { start };
        }

        HashSet<int> visited = new HashSet<int>();
        Dictionary<int, int> parent = new Dictionary<int, int>();
        DFSUtil(start, target, visited, parent);

        if (!visited.Contains(target))
        {
            return new List<int>(); // No path found
        }

        List<int> path = new List<int>();
        int currentVertex = target;
        while (currentVertex != start)
        {
            path.Add(currentVertex);
            if (!parent.ContainsKey(currentVertex))
            {
                return new List<int>();
            }
            currentVertex = parent[currentVertex];
        }
        path.Add(start);
        path.Reverse();
        return path;
    }

    // Supporting method for DFS to perform the recursive search
    private bool DFSUtil(int current, int target, HashSet<int> visited, Dictionary<int, int> parent)
    {
        visited.Add(current);
        if (current == target)
        {
            return true;
        }

        foreach (var edge in graph.GetNeighbors(current))
        {
            int neighbor = edge.GetTo();
            if (!visited.Contains(neighbor))
            {
                parent[neighbor] = current;
                if (DFSUtil(neighbor, target, visited, parent))
                {
                    return true;
                }
            }
        }
        return false;
    }

    // Dijkstra
    public List<int> Dijkstra(int start, int target, out double totalDistance)
    {
        List<int> vertices = new List<int>();

        foreach (var loc in graph.GetLocations())
        {
            vertices.Add(loc.GetId());
        }

        if (!vertices.Contains(start) || !vertices.Contains(target))
        {
            totalDistance = double.PositiveInfinity;
            return new List<int>();
        }

        if (start == target)
        {
            totalDistance = 0;
            return new List<int> { start };
        }

        Dictionary<int, double> distances = new Dictionary<int, double>();
        Dictionary<int, int> parent = new Dictionary<int, int>();
        HashSet<int> visited = new HashSet<int>();

        foreach (var vertex in vertices)
        {
            distances[vertex] = double.PositiveInfinity;
        }

        distances[start] = 0;

        while (visited.Count < vertices.Count)
        {
            int currentVertex = -1;
            double minDistance = double.PositiveInfinity;

            foreach (var vertex in vertices)
            {
                if (!visited.Contains(vertex) && distances[vertex] < minDistance)
                {
                    minDistance = distances[vertex];
                    currentVertex = vertex;
                }
            }

            if (currentVertex == -1)
            {
                break;
            }

            visited.Add(currentVertex);

            if (currentVertex == target)
            {
                break;
            }

            foreach (var edge in graph.GetNeighbors(currentVertex))
            {
                int neighbor = edge.GetTo();
                if (distances.ContainsKey(neighbor))
                {
                    double distanceThroughCurrent = distances[currentVertex] + edge.GetDistance();

                    if (distanceThroughCurrent < distances[neighbor])
                    {
                        distances[neighbor] = distanceThroughCurrent;
                        parent[neighbor] = currentVertex;
                    }
                }
            }
        }

        totalDistance = distances.TryGetValue(target, out double d) ? d : double.PositiveInfinity;

        if (double.IsPositiveInfinity(totalDistance))
        {
            return new List<int>();
        }

        return ReconstructPath(parent, start, target);
    }

    // Supporting method to reconstruct the path from start to target using the parent dictionary
    private List<int> ReconstructPath(Dictionary<int, int> parent, int start, int target)
    {
        List<int> path = new List<int>();

        if (start == target)
        {
            path.Add(start);
            return path;
        }

        if (!parent.ContainsKey(target))
        {
            return path;
        }

        int currentVertex = target;

        while (currentVertex != start)
        {
            path.Add(currentVertex);
            if (!parent.ContainsKey(currentVertex))
            {
                return new List<int>();
            }
            currentVertex = parent[currentVertex];
        }

        path.Add(start);
        path.Reverse();

        return path;
    }
}
