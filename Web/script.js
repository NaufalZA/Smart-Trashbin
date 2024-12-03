const API_URL = "https://smart-trashbin-api.onrender.com/api";
// const API_URL = "http://localhost:3000/api";
const categoryLabels = ["Organik", "Anorganik", "B3"];
const chartColors = ["#4CAF50", "#2196F3", "#F44336"];

let dailyChart, monthlyChart, rangeChart;

function createChart(ctx, type = "bar") {
  return new Chart(ctx, {
    type: type,
    data: {
      labels: categoryLabels,
      datasets: [
        {
          data: [],
          backgroundColor: chartColors,
          borderWidth: 1,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          beginAtZero: true,
          ticks: {
            stepSize: 1,
          },
        },
      },
      plugins: {
        legend: {
          display: false,
        },
      },
    },
  });
}

async function fetchData(endpoint) {
  try {
    const response = await fetch(`${API_URL}/${endpoint}`);
    const data = await response.json();
    return data;
  } catch (error) {
    console.error("Error fetching data:", error);
    return [];
  }
}

function updateChart(chart, data) {
  const counts = new Array(3).fill(0);
  data.forEach((item) => {
    if (item.kategori >= 1 && item.kategori <= 3) {
      counts[item.kategori - 1] = item.jumlah;
    }
  });
  chart.data.datasets[0].data = counts;
  chart.update();
}

const MAX_DISTANCE = 30; // Jarak maksimum sensor (cm)

function calculateFullness(distance) {
  // Konversi jarak ke persentase kepenuhan
  // Semakin dekat jarak (kecil), semakin penuh (besar persentasenya)
  let percentage = ((MAX_DISTANCE - distance) / MAX_DISTANCE) * 100;
  return Math.max(0, Math.min(100, percentage)); // Pastikan nilai antara 0-100
}

function updateFullnessIndicators(data) {
  const latestData = {};

  // Ambil data terbaru untuk setiap kategori
  data.forEach((item) => {
    if (
      !latestData[item.kategori] ||
      new Date(item.timestamp) > new Date(latestData[item.kategori].timestamp)
    ) {
      latestData[item.kategori] = item;
    }
  });

  // Update indikator untuk setiap kategori
  const categories = [
    { id: 1, name: "organik" },
    { id: 2, name: "anorganik" },
    { id: 3, name: "b3" },
  ];

  categories.forEach((category) => {
    const data = latestData[category.id];
    const fullness = data ? calculateFullness(data.jarak) : 0;

    const progressBar = document.getElementById(`${category.name}Progress`);
    const valueSpan = document.getElementById(`${category.name}Value`);

    progressBar.style.width = `${fullness}%`;
    valueSpan.textContent = `${Math.round(fullness)}%`;

    // Ubah warna berdasarkan tingkat kepenuhan
    if (fullness > 80) {
      progressBar.classList.add("bg-danger");
    } else if (fullness > 50) {
      progressBar.classList.add("bg-warning");
    }
  });
}

async function updateDashboard() {
  const [dailyData, monthlyData] = await Promise.all([
    fetchData("harian"),
    fetchData("bulanan"),
  ]);

  updateChart(dailyChart, dailyData);
  updateChart(monthlyChart, monthlyData);

  const recentData = await fetchData("getdata");
  updateRecentDataTable(recentData.slice(0, 10));
}

function updateRecentDataTable(data) {
  const tbody = document.querySelector("#recentData tbody");
  tbody.innerHTML = "";

  data.forEach((item) => {
    const fullness = calculateFullness(item.jarak);
    const tr = document.createElement("tr");
    tr.innerHTML = `
            <td>${categoryLabels[item.kategori - 1]}</td>
            <td>${item.jarak} cm (${Math.round(fullness)}% penuh)</td>
            <td>${item.timestamp}</td>
        `;
    tbody.appendChild(tr);
  });

  // Update indikator kepenuhan
  updateFullnessIndicators(data);
}

async function updateRangeData(start, end) {
  const data = await fetchData(`range?start=${start}&end=${end}`);
  updateChart(rangeChart, data);
}

document.addEventListener("DOMContentLoaded", () => {
  const dailyCtx = document.getElementById("dailyChart").getContext("2d");
  const monthlyCtx = document.getElementById("monthlyChart").getContext("2d");
  const rangeCtx = document.getElementById("rangeChart").getContext("2d");

  dailyChart = createChart(dailyCtx);
  monthlyChart = createChart(monthlyCtx);
  rangeChart = createChart(rangeCtx);

  $("#dateRange").daterangepicker(
    {
      startDate: moment().subtract(7, "days"),
      endDate: moment(),
      locale: {
        format: "YYYY-MM-DD",
      },
    },
    function (start, end) {
      updateRangeData(start.format("YYYY-MM-DD"), end.format("YYYY-MM-DD"));
    }
  );

  updateDashboard();
  setInterval(updateDashboard, 30000); // Update every 30 seconds
});
