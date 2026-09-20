using System;
using System.Collections.Generic;
namespace SmartMap.Models;

public class Preference
{
    private List<DestinationType> types;
    private double maxDistance;
    private double budget;

    public Preference()
    {
        types = new List<DestinationType>();
        maxDistance = 0;
        budget = 0;
    }

    public void AddType(DestinationType type)
    {
        if (!types.Contains(type))
        {
            types.Add(type);
        }
    }

    public void RemoveType(DestinationType type)
    {
        types.Remove(type);
    }

    public List<DestinationType> GetTypes()
    {
        return types;
    }

    public void SetMaxDistance(double maxDistance)
    {
        this.maxDistance = maxDistance;
    }

    public double GetMaxDistance()
    {
        return maxDistance;
    }

    public void SetBudget(double budget)
    {
        this.budget = budget;
    }

    public double GetBudget()
    {
        return budget;
    }
}