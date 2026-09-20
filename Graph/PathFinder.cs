using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace SmartMap.Graph;
public class PathFinder
{
    private Graph graph;
    public PathFinder(Graph graph)
    {
        this.graph = graph;
    }

//BFS
    public List<int> BFS(int start, int target)
    {
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

        if(!visited.Contains(target))
        {
            return new List<int>(); // No path found
        }

        // Reconstruct the path
        List<int> path = new List<int>();
        int currentVertex = target;
        while (currentVertex != start)
        {
            path.Add(currentVertex);
            currentVertex = parent[currentVertex];
        }
        path.Add(start);
        path.Reverse();
        return path;
    }


//DFS
    public List<int> DFS(int start, int target)
    {
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
            currentVertex = parent[currentVertex];
        }
        path.Add(start);
        path.Reverse();
        return path;
    }

    //Supporting method for DFS to perform the recursive search
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


//Dijkstra
    public List<int> Dijkstra(int start, int target, out double totalDistance)
    {
        List<int> vertices = new List<int>();

        foreach(var Location in graph.GetLocations())
        {
            vertices.Add(Location.GetId());
        }
        Dictionary<int, double> distances = new Dictionary<int, double>();
        Dictionary<int, int> parent = new Dictionary<int, int>();

        HashSet<int> visited = new HashSet<int>();
        
        //Khởi tạo khoảng cách từ start đến tất cả các đỉnh là vô cùng, ngoại trừ start có khoảng cách bằng 0
        foreach (var vertex in vertices)
        {
            distances[vertex] = double.PositiveInfinity;
        }

        if(start == target)
        {
            totalDistance = 0;
            return new List<int> { start };
        }
        distances[start] = 0;

        while (visited.Count < vertices.Count)
        {
            int currentVertex = -1;
            double minDistance = double.PositiveInfinity;
            //Tim đỉnh chưa được thăm có khoảng cách nhỏ nhất từ start

            foreach (var vertex in vertices)
            {
                if (!visited.Contains(vertex) && distances[vertex] < minDistance)
                {
                    minDistance = distances[vertex];
                    currentVertex = vertex;
                }
            }

            //Không còn đỉnh nào có thể tiếp cận được từ start
            if (currentVertex == -1)
            {
                break; 
            }

            visited.Add(currentVertex);

            //Xét các đỉnh kề
            foreach (var edge in graph.GetNeighbors(currentVertex))
            {
                int neighbor = edge.GetTo();
                double distanceThroughCurrent = distances[currentVertex] + edge.GetDistance();

                if (distanceThroughCurrent < distances[neighbor])
                {
                    distances[neighbor] = distanceThroughCurrent;
                    parent[neighbor] = currentVertex;
                }
            }
        }
        totalDistance = distances[target];

        //Không tìm thấy đường đi từ start đến target
        if (double.IsPositiveInfinity(totalDistance))
        {
            return new List<int>();
        }
        return ReconstructPath(parent, start, target);
    }

    //Supporting method to reconstruct the path from start to target using the parent dictionary
    private List<int> ReconstructPath( Dictionary<int, int> parent, int start, int target)
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
            currentVertex = parent[currentVertex];
        }

        path.Add(start);
        path.Reverse();

        return path;
    }
}
