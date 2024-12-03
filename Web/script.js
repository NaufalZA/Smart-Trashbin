const API_URL = "https://smart-trashbin-api.onrender.com/api";
// const API_URL = "http://localhost:3000/api";
const categoryLabels = ["Organik", "Anorganik", "B3"];
const chartColors = ["#4CAF50", "#2196F3", "#F44336"];

// Hapus variabel chart sebelumnya
let dailyPlot, monthlyPlot, rangePlot;
let currentData = []; // Add this at the top with other global variables

function createPlot(divId) {
    const trace = {
        x: categoryLabels,
        y: [0, 0, 0],
        type: 'bar',
        marker: {
            color: chartColors,
            borderRadius: 8,
            opacity: 0.8
        },
        hovertemplate: '<b>%{x}</b><br>Jumlah: %{y}<extra></extra>',
        transitions: {
            duration: 500,
            easing: 'cubic-in-out'
        }
    };

    const layout = {
        autosize: true,
        height: 280,
        title: {
            text: divId.replace('Chart', '').toUpperCase(),
            font: {
                size: 16,
                weight: 600
            }
        },
        font: {
            size: 12,
            color: getComputedStyle(document.body).getPropertyValue('--chart-font-color'),
            family: "poppins, san-serif",
        },
        margin: { t: 40, b: 30, l: 30, r: 20 },
        plot_bgcolor: getComputedStyle(document.body).getPropertyValue('--chart-background'),
        paper_bgcolor: getComputedStyle(document.body).getPropertyValue('--chart-background'),
        xaxis: {
            color: getComputedStyle(document.body).getPropertyValue('--chart-axis-color'),
            linecolor: getComputedStyle(document.body).getPropertyValue('--chart-axis-color'),
            tickfont: { size: 13 },
            gridcolor: 'rgba(128,128,128,0.1)' // Grid lines lebih subtle
        },
        yaxis: {
            color: getComputedStyle(document.body).getPropertyValue('--chart-axis-color'),
            linecolor: getComputedStyle(document.body).getPropertyValue('--chart-axis-color'),
            tickfont: { size: 13 },
            gridcolor: 'rgba(128,128,128,0.1)', // Grid lines lebih subtle
            title: {
                text: 'Jumlah',
                standoff: 10
            }
        },
        bargap: 0.4,
        shapes: [{
            type: 'rect',
            xref: 'paper',
            yref: 'paper',
            x0: 0,
            y0: 0,
            x1: 1,
            y1: 1,
            fillcolor: 'transparent',
            line: {
                color: 'rgba(255,255,255,0.1)',
                width: 1
            }
        }],
        hoverlabel: {
            bgcolor: 'rgba(255,255,255,0.9)',
            font: { color: '#333' },
            bordercolor: 'transparent'
        },
        showlegend: false,
        modebar: { 
            bgcolor: 'transparent',
            color: '#d3d3d3'
        }
    };

    const config = { 
        responsive: true,
        displayModeBar: false,
        displaylogo: false
    };
    
    Plotly.newPlot(divId, [trace], layout, config);
    return document.getElementById(divId);
}

function updatePlot(plot, data) {
    const counts = new Array(3).fill(0);
    data.forEach((item) => {
        if (item.kategori >= 1 && item.kategori <= 3) {
            counts[item.kategori - 1] = item.jumlah;
        }
    });

    const update = {
        y: [counts]
    };

    Plotly.update(plot, update);
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

const MAX_DISTANCE = 30;

function calculateFullness(distance) {
  let percentage = ((MAX_DISTANCE - distance) / MAX_DISTANCE) * 100;
  return Math.max(0, Math.min(100, percentage));
}

function updateFullnessIndicators(data) {
  const latestData = {};

  data.forEach((item) => {
    if (
      !latestData[item.kategori] ||
      new Date(item.timestamp) > new Date(latestData[item.kategori].timestamp)
    ) {
      latestData[item.kategori] = item;
    }
  });

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

    if (fullness > 80) {
      progressBar.classList.add("bg-danger");
    } else if (fullness > 50) {
      progressBar.classList.add("bg-warning");
    }
  });
}

// Improve loading management
let isLoading = false;

function showLoading() {
    if (isLoading) return; // Prevent multiple overlays
    isLoading = true;
    
    const existing = document.querySelector('.loading-overlay');
    if (existing) existing.remove();
    
    const loadingDiv = document.createElement('div');
    loadingDiv.className = 'loading-overlay';
    loadingDiv.innerHTML = '<div class="spinner"></div>';
    document.body.appendChild(loadingDiv);
}

function hideLoading() {
    isLoading = false;
    const loadingDiv = document.querySelector('.loading-overlay');
    if (loadingDiv) {
        loadingDiv.classList.add('fade-out');
        setTimeout(() => loadingDiv.remove(), 300);
    }
}

// Modify updateDashboard with better error handling
async function updateDashboard() {
    try {
        showLoading();
        const [dailyData, monthlyData, recentData] = await Promise.all([
            fetchData("harian"),
            fetchData("bulanan"),
            fetchData("getdata")
        ]);

        updatePlot(dailyPlot, dailyData);
        updatePlot(monthlyPlot, monthlyData);
        updateRecentDataTable(recentData.slice(0, 10));
    } catch (error) {
        console.error("Failed to update dashboard:", error);
    } finally {
        hideLoading();
    }
}

function updateRecentDataTable(data) {
    currentData = data; // Store the data globally
    const sortSelect = document.getElementById('sortKategori');
    const selectedKategori = sortSelect.value;
    
    const filteredData = selectedKategori === 'all' 
        ? data 
        : data.filter(item => item.kategori === parseInt(selectedKategori));

    const tbody = document.querySelector("#recentData tbody");
    tbody.innerHTML = "";

    filteredData.forEach((item) => {
        const fullness = calculateFullness(item.jarak);
        const tr = document.createElement("tr");
        tr.innerHTML = `
            <td>${categoryLabels[item.kategori - 1]}</td>
            <td>${item.jarak} cm (${Math.round(fullness)}% penuh)</td>
            <td>${item.timestamp}</td>
        `;
        tbody.appendChild(tr);
    });

    updateFullnessIndicators(data);
}

async function updateRangeData(start, end) {
  const data = await fetchData(`range?start=${start}&end=${end}`);
  updatePlot(rangePlot, data);
}

document.addEventListener("DOMContentLoaded", async () => {
  dailyPlot = createPlot('dailyChart');
  monthlyPlot = createPlot('monthlyChart');
  rangePlot = createPlot('rangeChart');

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

  // Initial load
  await updateDashboard();
  
  // Set interval for updates
  setInterval(async () => {
      if (!isLoading) { // Only update if not already loading
          await updateDashboard();
      }
  }, 30000);

  // Add event listener for sort select
  document.getElementById('sortKategori').addEventListener('change', (e) => {
      updateRecentDataTable(currentData);
  });
});

// Modify theme toggler code to remove menu button references
const themeToggler = document.querySelector(".theme-toggler");

themeToggler.addEventListener("click", () => {
    document.body.classList.toggle("light-theme-variables");
    themeToggler.querySelector("span:nth-child(1)").classList.toggle("active");
    themeToggler.querySelector("span:nth-child(2)").classList.toggle("active");
    
    updateChartsTheme();
});

// Update charts theme when toggling
function updateChartsTheme() {
    const isLight = document.body.classList.contains("light-theme-variables");
    const chartUpdate = {
        paper_bgcolor: isLight ? '#fff' : '#090d3e',
        plot_bgcolor: isLight ? '#fff' : '#090d3e',
        font: {
            color: isLight ? '#444' : '#fff'
        },
        xaxis: {
            color: isLight ? '#444' : '#fff',
            linecolor: isLight ? '#444' : '#fff'
        },
        yaxis: {
            color: isLight ? '#444' : '#fff',
            linecolor: isLight ? '#444' : '#fff'
        }
    };

    [dailyPlot, monthlyPlot, rangePlot].forEach(plot => {
        if (plot) {
            Plotly.relayout(plot, chartUpdate);
        }
    });
}
