using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net.Http;
using System.Text.Json;
using System.Threading.Tasks;
using SmartMap.Models;

namespace SmartMap.Service;

public class RoutingService
{
    private readonly HttpClient httpClient;

    public RoutingService()
    {
        httpClient = new HttpClient();
        httpClient.Timeout = TimeSpan.FromSeconds(10);
        httpClient.DefaultRequestHeaders.UserAgent.ParseAdd(
            "SmartMapOOP/1.0 (Contact: user@smartmap.local)"
        );
    }

    public async Task<RoadRoute?> GetRoute(
        Position start,
        Position destination)
    {
        var waypoints = new List<RouteWaypoint>
        {
            new RouteWaypoint
            {
                Name = "Điểm đến",
                Latitude = destination.GetLatitude(),
                Longitude = destination.GetLongitude()
            }
        };

        return await GetMultiRoute(start, "Vị trí xuất phát", waypoints, false);
    }

    public async Task<RoadRoute?> GetMultiRoute(
        Position start,
        string startName,
        List<RouteWaypoint> waypoints,
        bool optimizeOrder = false)
    {
        if (waypoints == null || waypoints.Count == 0)
        {
            return null;
        }

        // 1. Tối ưu hóa thứ tự ghé thăm (Toán rời rạc - Bài toán Người du lịch TSP)
        List<RouteWaypoint> orderedWaypoints;
        if (optimizeOrder && waypoints.Count > 1)
        {
            orderedWaypoints = FindOptimalOrder(start, waypoints);
        }
        else
        {
            orderedWaypoints = new List<RouteWaypoint>(waypoints);
        }

        // 2. Gom toàn bộ điểm: Điểm xuất phát -> Điểm 1 -> Điểm 2 -> Điểm 3...
        var allPositions = new List<(string Name, double Lat, double Lon)>
        {
            (string.IsNullOrWhiteSpace(startName) ? "Vị trí của bạn" : startName, start.GetLatitude(), start.GetLongitude())
        };

        foreach (var wp in orderedWaypoints)
        {
            allPositions.Add((wp.Name, wp.Latitude, wp.Longitude));
        }

        // 3. Xây dựng URL OSRM (kinh độ, vĩ độ ngăn cách bằng dấu chấm phẩy)
        var coordStrings = allPositions.Select(p =>
            $"{p.Lon.ToString(CultureInfo.InvariantCulture)},{p.Lat.ToString(CultureInfo.InvariantCulture)}"
        );
        string coordinatesParam = string.Join(";", coordStrings);

        string url =
            $"https://router.project-osrm.org/route/v1/driving/{coordinatesParam}" +
            "?overview=full&geometries=geojson&steps=false";

        try
        {
            HttpResponseMessage response = await httpClient.GetAsync(url);

            if (response.IsSuccessStatusCode)
            {
                string json = await response.Content.ReadAsStringAsync();

                using JsonDocument document = JsonDocument.Parse(json);
                JsonElement root = document.RootElement;

                if (root.TryGetProperty("code", out var codeElem) && codeElem.GetString() == "Ok")
                {
                    JsonElement route = root.GetProperty("routes")[0];

                    double distance = route.GetProperty("distance").GetDouble();
                    double duration = route.GetProperty("duration").GetDouble();

                    JsonElement coordinates = route
                        .GetProperty("geometry")
                        .GetProperty("coordinates");

                    List<double[]> routeCoordinates = new List<double[]>();

                    foreach (JsonElement point in coordinates.EnumerateArray())
                    {
                        double longitude = point[0].GetDouble();
                        double latitude = point[1].GetDouble();

                        routeCoordinates.Add(new double[] { longitude, latitude });
                    }

                    // Tách thông tin từng chặng (Legs)
                    List<RouteLeg> legs = new List<RouteLeg>();
                    if (route.TryGetProperty("legs", out var legsElem))
                    {
                        int legIndex = 0;
                        foreach (JsonElement leg in legsElem.EnumerateArray())
                        {
                            double legDist = leg.GetProperty("distance").GetDouble();
                            double legDur = leg.GetProperty("duration").GetDouble();

                            string from = legIndex < allPositions.Count ? allPositions[legIndex].Name : $"Điểm {legIndex}";
                            string to = (legIndex + 1) < allPositions.Count ? allPositions[legIndex + 1].Name : $"Điểm {legIndex + 1}";

                            legs.Add(new RouteLeg
                            {
                                FromName = from,
                                ToName = to,
                                DistanceKm = Math.Round(legDist / 1000.0, 2),
                                DurationMin = Math.Round(legDur / 60.0, 1)
                            });

                            legIndex++;
                        }
                    }

                    return new RoadRoute(distance, duration, routeCoordinates, orderedWaypoints, legs);
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[RoutingService] OSRM multi-route query failed: {ex.Message}. Falling back to straight-line path.");
        }

        // Fallback: Haversine distance & straight line coordinates cho từng chặng
        DistanceService distanceService = new DistanceService();
        double totalDistanceKm = 0;
        List<double[]> fallbackCoordinates = new List<double[]>();
        List<RouteLeg> fallbackLegs = new List<RouteLeg>();

        for (int i = 0; i < allPositions.Count; i++)
        {
            fallbackCoordinates.Add(new double[] { allPositions[i].Lon, allPositions[i].Lat });

            if (i < allPositions.Count - 1)
            {
                var posA = new Position(allPositions[i].Lat, allPositions[i].Lon);
                var posB = new Position(allPositions[i + 1].Lat, allPositions[i + 1].Lon);

                double dKm = distanceService.CalculateDistance(posA, posB);
                totalDistanceKm += dKm;

                fallbackLegs.Add(new RouteLeg
                {
                    FromName = allPositions[i].Name,
                    ToName = allPositions[i + 1].Name,
                    DistanceKm = Math.Round(dKm, 2),
                    DurationMin = Math.Round(dKm * 2.0, 1)
                });
            }
        }

        return new RoadRoute(
            totalDistanceKm * 1000.0,
            totalDistanceKm * 120.0,
            fallbackCoordinates,
            orderedWaypoints,
            fallbackLegs
        );
    }

    /// <summary>
    /// Thuật toán Toán Rời Rạc: Tìm hoán vị có tổng quãng đường ngắn nhất xuất phát từ Start (TSP / Hamiltonian Path)
    /// </summary>
    private List<RouteWaypoint> FindOptimalOrder(Position start, List<RouteWaypoint> waypoints)
    {
        int n = waypoints.Count;
        var permutations = GeneratePermutations(n);
        DistanceService distanceService = new DistanceService();

        List<int>? bestPermutation = null;
        double minTotalDistance = double.MaxValue;

        foreach (var perm in permutations)
        {
            double currentDist = 0;
            Position currentPos = start;

            foreach (int index in perm)
            {
                var nextWp = waypoints[index];
                var nextPos = new Position(nextWp.Latitude, nextWp.Longitude);
                currentDist += distanceService.CalculateDistance(currentPos, nextPos);
                currentPos = nextPos;
            }

            if (currentDist < minTotalDistance)
            {
                minTotalDistance = currentDist;
                bestPermutation = perm;
            }
        }

        if (bestPermutation == null)
        {
            return new List<RouteWaypoint>(waypoints);
        }

        return bestPermutation.Select(idx => waypoints[idx]).ToList();
    }

    private static List<List<int>> GeneratePermutations(int n)
    {
        var result = new List<List<int>>();
        var current = new List<int>();
        var used = new bool[n];

        void Backtrack()
        {
            if (current.Count == n)
            {
                result.Add(new List<int>(current));
                return;
            }

            for (int i = 0; i < n; i++)
            {
                if (!used[i])
                {
                    used[i] = true;
                    current.Add(i);
                    Backtrack();
                    used[i] = false;
                    current.RemoveAt(current.Count - 1);
                }
            }
        }

        Backtrack();
        return result;
    }
}

public class RouteLeg
{
    public string FromName { get; set; } = "";
    public string ToName { get; set; } = "";
    public double DistanceKm { get; set; }
    public double DurationMin { get; set; }
}

public class RoadRoute
{
    public double Distance { get; }
    public double Duration { get; }
    public List<double[]> RouteCoordinates { get; }
    public List<RouteWaypoint>? OrderedWaypoints { get; }
    public List<RouteLeg>? Legs { get; }

    public RoadRoute(
        double distance,
        double duration,
        List<double[]> routeCoordinates,
        List<RouteWaypoint>? orderedWaypoints = null,
        List<RouteLeg>? legs = null)
    {
        Distance = distance;
        Duration = duration;
        RouteCoordinates = routeCoordinates;
        OrderedWaypoints = orderedWaypoints;
        Legs = legs;
    }
}