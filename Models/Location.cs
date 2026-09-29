using System;
using System.Collections.Generic;
namespace SmartMap.Models;

public class Location
{
    private int id;
    private string name;
    private string address;
    private Position position;
    private DestinationType type;
    private double price;
    private string placeId;

    public Location(
        int id,
        string name,
        string address,
        Position position,
        DestinationType type,
        double price,
        string placeId)
    {
        this.id = id;
        this.name = name;
        this.address = address;
        this.position = position;
        this.type = type;
        this.price = price;
        this.placeId = placeId;
    }

    public int GetId()
    {
        return id;
    }

    public string GetName()
    {
        return name;
    }

    public string GetAddress()
    {
        return address;
    }

    public Position GetPosition()
    {
        return position;
    }

    public DestinationType GetDestinationType()
    {
        return type;
    }

    public double GetPrice()
    {
        return price;
    }

    public string GetPlaceId()
    {
        return placeId;
    }
}