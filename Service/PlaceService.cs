using System.Text.Json;
using SmartMap.Models;

namespace SmartMap.Service;

public class PlaceService
{
    private readonly HttpClient httpClient;

    public PlaceService()
    {
        httpClient = new HttpClient();

        // Overpass/OSM cần User-Agent xác định ứng dụng
        httpClient.DefaultRequestHeaders.UserAgent.ParseAdd(
            "SmartMapOOP/1.0"
        );
    }

    public async Task<List<Location>> SearchPlaces(
        double latitude,
        double longitude,
        double radius,
        DestinationType type)
    {
        string tag = GetTag(type);

        string query = $"""
        [out:json][timeout:25];
        (
          nwr["{tag.Split('=')[0]}"="{tag.Split('=')[1]}"]
          (around:{radius},{latitude},{longitude});
        );
        out center;
        """;

        var content = new FormUrlEncodedContent(
            new[]
            {
                new KeyValuePair<string, string>("data", query)
            }
        );

        HttpResponseMessage response = await httpClient.PostAsync(
            "https://overpass-api.de/api/interpreter",
            content
        );

        if (!response.IsSuccessStatusCode)
        {
            Console.WriteLine(
                $"Overpass API error: {(int)response.StatusCode} {response.ReasonPhrase}"
            );

            return new List<Location>();
        }

        string json = await response.Content.ReadAsStringAsync();

        return ConvertToLocations(json, type);
    }

    private string GetTag(DestinationType type)
    {
        switch (type)
        {
            case DestinationType.FOOD:
                return "amenity=restaurant";

            case DestinationType.COFFEE:
                return "amenity=cafe";

            case DestinationType.SHOPPING:
                return "shop=mall";

            case DestinationType.STUDY:
                return "amenity=library";

            case DestinationType.RELAX:
                return "leisure=park";

            case DestinationType.ENTERTAINMENT:
                return "amenity=cinema";

            default:
                return "amenity=restaurant";
        }
    }

    private List<Location> ConvertToLocations(
        string json,
        DestinationType type)
    {
        List<Location> locations = new List<Location>();

        using JsonDocument document = JsonDocument.Parse(json);

        JsonElement elements = document.RootElement
            .GetProperty("elements");

        int id = 1;

        foreach (JsonElement element in elements.EnumerateArray())
        {
            string name = "Unknown";

            if (element.TryGetProperty("tags", out JsonElement tags))
            {
                if (tags.TryGetProperty("name", out JsonElement nameElement))
                {
                    name = nameElement.GetString() ?? "Unknown";
                }
            }

            double latitude;
            double longitude;

            if (element.TryGetProperty("lat", out JsonElement latElement))
            {
                latitude = latElement.GetDouble();
                longitude = element.GetProperty("lon").GetDouble();
            }
            else
            {
                JsonElement center = element.GetProperty("center");

                latitude = center.GetProperty("lat").GetDouble();
                longitude = center.GetProperty("lon").GetDouble();
            }

            Position position = new Position(
                latitude,
                longitude
            );

            Location location = new Location(
                id,
                name,
                "",
                position,
                type,
                0,
                element.GetProperty("id").ToString()
            );

            locations.Add(location);

            id++;
        }

        return locations;
    }
}