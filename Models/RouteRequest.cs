namespace SmartMap.Models;

public class RouteWaypoint
{
    public string Name { get; set; } = "";
    public double Latitude { get; set; }
    public double Longitude { get; set; }
}

public class RouteRequest
{
    public string StartName { get; set; } = "";
    public string DestinationName { get; set; } = "";
    public double? StartLat { get; set; }
    public double? StartLon { get; set; }
    public double? DestLat { get; set; }
    public double? DestLon { get; set; }

    // Hỗ trợ chọn 2 đến 3 điểm dừng liên tiếp
    public List<RouteWaypoint>? Waypoints { get; set; }
    public bool OptimizeOrder { get; set; } = true;
}