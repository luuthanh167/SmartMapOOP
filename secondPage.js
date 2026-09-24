const params = new URLSearchParams(window.location.search);

const category = params.get("category") || "";
const budget = params.get("budget") || "";
const distance = params.get("distance") || "";

const searchInfo = document.getElementById("searchInfo");
const mapLoading = document.getElementById("mapLoading");
const backButton = document.getElementById("backButton");

searchInfo.textContent =
  "Start: HCM-UTE • " +
  category +
  " • " +
  Number(budget).toLocaleString("vi-VN") +
  " VND • " +
  distance +
  " km";

const uteLatitude = 10.8498;
const uteLongitude = 106.7713;

const map = new maplibregl.Map({
  container: "map",
  style: "https://tiles.openfreemap.org/styles/liberty",
  center: [uteLongitude, uteLatitude],
  zoom: 15,
  attributionControl: {
    compact: true,
  },
});

map.addControl(new maplibregl.NavigationControl(), "top-right");

let uteMarker = null;
let destinationMarker = null;

function createUteMarker() {
  uteMarker = new maplibregl.Marker({
    color: "#6c63d9",
  })
    .setLngLat([uteLongitude, uteLatitude])
    .setPopup(
      new maplibregl.Popup({
        offset: 25,
      }).setHTML("<b>HCM-UTE</b>"),
    )
    .addTo(map);
}

function showDestination(latitude, longitude, name) {
  if (destinationMarker !== null) {
    destinationMarker.remove();
  }

  destinationMarker = new maplibregl.Marker({
    color: "#e74c3c",
  })
    .setLngLat([longitude, latitude])
    .setPopup(
      new maplibregl.Popup({
        offset: 25,
      }).setHTML("<b>" + name + "</b>"),
    )
    .addTo(map);
}

function drawRoute(routeCoordinates) {
  const routeData = {
    type: "Feature",
    properties: {},
    geometry: {
      type: "LineString",
      coordinates: routeCoordinates,
    },
  };

  if (map.getSource("shortestRoute")) {
    map.getSource("shortestRoute").setData(routeData);
    return;
  }

  map.addSource("shortestRoute", {
    type: "geojson",
    data: routeData,
  });

  map.addLayer({
    id: "shortestRoute",
    type: "line",
    source: "shortestRoute",
    layout: {
      "line-join": "round",
      "line-cap": "round",
    },
    paint: {
      "line-color": "#6c63d9",
      "line-width": 6,
    },
  });
}

function findRoute(placeName) {
  mapLoading.textContent = "Finding shortest route...";
  mapLoading.style.display = "block";

  fetch("/api/route", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      startName: "Đại học Công nghệ Kỹ thuật TP.HCM",
      destinationName: placeName,
    }),
  })
    .then(function (response) {
      if (!response.ok) {
        throw new Error("Route request failed");
      }

      return response.json();
    })
    .then(function (data) {
      if (!data.routeCoordinates || data.routeCoordinates.length === 0) {
        throw new Error("No route found");
      }

      drawRoute(data.routeCoordinates);

      showDestination(
        data.destination.latitude,
        data.destination.longitude,
        placeName,
      );

      searchInfo.textContent =
        "HCM-UTE → " +
        placeName +
        " • " +
        Number(data.totalDistance).toFixed(2) +
        " km";

      map.fitBounds(
        data.routeCoordinates.map(function (point) {
          return [point[1], point[0]];
        }),
        {
          padding: 60,
        },
      );

      mapLoading.style.display = "none";
    })
    .catch(function (error) {
      console.log(error);

      mapLoading.textContent = "Cannot find shortest route.";
    });
}

document.querySelectorAll(".view-button").forEach(function (button) {
  button.addEventListener("click", function () {
    const card = button.closest(".place-card");
    const placeName = card.dataset.placeName;

    findRoute(placeName);
  });
});

map.on("load", function () {
  createUteMarker();
});

backButton.addEventListener("click", function () {
  window.location.href = "firstPage.html";
});
