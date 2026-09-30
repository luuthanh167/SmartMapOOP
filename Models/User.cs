using System;
using System.Collections.Generic;
namespace SmartMap.Models;

public class User
{
    private string name;
    private Position? currentPosition;
    private Preference preference;

    public User(string name)
    {
        this.name = name;
        this.preference = new Preference();
    }

    public void SetName(string name)
    {
        this.name = name;
    }

    public string GetName()
    {
        return name;
    }

    public void SetCurrentPosition(Position position)
    {
        currentPosition = position;
    }

    public Position? GetCurrentPosition()
    {
        return currentPosition;
    }

    public Preference GetPreference()
    {
        return preference;
    }
}