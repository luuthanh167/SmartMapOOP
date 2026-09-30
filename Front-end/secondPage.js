const params = new URLSearchParams(window.location.search);

const userLocation = params.get("location") || "Đại học Công nghệ Kỹ thuật TP.HCM";
const category = params.get("category") || "coffee";
const budget = params.get("budget") || "50000";
const distance = params.get("distance") || "2";

const searchInfo = document.getElementById("searchInfo");
const summaryTitle = document.getElementById("summaryTitle");
const placesCountBadge = document.getElementById("placesCountBadge");
const mapLoading = document.getElementById("mapLoading");
const mapLoadingText = document.getElementById("mapLoadingText");
const backButton = document.getElementById("backButton");
const routeButton = document.getElementById("routeButton");
const placesList = document.getElementById("placesList");
const recenterBtn = document.getElementById("recenterBtn");
const routeBanner = document.getElementById("routeBanner");
const routeBannerTitle = document.getElementById("routeBannerTitle");
const routeBannerDetails = document.getElementById("routeBannerDetails");
const closeBannerBtn = document.getElementById("closeBannerBtn");

// Tọa độ các điểm trường học/địa danh phổ biến
const knownCoordinates = {
  "Đại học Công nghệ Kỹ thuật TP.HCM": [10.8498, 106.7713],
  "Đại học Sư phạm Kỹ thuật TP.HCM": [10.8506, 106.7719],
  "Vincom Plaza Lê Văn Việt": [10.8458, 106.7778],
  "Làng Đại học Quốc gia TP.HCM": [10.8700, 106.8030],
  "Đại học Văn Lang": [10.8275, 106.6990],
  "Đại học Bách Khoa TP.HCM": [10.7720, 106.6578],
};

let startLat = 10.8498;
let startLon = 106.7713;

if (knownCoordinates[userLocation]) {
  startLat = knownCoordinates[userLocation][0];
  startLon = knownCoordinates[userLocation][1];
}

let selectedPlace = null;
let selectedLat = null;
let selectedLon = null;
let allPlaces = [];
let placeMarkers = [];
let startMarker = null;

// Cập nhật tiêu đề thanh điều hướng trên
summaryTitle.textContent = `🗺️ ${userLocation}`;
searchInfo.textContent = `${getCategoryLabel(category)} • Ngân sách: ${
  budget ? Number(budget).toLocaleString("vi-VN") + " đ" : "Tự do"
} • Bán kính: ${distance} km`;

// Khởi tạo bản đồ MapLibre
const map = new maplibregl.Map({
  container: "map",
  style: "https://tiles.openfreemap.org/styles/liberty",
  center: [startLon, startLat],
  zoom: 15,
  attributionControl: { compact: true },
});

map.addControl(new maplibregl.NavigationControl({ showCompass: true }), "bottom-right");

// Thêm Marker vị trí xuất phát phong cách Google Maps (Pulsing Blue Dot)
function createStartMarker() {
  const startEl = document.createElement("div");
  startEl.className = "user-marker";
  startEl.innerHTML = `
    <div class="user-marker-ring">
      <div class="user-marker-pulse"></div>
      <div class="user-marker-dot"></div>
    </div>
    <div class="user-marker-label">Vị trí của bạn (${userLocation})</div>
  `;

  startMarker = new maplibregl.Marker({ element: startEl, anchor: "center" })
    .setLngLat([startLon, startLat])
    .addTo(map);
}

map.on("load", function () {
  createStartMarker();
  loadPlaces();
});

// Tính khoảng cách Haversine sơ bộ để hiển thị trên thẻ
function calculateDistanceKm(lat1, lon1, lat2, lon2) {
  const R = 6371;
  const dLat = ((lat2 - lat1) * Math.PI) / 180;
  const dLon = ((lon2 - lon1) * Math.PI) / 180;
  const a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos((lat1 * Math.PI) / 180) *
      Math.cos((lat2 * Math.PI) / 180) *
      Math.sin(dLon / 2) *
      Math.sin(dLon / 2);
  return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
}

function getCategoryIcon(cat) {
  const c = (cat || "").toLowerCase();
  if (c.includes("coffee") || c.includes("cafe")) return "☕";
  if (c.includes("food") || c.includes("restaurant")) return "🍜";
  if (c.includes("entertainment") || c.includes("cinema")) return "🎮";
  if (c.includes("shopping") || c.includes("mall")) return "🛍️";
  if (c.includes("study") || c.includes("library")) return "📚";
  return "📍";
}

function getCategoryLabel(cat) {
  const c = (cat || "").toLowerCase();
  if (c.includes("coffee")) return "Cà phê";
  if (c.includes("food")) return "Ẩm thực";
  if (c.includes("entertainment")) return "Giải trí";
  if (c.includes("shopping")) return "Mua sắm";
  if (c.includes("study")) return "Học tập / Thư viện";
  return "Địa điểm";
}

// Tải tất cả địa điểm và hiển thị toàn bộ Marker + Tên lên bản đồ
function loadPlaces() {
  const apiUrl = window.location.origin.startsWith("http")
    ? `/api/places?category=${encodeURIComponent(category)}&lat=${startLat}&lon=${startLon}&distance=${distance}&budget=${budget}`
    : `http://localhost:5000/api/places?category=${encodeURIComponent(category)}&lat=${startLat}&lon=${startLon}&distance=${distance}&budget=${budget}`;

  fetch(apiUrl)
    .then(function (res) {
      if (!res.ok) throw new Error("Could not load places");
      return res.json();
    })
    .then(function (places) {
      if (Array.isArray(places) && places.length > 0) {
        allPlaces = places;
      } else {
        allPlaces = getDefaultFallbackPlaces();
      }
      renderPlacesAndMarkers();
    })
    .catch(function (err) {
      console.log("Using default fallback places:", err);
      allPlaces = getDefaultFallbackPlaces();
      renderPlacesAndMarkers();
    });
}

function renderPlacesAndMarkers() {
  // Xóa marker cũ nếu có
  placeMarkers.forEach((m) => m.remove());
  placeMarkers = [];
  placesList.innerHTML = "";

  placesCountBadge.textContent = `${allPlaces.length} địa điểm`;

  const bounds = new maplibregl.LngLatBounds();
  bounds.extend([startLon, startLat]);

  const icon = getCategoryIcon(category);

  allPlaces.forEach(function (p, index) {
    const pLat = parseFloat(p.latitude);
    const pLon = parseFloat(p.longitude);

    bounds.extend([pLon, pLat]);

    const distKm = calculateDistanceKm(startLat, startLon, pLat, pLon);
    const distText = distKm < 1 ? `${Math.round(distKm * 1000)} m` : `${distKm.toFixed(1)} km`;

    // 1. TẠO MARKER VỚI TÊN HIỂN THỊ TRỰC TIẾP TRÊN BẢN ĐỒ (Google Maps Style)
    const markerEl = document.createElement("div");
    markerEl.className = "custom-marker";
    markerEl.dataset.index = index;
    markerEl.innerHTML = `
      <div class="marker-pin">
        <span class="marker-icon">${icon}</span>
      </div>
      <div class="marker-label" title="${p.name}">${p.name}</div>
    `;

    // Thao tác click vào Marker trên bản đồ
    markerEl.addEventListener("click", function (e) {
      e.stopPropagation();
      selectPlace(index);
    });

    const marker = new maplibregl.Marker({ element: markerEl, anchor: "bottom" })
      .setLngLat([pLon, pLat])
      .addTo(map);

    placeMarkers.push({ marker, element: markerEl, data: p });

    // 2. TẠO THẺ PLACE CARD Ở DANH SÁCH BÊN DƯỚI
    const card = document.createElement("div");
    card.className = "place-card";
    card.dataset.index = index;

    const priceText = p.price && p.price > 0
      ? Number(p.price).toLocaleString("vi-VN") + " đ"
      : (p.address || "Gần bạn");

    card.innerHTML = `
      <div class="place-icon">${icon}</div>
      <div class="place-info">
        <div class="place-name" title="${p.name}">${p.name}</div>
        <div class="place-details">${priceText}</div>
        <div class="place-meta">
          <span class="place-rating">⭐ 4.5</span>
          <span class="place-distance">📍 ${distText}</span>
        </div>
      </div>
    `;

    card.addEventListener("click", function () {
      selectPlace(index);
    });

    placesList.appendChild(card);
  });

  // Tự động căn chỉnh bản đồ hiển thị trọn vẹn điểm xuất phát và toàn bộ các điểm gợi ý
  map.fitBounds(bounds, {
    padding: { top: 90, bottom: 220, left: 60, right: 60 },
    maxZoom: 16,
  });
}

// Hàm chọn một địa điểm (khi click vào Card hoặc Marker)
function selectPlace(index) {
  const item = placeMarkers[index];
  if (!item) return;

  const place = item.data;
  selectedPlace = place.name;
  selectedLat = parseFloat(place.latitude);
  selectedLon = parseFloat(place.longitude);

  // Cập nhật trạng thái active trên toàn bộ Marker
  placeMarkers.forEach(function (m, idx) {
    if (idx === index) {
      m.element.classList.add("active");
    } else {
      m.element.classList.remove("active");
    }
  });

  // Cập nhật trạng thái selected trên toàn bộ Place Card
  const cards = placesList.querySelectorAll(".place-card");
  cards.forEach(function (c, idx) {
    if (idx === index) {
      c.classList.add("selected");
      c.scrollIntoView({ behavior: "smooth", block: "nearest", inline: "center" });
    } else {
      c.classList.remove("selected");
    }
  });

  // Bật nút "CHỈ ĐƯỜNG"
  routeButton.disabled = false;
  routeButton.innerHTML = `<span class="route-btn-icon">➤</span><span>ĐẾN ${truncate(place.name, 15).toUpperCase()}</span>`;

  // Bay nhẹ tới địa điểm vừa chọn
  map.flyTo({
    center: [selectedLon, selectedLat],
    zoom: 16,
    speed: 1.2,
    offset: [0, -60],
  });
}

function truncate(str, max) {
  return str.length > max ? str.substring(0, max) + "..." : str;
}

// Căn chỉnh lại tầm nhìn xem toàn bộ các điểm
recenterBtn.addEventListener("click", function () {
  if (allPlaces.length === 0) return;
  const bounds = new maplibregl.LngLatBounds();
  bounds.extend([startLon, startLat]);
  allPlaces.forEach((p) => bounds.extend([parseFloat(p.longitude), parseFloat(p.latitude)]));
  map.fitBounds(bounds, {
    padding: { top: 90, bottom: 220, left: 60, right: 60 },
    maxZoom: 16,
  });
});

// Vẽ đường đi theo phong cách Google Maps (viền casing đậm + lõi xanh nổi bật)
function drawRoute(routeCoordinates) {
  const routeData = {
    type: "Feature",
    properties: {},
    geometry: {
      type: "LineString",
      coordinates: routeCoordinates,
    },
  };

  // Cập nhật nếu đã có layer
  if (map.getSource("shortestRoute")) {
    map.getSource("shortestRoute").setData(routeData);
    return;
  }

  map.addSource("shortestRoute", {
    type: "geojson",
    data: routeData,
  });

  // 1. Lớp viền ngoài (Casing)
  map.addLayer({
    id: "shortestRoute-casing",
    type: "line",
    source: "shortestRoute",
    layout: {
      "line-join": "round",
      "line-cap": "round",
    },
    paint: {
      "line-color": "#174ea6",
      "line-width": 8,
      "line-opacity": 0.8,
    },
  });

  // 2. Lớp đường đi chính Google Blue
  map.addLayer({
    id: "shortestRoute-inner",
    type: "line",
    source: "shortestRoute",
    layout: {
      "line-join": "round",
      "line-cap": "round",
    },
    paint: {
      "line-color": "#4285f4",
      "line-width": 5,
    },
  });
}

function fitRoute(routeCoordinates) {
  const bounds = new maplibregl.LngLatBounds();
  routeCoordinates.forEach(function (point) {
    bounds.extend([point[0], point[1]]);
  });

  map.fitBounds(bounds, {
    padding: { top: 120, bottom: 240, left: 80, right: 80 },
  });
}

// Xử lý khi nhấn nút "CHỈ ĐƯỜNG"
function findRoute() {
  if (selectedPlace === null) return;

  mapLoadingText.textContent = `Đang tìm tuyến đường nhanh nhất đến ${selectedPlace}...`;
  mapLoading.style.display = "flex";

  const apiUrl = window.location.origin.startsWith("http")
    ? "/api/route"
    : "http://localhost:5000/api/route";

  fetch(apiUrl, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      startName: userLocation,
      destinationName: selectedPlace,
      startLat: startLat,
      startLon: startLon,
      destLat: selectedLat,
      destLon: selectedLon,
    }),
  })
    .then(function (response) {
      if (!response.ok) throw new Error("Route request failed");
      return response.json();
    })
    .then(function (data) {
      if (!data.routeCoordinates || data.routeCoordinates.length === 0) {
        throw new Error("No route found");
      }

      drawRoute(data.routeCoordinates);
      fitRoute(data.routeCoordinates);

      // Hiển thị banner kết quả kiểu Google Maps
      const dist = Number(data.totalDistance).toFixed(2);
      const estMin = Math.max(1, Math.round(data.totalDistance * 3)); // ước tính phút di chuyển
      routeBannerTitle.textContent = `${userLocation} → ${selectedPlace}`;
      routeBannerDetails.textContent = `🚗 Quãng đường: ${dist} km • Ước tính ~${estMin} phút`;
      routeBanner.style.display = "flex";

      mapLoading.style.display = "none";
    })
    .catch(function (error) {
      console.log("Routing error:", error);
      mapLoadingText.textContent = "Không tìm thấy tuyến đường.";
      setTimeout(() => {
        mapLoading.style.display = "none";
      }, 2000);
    });
}

routeButton.addEventListener("click", function () {
  findRoute();
});

closeBannerBtn.addEventListener("click", function () {
  routeBanner.style.display = "none";
});

backButton.addEventListener("click", function () {
  window.location.href = "firstPage.html";
});

// Danh sách dự phòng nếu không kết nối được mạng
function getDefaultFallbackPlaces() {
  return [
    {
      id: 1,
      name: "The Coffee House - Lê Văn Việt",
      address: "Lê Văn Việt, TP. Thủ Đức",
      latitude: startLat - 0.0033,
      longitude: startLon + 0.0067,
      type: "COFFEE",
      price: 45000,
    },
    {
      id: 2,
      name: "Highlands Coffee - Vincom Plaza",
      address: "Vincom Plaza Lê Văn Việt",
      latitude: startLat - 0.0040,
      longitude: startLon + 0.0065,
      type: "COFFEE",
      price: 45000,
    },
    {
      id: 3,
      name: "Local Food Restaurant",
      address: "Võ Văn Ngân, TP. Thủ Đức",
      latitude: startLat + 0.0014,
      longitude: startLon + 0.0012,
      type: "FOOD",
      price: 50000,
    },
    {
      id: 4,
      name: "Phúc Long Coffee & Tea",
      address: "Đỗ Xuân Hợp, TP. Thủ Đức",
      latitude: startLat - 0.0080,
      longitude: startLon - 0.0030,
      type: "COFFEE",
      price: 55000,
    },
    {
      id: 5,
      name: "Entertainment CGV Cinema",
      address: "Tầng 5, Vincom Plaza",
      latitude: startLat - 0.0042,
      longitude: startLon + 0.0068,
      type: "ENTERTAINMENT",
      price: 90000,
    },
  ];
}
