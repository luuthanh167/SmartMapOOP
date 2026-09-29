const params = new URLSearchParams(window.location.search);

const category = params.get("category") || "";
const budget = params.get("budget") || "";
const distance = params.get("distance") || "";

const searchInfo = document.getElementById("searchInfo");
const mapLoading = document.getElementById("mapLoading");
const backButton = document.getElementById("backButton");
const routeButton = document.getElementById("routeButton");

const placeCards = document.querySelectorAll(".place-card");

const uteLatitude = 10.8498;
const uteLongitude = 106.7713;

let selectedPlace = null;
let destinationMarker = null;

searchInfo.textContent =
  "Start: HCM-UTE • " +
  category +
  " • " +
  Number(budget).toLocaleString("vi-VN") +
  " VND • " +
  distance +
  " km";

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

const uteMarker = new maplibregl.Marker({
  color: "#6c63d9",
})
  .setLngLat([uteLongitude, uteLatitude])
  .setPopup(
    new maplibregl.Popup({
      offset: 25,
    }).setHTML("<b>HCM-UTE</b>"),
  )
  .addTo(map);

placeCards.forEach(function (card) {
  card.addEventListener("click", function () {
    placeCards.forEach(function (item) {
      item.classList.remove("selected");
    });

    card.classList.add("selected");

    selectedPlace = card.dataset.placeName;

    routeButton.disabled = false;

    searchInfo.textContent = "Selected: " + selectedPlace;
  });
});

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

function fitRoute(routeCoordinates) {
  const bounds = new maplibregl.LngLatBounds();

  routeCoordinates.forEach(function (point) {
    bounds.extend([point[0], point[1]]);
  });

  map.fitBounds(bounds, {
    padding: 60,
  });
}

function findRoute() {
  if (selectedPlace === null) {
    return;
  }

  mapLoading.textContent = "Finding shortest route...";

  mapLoading.style.display = "block";

  fetch("http://localhost:5000/api/route", {
    method: "POST",

    headers: {
      "Content-Type": "application/json",
    },

    body: JSON.stringify({
      startName: "Đại học Công nghệ Kỹ thuật TP.HCM",

      destinationName: selectedPlace,
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
        selectedPlace,
      );

      fitRoute(data.routeCoordinates);

      searchInfo.textContent =
        "HCM-UTE → " +
        selectedPlace +
        " • " +
        Number(data.totalDistance).toFixed(2) +
        " km";

      mapLoading.style.display = "none";
    })

    .catch(function (error) {
      console.log(error);

      mapLoading.textContent = "Cannot find shortest route.";
    });
}

routeButton.addEventListener("click", function () {
  findRoute();
});

backButton.addEventListener("click", function () {
  window.location.href = "firstPage.html";
});

mapLoading.style.display = "none";
