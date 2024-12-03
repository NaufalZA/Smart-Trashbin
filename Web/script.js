const API_URL = 'https://smart-trashbin.onrender.com/api';
const categoryLabels = ['Organik', 'Anorganik', 'B3'];
const chartColors = ['#4CAF50', '#2196F3', '#F44336'];

let dailyChart, monthlyChart, rangeChart;

function createChart(ctx, type = 'bar') {
    return new Chart(ctx, {
        type: type,
        data: {
            labels: categoryLabels,
            datasets: [{
                data: [],
                backgroundColor: chartColors,
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        stepSize: 1
                    }
                }
            },
            plugins: {
                legend: {
                    display: false
                }
            }
        }
    });
}

async function fetchData(endpoint) {
    try {
        const response = await fetch(`${API_URL}/${endpoint}`);
        const data = await response.json();
        return data;
    } catch (error) {
        console.error('Error fetching data:', error);
        return [];
    }
}

function updateChart(chart, data) {
    const counts = new Array(3).fill(0);
    data.forEach(item => {
        if (item.kategori >= 1 && item.kategori <= 3) {
            counts[item.kategori - 1] = item.jumlah;
        }
    });
    chart.data.datasets[0].data = counts;
    chart.update();
}

async function updateDashboard() {
    const [dailyData, monthlyData] = await Promise.all([
        fetchData('harian'),
        fetchData('bulanan')
    ]);

    updateChart(dailyChart, dailyData);
    updateChart(monthlyChart, monthlyData);

    const recentData = await fetchData('getdata');
    updateRecentDataTable(recentData.slice(0, 10));
}

function updateRecentDataTable(data) {
    const tbody = document.querySelector('#recentData tbody');
    tbody.innerHTML = '';
    
    data.forEach(item => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${categoryLabels[item.kategori - 1]}</td>
            <td>${item.jarak} cm</td>
            <td>${item.timestamp}</td>
        `;
        tbody.appendChild(tr);
    });
}

async function updateRangeData(start, end) {
    const data = await fetchData(`range?start=${start}&end=${end}`);
    updateChart(rangeChart, data);
}

document.addEventListener('DOMContentLoaded', () => {
    const dailyCtx = document.getElementById('dailyChart').getContext('2d');
    const monthlyCtx = document.getElementById('monthlyChart').getContext('2d');
    const rangeCtx = document.getElementById('rangeChart').getContext('2d');

    dailyChart = createChart(dailyCtx);
    monthlyChart = createChart(monthlyCtx);
    rangeChart = createChart(rangeCtx);

    $('#dateRange').daterangepicker({
        startDate: moment().subtract(7, 'days'),
        endDate: moment(),
        locale: {
            format: 'YYYY-MM-DD'
        }
    }, function(start, end) {
        updateRangeData(start.format('YYYY-MM-DD'), end.format('YYYY-MM-DD'));
    });

    updateDashboard();
    setInterval(updateDashboard, 30000); // Update every 30 seconds
});
