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
        double startLat = start.GetLatitude();
        double startLon = start.GetLongitude();

        double destLat = destination.GetLatitude();
        double destLon = destination.GetLongitude();

        string startLonStr = startLon.ToString(CultureInfo.InvariantCulture);
        string startLatStr = startLat.ToString(CultureInfo.InvariantCulture);
        string destLonStr = destLon.ToString(CultureInfo.InvariantCulture);
        string destLatStr = destLat.ToString(CultureInfo.InvariantCulture);

        // OSRM dùng thứ tự: longitude,latitude
        string url =
            $"https://router.project-osrm.org/route/v1/driving/" +
            $"{startLonStr},{startLatStr};{destLonStr},{destLatStr}" +
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

                    return new RoadRoute(distance, duration, routeCoordinates);
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[RoutingService] OSRM query failed: {ex.Message}. Falling back to straight-line path.");
        }

        // Fallback: Haversine distance & straight line coordinates
        DistanceService distanceService = new DistanceService();
        double directDistKm = distanceService.CalculateDistance(start, destination);

        return new RoadRoute(
            directDistKm * 1000.0,
            directDistKm * 120.0,
            new List<double[]>
            {
                new double[] { startLon, startLat },
                new double[] { destLon, destLat }
            }
        );
    }
}

public class RoadRoute
{
    public double Distance { get; }
    public double Duration { get; }
    public List<double[]> RouteCoordinates { get; }

    public RoadRoute(
        double distance,
        double duration,
        List<double[]> routeCoordinates)
    {
        Distance = distance;
        Duration = duration;
        RouteCoordinates = routeCoordinates;
    }
}