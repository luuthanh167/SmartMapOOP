using SmartMap.Models;

namespace SmartMap.Service;

public class DistanceService
{
    public double CalculateDistance(Position a, Position b)
    {
        double lat1 = a.GetLatitude();
        double lon1 = a.GetLongitude();

        double lat2 = b.GetLatitude();
        double lon2 = b.GetLongitude();

        const double earthRadiusKm = 6371.0;

        double dLat = DegreesToRadians(lat2 - lat1);
        double dLon = DegreesToRadians(lon2 - lon1);

        double lat1Rad = DegreesToRadians(lat1);
        double lat2Rad = DegreesToRadians(lat2);

        double h =
            Math.Sin(dLat / 2) * Math.Sin(dLat / 2)
            + Math.Cos(lat1Rad)
            * Math.Cos(lat2Rad)
            * Math.Sin(dLon / 2)
            * Math.Sin(dLon / 2);

        double distance = 2
            * earthRadiusKm
            * Math.Asin(Math.Sqrt(h));

        return distance;
    }

    private double DegreesToRadians(double degrees)
    {
        return degrees * Math.PI / 180.0;
    }
}