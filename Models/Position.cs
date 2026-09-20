using System;
using System.Collections.Generic;
namespace SmartMap.Models;

public class Position
{
    private double latitude;
    private double longitude;

    public Position(double latitude, double longitude)
    {
        this.latitude = latitude;
        this.longitude = longitude;
    }

    public double GetLatitude()
    {
        return latitude;
    }

    public double GetLongitude()
    {
        return longitude;
    }
}