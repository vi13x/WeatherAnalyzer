// script.js
document.addEventListener('DOMContentLoaded', function() {
    // Navigation
    const navLinks = document.querySelectorAll('nav a');
    const sections = document.querySelectorAll('.section');

    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();

            // Remove active class from all links and sections
            navLinks.forEach(l => l.classList.remove('active'));
            sections.forEach(s => s.classList.remove('active'));

            // Add active class to clicked link
            this.classList.add('active');

            // Show corresponding section
            const sectionId = this.getAttribute('data-section');
            document.getElementById(sectionId).classList.add('active');

            // Scroll to top
            window.scrollTo(0, 0);
        });
    });

    // Chart functionality
    const updateChartBtn = document.getElementById('update-chart');
    const periodSelect = document.getElementById('period');
    const customDateGroup = document.querySelector('.custom-date');

    // Show/hide custom date inputs
    periodSelect.addEventListener('change', function() {
        if (this.value === 'custom') {
            customDateGroup.style.display = 'flex';
        } else {
            customDateGroup.style.display = 'none';
        }
    });

    // Initialize chart
    const ctx = document.getElementById('weather-chart').getContext('2d');
    let weatherChart;

    function initChart() {
        const dataType = document.getElementById('data-type').value;
        const chartType = document.getElementById('chart-type').value;

        // Sample data based on selection
        let labels, data, label, borderColor, backgroundColor;

        switch(document.getElementById('period').value) {
            case '7day':
                labels = ['Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Вс'];
                break;
            case '30day':
                labels = Array.from({length: 30}, (_, i) => (i+1) + ' мая');
                break;
            case '3month':
                labels = ['Март', 'Апрель', 'Май'];
                break;
            case '1year':
                labels = ['Янв', 'Фев', 'Мар', 'Апр', 'Май', 'Июн', 'Июл', 'Авг', 'Сен', 'Окт', 'Ноя', 'Дек'];
                break;
            default:
                labels = ['Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Вс'];
        }

        // Generate sample data based on data type
        switch(dataType) {
            case 'temperature':
                label = 'Температура (°C)';
                data = labels.map(() => Math.floor(Math.random() * 30) + 5); // 5-35°C
                borderColor = 'rgb(255, 99, 132)';
                backgroundColor = 'rgba(255, 99, 132, 0.2)';
                break;
            case 'humidity':
                label = 'Влажность (%)';
                data = labels.map(() => Math.floor(Math.random() * 60) + 30); // 30-90%
                borderColor = 'rgb(54, 162, 235)';
                backgroundColor = 'rgba(54, 162, 235, 0.2)';
                break;
            case 'pressure':
                label = 'Давление (гПа)';
                data = labels.map(() => Math.floor(Math.random() * 40) + 1000); // 1000-1040 гПа
                borderColor = 'rgb(75, 192, 192)';
                backgroundColor = 'rgba(75, 192, 192, 0.2)';
                break;
            case 'precipitation':
                label = 'Осадки (мм)';
                data = labels.map(() => Math.floor(Math.random() * 15)); // 0-15 мм
                borderColor = 'rgb(153, 102, 255)';
                backgroundColor = 'rgba(153, 102, 255, 0.2)';
                break;
            case 'wind':
                label = 'Скорость ветра (м/с)';
                data = labels.map(() => (Math.random() * 10).toFixed(1)); // 0-10 м/с
                borderColor = 'rgb(255, 159, 64)';
                backgroundColor = 'rgba(255, 159, 64, 0.2)';
                break;
        }

        // Destroy existing chart if it exists
        if (weatherChart) {
            weatherChart.destroy();
        }

        // Create new chart
        weatherChart = new Chart(ctx, {
            type: chartType,
            data: {
                labels: labels,
                datasets: [{
                    label: label,
                    data: data,
                    borderColor: borderColor,
                    backgroundColor: chartType === 'line' ? 'transparent' : backgroundColor,
                    borderWidth: 2,
                    fill: chartType === 'line'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: dataType !== 'temperature'
                    }
                },
                plugins: {
                    title: {
                        display: true,
                        text: `${label} - ${document.getElementById('city').selectedOptions[0].text}`
                    },
                    legend: {
                        position: 'top',
                    }
                }
            }
        });
    }

    // Update chart on button click
    updateChartBtn.addEventListener('click', initChart);

    // Initialize chart on page load if analysis section is active
    if (document.getElementById('analysis').classList.contains('active')) {
        initChart();
    }

    // API Test functionality
    const testApiBtn = document.getElementById('test-api');
    const apiResponseContent = document.getElementById('api-response-content');

    testApiBtn.addEventListener('click', function() {
        const endpoint = document.getElementById('api-endpoint').value;
        const params = document.getElementById('api-params').value;

        // Simulate API response
        let response;

        if (endpoint === '/api/weather') {
            response = {
                "city": "moscow",
                "start_date": "2023-01-01",
                "end_date": "2023-01-31",
                "data": [
                    {"date": "2023-01-01", "temperature": -5.2, "humidity": 85},
                    {"date": "2023-01-02", "temperature": -7.1, "humidity": 82},
                    {"date": "2023-01-03", "temperature": -3.5, "humidity": 78}
                ]
            };
        } else if (endpoint === '/api/stats') {
            response = {
                "city": "moscow",
                "year": 2023,
                "metric": "temperature",
                "stats": {
                    "mean": 5.3,
                    "min": -15.2,
                    "max": 28.7,
                    "std": 8.4
                }
            };
        }

        apiResponseContent.textContent = JSON.stringify(response, null, 2);
    });

    // Export data functionality
    const exportDataBtn = document.getElementById('export-data');

    exportDataBtn.addEventListener('click', function() {
        alert('В реальном приложении здесь будет функция экспорта данных');
    });

    // Load data functionality
    const loadDataBtn = document.getElementById('load-data');

    loadDataBtn.addEventListener('click', function() {
        alert('Загрузка данных...');
        // In a real app, this would fetch data from an API
    });

    // Mobile menu functionality
    const mobileMenuBtn = document.querySelector('.mobile-menu-btn');
    const nav = document.querySelector('nav');

    mobileMenuBtn.addEventListener('click', function() {
        nav.style.display = nav.style.display === 'flex' ? 'none' : 'flex';
    });

    // Adjust navigation for mobile on resize
    window.addEventListener('resize', function() {
        if (window.innerWidth > 768) {
            nav.style.display = 'flex';
        } else {
            nav.style.display = 'none';
        }
    });
});