using SmartMap.Models;
var builder = WebApplication.CreateBuilder(args);

var app = builder.Build();

app.MapGet("/", () =>
{
    return "SmartMap API is running!";
});

app.MapPost("/api/route", (RouteRequest request) =>
{
    return Results.Ok(new
    {
        message = "Route API is working!",
        start = request.StartName,
        destination = request.DestinationName
    });
});

app.Run();