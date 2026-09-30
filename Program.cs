using SmartMap.Models;
using SmartMap.Service;

var builder = WebApplication.CreateBuilder(args);

// 1. Đăng ký các dịch vụ (Dependency Injection)
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
    {
        policy.AllowAnyOrigin()
              .AllowAnyHeader()
              .AllowAnyMethod();
    });
});

builder.Services.AddSingleton<PlaceService>();
builder.Services.AddSingleton<RoutingService>();
builder.Services.AddSingleton<DistanceService>();

var app = builder.Build();

app.UseCors();

// 2. Cấu hình phục vụ file Web (HTML, CSS, JS) trong thư mục wwwroot
var defaultFilesOptions = new DefaultFilesOptions();
defaultFilesOptions.DefaultFileNames.Clear();
defaultFilesOptions.DefaultFileNames.Add("firstPage.html");
app.UseDefaultFiles(defaultFilesOptions); // Tự động mở firstPage.html khi vào trang chủ /
app.UseStaticFiles();                   // Cho phép tải các file tĩnh trong wwwroot

// 3. API lấy danh sách các địa điểm
app.MapGet("/api/places", async (
    string? category,
    double? lat,
    double? lon,
    double? distance,
    PlaceService placeService) =>
{
    double centerLat = lat ?? 10.8498;
    double centerLon = lon ?? 106.7713;
    double radiusMeters = (distance ?? 2.0) * 1000.0;

    DestinationType type = (category?.ToLower()) switch
    {
        "food" => DestinationType.FOOD,
        "coffee" => DestinationType.COFFEE,
        "shopping" => DestinationType.SHOPPING,
        "study" => DestinationType.STUDY,
        "relax" => DestinationType.RELAX,
        "entertainment" => DestinationType.ENTERTAINMENT,
        _ => DestinationType.COFFEE
    };

    var places = await placeService.SearchPlaces(centerLat, centerLon, radiusMeters, type);

    var result = places.Select(p => new
    {
        id = p.GetId(),
        name = p.GetName(),
        address = p.GetAddress(),
        latitude = p.GetPosition().GetLatitude(),
        longitude = p.GetPosition().GetLongitude(),
        type = p.GetType().ToString(),
        price = p.GetPrice()
    });

    return Results.Ok(result);
});

// 4. API tìm lộ trình đường đi
app.MapPost("/api/route", async (
    RouteRequest request,
    RoutingService routingService,
    DistanceService distanceService) =>
{
    var knownLocations = new Dictionary<string, (double Lat, double Lon)>(StringComparer.OrdinalIgnoreCase)
    {
        ["Đại học Công nghệ Kỹ thuật TP.HCM"] = (10.8498, 106.7713),
        ["Đại học Sư phạm Kỹ thuật TP.HCM"] = (10.8506, 106.7719),
        ["Vincom Plaza Lê Văn Việt"] = (10.8458, 106.7778),
        ["Làng Đại học Quốc gia TP.HCM"] = (10.8700, 106.8030),
        ["Đại học Văn Lang"] = (10.8275, 106.6990),
        ["Đại học Bách Khoa TP.HCM"] = (10.7720, 106.6578),
        ["Local Food Restaurant"] = (10.8512, 106.7725),
        ["Coffee House"] = (10.8465, 106.7780),
        ["Entertainment Place"] = (10.8580, 106.7800)
    };

    double startLat = request.StartLat ?? (knownLocations.TryGetValue(request.StartName, out var sPos) ? sPos.Lat : 10.8498);
    double startLon = request.StartLon ?? (knownLocations.TryGetValue(request.StartName, out sPos) ? sPos.Lon : 106.7713);

    double destLat = request.DestLat ?? (knownLocations.TryGetValue(request.DestinationName, out var dPos) ? dPos.Lat : 10.8465);
    double destLon = request.DestLon ?? (knownLocations.TryGetValue(request.DestinationName, out dPos) ? dPos.Lon : 106.7780);

    var start = new Position(startLat, startLon);
    var destination = new Position(destLat, destLon);

    var route = await routingService.GetRoute(start, destination);

    if (route == null)
    {
        double directKm = distanceService.CalculateDistance(start, destination);
        return Results.Ok(new
        {
            totalDistance = directKm,
            duration = directKm * 120.0,
            routeCoordinates = new List<double[]>
            {
                new double[] { startLon, startLat },
                new double[] { destLon, destLat }
            },
            destination = new { latitude = destLat, longitude = destLon }
        });
    }

    return Results.Ok(new
    {
        totalDistance = route.Distance / 1000.0,
        duration = route.Duration,
        routeCoordinates = route.RouteCoordinates,
        destination = new { latitude = destLat, longitude = destLon }
    });
});

Console.WriteLine();
Console.WriteLine("=================================================");
Console.WriteLine(" Web SmartMap đang chạy tại: http://localhost:5000");
Console.WriteLine(" Mở trình duyệt và truy cập: http://localhost:5000");
Console.WriteLine("=================================================");
Console.WriteLine();

app.Run("http://localhost:5000");