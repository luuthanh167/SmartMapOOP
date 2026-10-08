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
const routeBtnText = document.getElementById("routeBtnText");
const placesList = document.getElementById("placesList");
const recenterBtn = document.getElementById("recenterBtn");
const routeBanner = document.getElementById("routeBanner");
const routeBannerTitle = document.getElementById("routeBannerTitle");
const routeBannerDetails = document.getElementById("routeBannerDetails");
const routeBannerLegs = document.getElementById("routeBannerLegs");
const closeBannerBtn = document.getElementById("closeBannerBtn");
const optimizeOrderCheckbox = document.getElementById("optimizeOrderCheckbox");
const clearSelectionBtn = document.getElementById("clearSelectionBtn");
const selectedWaypointsBar = document.getElementById("selectedWaypointsBar");

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

// Quản lý danh sách các điểm được chọn (hỗ trợ chọn 1 đến 3 điểm liên tiếp)
let selectedPlaces = [];
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
      toggleSelectPlace(index);
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
      toggleSelectPlace(index);
    });

    placesList.appendChild(card);
  });

  // Tự động căn chỉnh bản đồ hiển thị trọn vẹn điểm xuất phát và toàn bộ các điểm gợi ý
  map.fitBounds(bounds, {
    padding: { top: 90, bottom: 220, left: 60, right: 60 },
    maxZoom: 16,
  });
}

// Thông báo Toast ngắn
function showToast(message) {
  const existing = document.querySelector(".toast-msg");
  if (existing) existing.remove();

  const toast = document.createElement("div");
  toast.className = "toast-msg";
  toast.textContent = message;
  document.body.appendChild(toast);

  setTimeout(function () {
    toast.remove();
  }, 2500);
}

// Hàm chọn / bỏ chọn địa điểm (hỗ trợ chọn 1 đến 3 điểm liên tiếp)
function toggleSelectPlace(index) {
  const item = placeMarkers[index];
  if (!item) return;

  const place = item.data;
  const existingIndex = selectedPlaces.findIndex((p) => p.index === index);

  if (existingIndex !== -1) {
    // Đã chọn -> Bỏ chọn
    selectedPlaces.splice(existingIndex, 1);
  } else {
    // Chưa chọn -> Kiểm tra giới hạn tối đa 10 điểm dừng
    if (selectedPlaces.length >= 10) {
      showToast("⚠️ Bạn chỉ có thể chọn tối đa 10 điểm dừng liên tiếp!");
      return;
    }

    selectedPlaces.push({
      index: index,
      name: place.name,
      latitude: parseFloat(place.latitude),
      longitude: parseFloat(place.longitude),
      address: place.address,
      price: place.price,
      type: place.type,
    });

    // Bay nhẹ tới địa điểm vừa chọn
    map.flyTo({
      center: [parseFloat(place.longitude), parseFloat(place.latitude)],
      zoom: 16,
      speed: 1.2,
      offset: [0, -60],
    });
  }

  // Giữ lại selectedPlace cho tương thích
  if (selectedPlaces.length > 0) {
    selectedPlace = selectedPlaces[0].name;
    selectedLat = selectedPlaces[0].latitude;
    selectedLon = selectedPlaces[0].longitude;
  } else {
    selectedPlace = null;
    selectedLat = null;
    selectedLon = null;
  }

  updateSelectionUI();
}

// Cập nhật toàn bộ giao diện: Marker (1..10), Cards, Chips Bar, Nút chỉ đường
function updateSelectionUI() {
  // 1. Cập nhật Marker trên bản đồ
  placeMarkers.forEach(function (m, idx) {
    const selIdx = selectedPlaces.findIndex((p) => p.index === idx);
    const pinEl = m.element.querySelector(".marker-pin");

    m.element.classList.remove("active", "selected-stop");
    for (let s = 1; s <= 10; s++) {
      m.element.classList.remove("selected-stop-" + s);
    }

    const oldBadge = pinEl.querySelector(".stop-number-badge");
    if (oldBadge) oldBadge.remove();

    if (selIdx !== -1) {
      const stopNum = selIdx + 1;
      m.element.classList.add("selected-stop", "selected-stop-" + stopNum);

      const badge = document.createElement("div");
      badge.className = "stop-number-badge";
      badge.textContent = stopNum;
      pinEl.appendChild(badge);
    }
  });

  // 2. Cập nhật thẻ Place Card ở danh sách bên dưới
  const cards = placesList.querySelectorAll(".place-card");
  cards.forEach(function (c, idx) {
    const selIdx = selectedPlaces.findIndex((p) => p.index === idx);

    c.classList.remove("selected");
    for (let s = 1; s <= 10; s++) {
      c.classList.remove("selected-stop-" + s);
    }

    const oldCardBadge = c.querySelector(".card-stop-badge");
    if (oldCardBadge) oldCardBadge.remove();

    if (selIdx !== -1) {
      const stopNum = selIdx + 1;
      c.classList.add("selected", "selected-stop-" + stopNum);

      const badge = document.createElement("span");
      badge.className = "card-stop-badge";
      badge.textContent = `Chặng ${stopNum}`;
      c.appendChild(badge);

      c.scrollIntoView({ behavior: "smooth", block: "nearest", inline: "center" });
    }
  });

  // 3. Cập nhật thanh Waypoints Bar (chips)
  renderWaypointsBar();

  // 4. Cập nhật nút CHỈ ĐƯỜNG & nút Bỏ chọn
  const count = selectedPlaces.length;
  if (count === 0) {
    routeButton.disabled = true;
    if (routeBtnText) routeBtnText.textContent = "CHỌN 1-10 ĐỊA ĐIỂM";
    if (clearSelectionBtn) clearSelectionBtn.style.display = "none";
  } else {
    routeButton.disabled = false;
    if (clearSelectionBtn) clearSelectionBtn.style.display = "inline-flex";

    if (routeBtnText) {
      if (count === 1) {
        routeBtnText.textContent = `ĐẾN ${truncate(selectedPlaces[0].name, 14).toUpperCase()}`;
      } else {
        routeBtnText.textContent = `CHỈ ĐƯỜNG (${count} ĐIỂM LIÊN TIẾP)`;
      }
    }
  }
}

// Vẽ thanh danh sách điểm dừng (Chip List)
function renderWaypointsBar() {
  if (!selectedWaypointsBar) return;

  if (selectedPlaces.length === 0) {
    selectedWaypointsBar.style.display = "none";
    selectedWaypointsBar.innerHTML = "";
    return;
  }

  selectedWaypointsBar.style.display = "flex";
  selectedWaypointsBar.innerHTML = "";

  // Chip Xuất phát
  const startChip = document.createElement("div");
  startChip.className = "waypoint-chip chip-start";
  startChip.innerHTML = `<span>📍 Xuất phát: <strong>${truncate(userLocation, 14)}</strong></span>`;
  selectedWaypointsBar.appendChild(startChip);

  // Chips cho các điểm dừng đã chọn
  selectedPlaces.forEach(function (p, idx) {
    const arrow = document.createElement("span");
    arrow.className = "waypoint-arrow";
    arrow.textContent = "➔";
    selectedWaypointsBar.appendChild(arrow);

    const chip = document.createElement("div");
    chip.className = `waypoint-chip chip-stop-${idx + 1}`;
    chip.innerHTML = `
      <span><strong>${idx + 1}.</strong> ${truncate(p.name, 15)}</span>
      <span class="waypoint-chip-remove" title="Bỏ chọn điểm này">✕</span>
    `;

    chip.querySelector(".waypoint-chip-remove").addEventListener("click", function (e) {
      e.stopPropagation();
      toggleSelectPlace(p.index);
    });

    selectedWaypointsBar.appendChild(chip);
  });
}

// Xóa toàn bộ điểm đã chọn
function clearAllSelections() {
  selectedPlaces = [];
  selectedPlace = null;
  selectedLat = null;
  selectedLon = null;
  updateSelectionUI();

  if (map.getSource("shortestRoute")) {
    map.getSource("shortestRoute").setData({
      type: "Feature",
      geometry: { type: "LineString", coordinates: [] },
    });
  }
  routeBanner.style.display = "none";
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

// Xử lý khi nhấn nút "CHỈ ĐƯỜNG" (hỗ trợ 1 đến 3 điểm liên tiếp & tối ưu đường ngắn nhất)
function findRoute() {
  if (selectedPlaces.length === 0) return;

  const count = selectedPlaces.length;
  mapLoadingText.textContent = count === 1
    ? `Đang tìm đường ngắn nhất đến ${selectedPlaces[0].name}...`
    : `Đang tìm đường đi ngắn nhất qua ${count} điểm liên tiếp...`;
  mapLoading.style.display = "flex";

  const isOptimize = optimizeOrderCheckbox ? optimizeOrderCheckbox.checked : true;

  const waypointsPayload = selectedPlaces.map(function (p) {
    return {
      name: p.name,
      latitude: p.latitude,
      longitude: p.longitude,
    };
  });

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
      startLat: startLat,
      startLon: startLon,
      waypoints: waypointsPayload,
      optimizeOrder: isOptimize,
      destinationName: selectedPlaces[0].name,
      destLat: selectedPlaces[0].latitude,
      destLon: selectedPlaces[0].longitude,
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

      // Nếu backend đã tối ưu thứ tự ghé thăm (TSP), cập nhật lại thứ tự selectedPlaces
      if (Array.isArray(data.orderedWaypoints) && data.orderedWaypoints.length > 1) {
        const reordered = [];
        data.orderedWaypoints.forEach(function (ow) {
          const match = selectedPlaces.find(function (sp) {
            return (
              Math.abs(sp.latitude - ow.latitude) < 0.0001 &&
              Math.abs(sp.longitude - ow.longitude) < 0.0001
            );
          });
          if (match) reordered.push(match);
        });

        if (reordered.length === selectedPlaces.length) {
          selectedPlaces = reordered;
          updateSelectionUI();
        }
      }

      // Hiển thị banner kết quả
      const dist = Number(data.totalDistance).toFixed(2);
      const estMin = Math.max(1, Math.round(data.totalDistance * 3));

      if (selectedPlaces.length === 1) {
        routeBannerTitle.textContent = `${userLocation} ➔ ${selectedPlaces[0].name}`;
      } else {
        routeBannerTitle.textContent = isOptimize
          ? `⚡ Lộ trình tối ưu (${selectedPlaces.length} điểm dừng - TSP)`
          : `🚗 Lộ trình (${selectedPlaces.length} điểm dừng)`;
      }

      routeBannerDetails.textContent = `🚗 Quãng đường: ${dist} km • Ước tính ~${estMin} phút`;

      // Hiển thị chi tiết từng chặng (Legs)
      if (Array.isArray(data.legs) && data.legs.length > 1 && routeBannerLegs) {
        routeBannerLegs.style.display = "flex";
        routeBannerLegs.innerHTML = data.legs
          .map(function (leg, idx) {
            return `
            <div class="leg-item">
              <span class="leg-bullet">●</span>
              <span><strong>Chặng ${idx + 1}:</strong> ${truncate(leg.fromName, 14)} ➔ ${truncate(leg.toName, 14)}</span>
              <span>(${leg.distanceKm} km • ~${Math.round(leg.durationMin)} phút)</span>
            </div>`;
          })
          .join("");
      } else if (routeBannerLegs) {
        routeBannerLegs.style.display = "none";
        routeBannerLegs.innerHTML = "";
      }

      routeBanner.style.display = "flex";
      mapLoading.style.display = "none";
    })
    .catch(function (error) {
      console.log("Routing error:", error);
      mapLoadingText.textContent = "Không tìm thấy tuyến đường.";
      setTimeout(function () {
        mapLoading.style.display = "none";
      }, 2000);
    });
}

routeButton.addEventListener("click", function () {
  findRoute();
});

if (clearSelectionBtn) {
  clearSelectionBtn.addEventListener("click", function () {
    clearAllSelections();
  });
}

if (optimizeOrderCheckbox) {
  optimizeOrderCheckbox.addEventListener("change", function () {
    if (selectedPlaces.length > 1 && map.getSource("shortestRoute")) {
      findRoute();
    }
  });
}

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
      address: "114 Lê Văn Việt, TP. Thủ Đức",
      latitude: startLat - 0.0033,
      longitude: startLon + 0.0067,
      type: "COFFEE",
      price: 49000,
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
      name: "Katinat Saigon Kafe - Lê Văn Việt",
      address: "128 Lê Văn Việt, TP. Thủ Đức",
      latitude: startLat - 0.0025,
      longitude: startLon + 0.0048,
      type: "COFFEE",
      price: 55000,
    },
    {
      id: 4,
      name: "Quán Cơm Niêu Sài Gòn",
      address: "200 Võ Văn Ngân, TP. Thủ Đức",
      latitude: startLat + 0.0012,
      longitude: startLon - 0.0041,
      type: "FOOD",
      price: 75000,
    },
    {
      id: 5,
      name: "Phở 24 - Vincom Lê Văn Việt",
      address: "Tầng 4 Vincom Plaza",
      latitude: startLat - 0.0039,
      longitude: startLon + 0.0066,
      type: "FOOD",
      price: 65000,
    },
    {
      id: 6,
      name: "CGV Cinemas - Vincom Lê Văn Việt",
      address: "Tầng 5, Vincom Plaza Lê Văn Việt",
      latitude: startLat - 0.0042,
      longitude: startLon + 0.0068,
      type: "ENTERTAINMENT",
      price: 95000,
    },
    {
      id: 7,
      name: "Bida Club 68 - Hoàng Diệu 2",
      address: "68 Hoàng Diệu 2, TP. Thủ Đức",
      latitude: startLat + 0.0047,
      longitude: startLon - 0.0001,
      type: "ENTERTAINMENT",
      price: 70000,
    },
    {
      id: 8,
      name: "Vincom Plaza Lê Văn Việt",
      address: "50 Lê Văn Việt, Hiệp Phú, TP. Thủ Đức",
      latitude: startLat - 0.0040,
      longitude: startLon + 0.0065,
      type: "SHOPPING",
      price: 0,
    },
    {
      id: 9,
      name: "Nhà Sách Fahasa Thủ Đức",
      address: "04 Tô Ngọc Vân, Linh Tây, TP. Thủ Đức",
      latitude: startLat + 0.0062,
      longitude: startLon - 0.0128,
      type: "SHOPPING",
      price: 0,
    },
    {
      id: 10,
      name: "Thư viện ĐH Sư phạm Kỹ thuật",
      address: "Tòa nhà Trung tâm, 1 Võ Văn Ngân",
      latitude: startLat + 0.0010,
      longitude: startLon + 0.0007,
      type: "STUDY",
      price: 0,
    },
    {
      id: 11,
      name: "The Study Hub - Không Gian Tự Học",
      address: "38 Dân Chủ, Bình Thọ, TP. Thủ Đức",
      latitude: startLat + 0.0032,
      longitude: startLon - 0.0023,
      type: "STUDY",
      price: 30000,
    },
    {
      id: 12,
      name: "Công viên Làng Đại Học Quốc Gia",
      address: "ĐHQG TP.HCM, TP. Thủ Đức",
      latitude: startLat + 0.0217,
      longitude: startLon + 0.0297,
      type: "RELAX",
      price: 0,
    },
  ];
}
