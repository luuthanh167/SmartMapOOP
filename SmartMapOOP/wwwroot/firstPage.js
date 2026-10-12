const findButton = document.getElementById("findButton");
const locationInput = document.getElementById("location");
const locationSuggestions = document.getElementById("locationSuggestions");
const budgetInput = document.getElementById("budget");

const locations = [
  "Đại học Công nghệ Kỹ thuật TP.HCM",
  "Đại học Sư phạm Kỹ thuật TP.HCM",
  "Vincom Plaza Lê Văn Việt",
  "Làng Đại học Quốc gia TP.HCM",
  "Đại học Văn Lang",
  "Đại học Bách Khoa TP.HCM",
];

locationInput.addEventListener("input", function () {
  const text = locationInput.value.trim().toLowerCase();

  locationSuggestions.innerHTML = "";

  if (text === "") {
    locationSuggestions.style.display = "none";
    return;
  }

  const results = locations.filter(function (location) {
    return location.toLowerCase().includes(text);
  });

  if (results.length === 0) {
    locationSuggestions.style.display = "none";
    return;
  }

  locationSuggestions.style.display = "block";

  results.forEach(function (location) {
    const item = document.createElement("div");

    item.className = "suggestion-item";
    item.textContent = location;

    item.addEventListener("click", function () {
      locationInput.value = location;
      locationSuggestions.innerHTML = "";
      locationSuggestions.style.display = "none";
    });

    locationSuggestions.appendChild(item);
  });
});

document.addEventListener("click", function (e) {
  if (!locationInput.contains(e.target) && !locationSuggestions.contains(e.target)) {
    locationSuggestions.style.display = "none";
  }
});

budgetInput.addEventListener("focus", function () {
  budgetInput.value = budgetInput.value.replace(/\./g, "");
});

budgetInput.addEventListener("input", function () {
  budgetInput.value = budgetInput.value.replace(/\D/g, "");
});

budgetInput.addEventListener("blur", function () {
  const value = budgetInput.value;

  if (value === "") {
    return;
  }

  budgetInput.value = Number(value).toLocaleString("vi-VN");
});

findButton.addEventListener("click", function () {
  const location = locationInput.value.trim();
  const category = document.getElementById("category").value;
  const budgetText = budgetInput.value;
  const distanceText = document.getElementById("distance").value.trim();

  const budget = Number(budgetText.replace(/\./g, ""));
  const distance = Number(distanceText);

  if (location === "") {
    alert("Please enter your location.");
    return;
  }

  if (budgetText === "" || isNaN(budget) || budget < 0) {
    alert("Please enter a valid budget.");
    return;
  }

  if (distanceText === "" || isNaN(distance) || distance < 0) {
    alert("Please enter a valid distance.");
    return;
  }

  window.location.href =
    "secondPage.html" +
    "?location=" +
    encodeURIComponent(location) +
    "&category=" +
    encodeURIComponent(category) +
    "&budget=" +
    budget +
    "&distance=" +
    encodeURIComponent(distance);
});
