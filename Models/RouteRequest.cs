namespace SmartMap.Models;

public class RouteRequest
{
    public string StartName { get; set; } = "";
    public string DestinationName { get; set; } = "";
    public double? StartLat { get; set; }
    public double? StartLon { get; set; }
    public double? DestLat { get; set; }
    public double? DestLon { get; set; }
}