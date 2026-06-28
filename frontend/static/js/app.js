// 从配置文件读取后端地址
let API_BASE = 'http://localhost:8080/api';

fetch('config.json')
    .then(res => res.json())
    .then(config => {
        if (config.backend_url) {
            API_BASE = config.backend_url + '/api';
        }
    })
    .catch(() => {
        console.log('Using default backend URL');
    });

function fetchDrones() {
    return fetch(`${API_BASE}/drones`)
        .then(res => res.json());
}

function fetchAlerts() {
    return fetch(`${API_BASE}/alerts`)
        .then(res => res.json());
}

function updateAlertStatus(alertId, status) {
    return fetch(`${API_BASE}/alerts/${alertId}`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ status })
    }).then(res => res.json());
}

function formatTime(timestamp) {
    return timestamp || '-';
}

function getBatteryClass(battery) {
    if (battery > 50) return 'high';
    if (battery > 20) return 'medium';
    return 'low';
}

function getAlertLevelLabel(level) {
    const labels = ['low', 'medium', 'high'];
    return labels[level] || 'low';
}

function getAlertStatusLabel(status) {
    const labels = ['unhandled', 'confirmed', 'ignored'];
    return labels[status] || 'unhandled';
}