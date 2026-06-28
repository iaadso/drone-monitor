#include "AlertEngine.h"
#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>

const double TEMP_THRESHOLD = 60.0;
const double SMOKE_THRESHOLD = 50.0;
const double FIRE_CONFIDENCE_THRESHOLD = 80.0;
const double BATTERY_THRESHOLD = 20.0;
const int CONTINUOUS_ANOMALY_THRESHOLD = 3;

AlertEngine::AlertEngine() {}

std::vector<AlertRecord> AlertEngine::check_alerts(const json& drone_data,
                                                    const std::string& drone_id,
                                                    double lat, double lng) {
    std::vector<AlertRecord> alerts;
    
    bool has_anomaly = false;
    
    // 提取完整无人机信息
    auto fill_drone_info = [&](AlertRecord& alert) {
        alert.drone_id = drone_id;
        alert.drone_name = drone_data.contains("name") ? drone_data["name"].get<std::string>() : drone_id;
        alert.latitude = lat;
        alert.longitude = lng;
        alert.altitude = drone_data.contains("altitude") ? drone_data["altitude"].get<double>() : 0.0;
        alert.battery = drone_data.contains("battery") ? drone_data["battery"].get<double>() : 0.0;
        alert.temperature = drone_data.contains("temperature") ? drone_data["temperature"].get<double>() : 0.0;
        alert.smoke_concentration = drone_data.contains("smoke_concentration") ? drone_data["smoke_concentration"].get<double>() : 0.0;
        alert.fire_confidence = drone_data.contains("fire_confidence") ? drone_data["fire_confidence"].get<double>() : 0.0;
        alert.drone_status = drone_data.contains("status") ? drone_data["status"].get<std::string>() : "unknown";
        // 复制轨迹
        if (drone_data.contains("trajectory")) {
            for (const auto& point : drone_data["trajectory"]) {
                alert.trajectory.push_back({point[0].get<double>(), point[1].get<double>()});
            }
        }
    };
    
    if (check_temperature(drone_data)) {
        AlertRecord alert;
        alert.alert_type = "temperature";
        alert.alert_id = generate_id(drone_id, alert.alert_type);
        fill_drone_info(alert);
        alert.level = AlertLevel::HIGH;
        alert.reason = "Temperature exceeds threshold (" + std::to_string(drone_data["temperature"].get<double>()) + "C > " + std::to_string(TEMP_THRESHOLD) + "C)";
        alert.timestamp = get_current_time();
        alert.alert_status = AlertStatus::UNHANDLED;
        alerts.push_back(alert);
        has_anomaly = true;
    }
    
    if (check_smoke(drone_data)) {
        AlertRecord alert;
        alert.alert_type = "smoke";
        alert.alert_id = generate_id(drone_id, alert.alert_type);
        fill_drone_info(alert);
        alert.level = AlertLevel::HIGH;
        alert.reason = "Smoke concentration exceeds threshold (" + std::to_string(drone_data["smoke_concentration"].get<double>()) + " > " + std::to_string(SMOKE_THRESHOLD) + ")";
        alert.timestamp = get_current_time();
        alert.alert_status = AlertStatus::UNHANDLED;
        alerts.push_back(alert);
        has_anomaly = true;
    }
    
    if (check_fire_confidence(drone_data)) {
        AlertRecord alert;
        alert.alert_type = "fire_confidence";
        alert.alert_id = generate_id(drone_id, alert.alert_type);
        fill_drone_info(alert);
        alert.level = AlertLevel::HIGH;
        alert.reason = "Fire detection confidence exceeds threshold (" + std::to_string(drone_data["fire_confidence"].get<double>()) + "% > " + std::to_string(FIRE_CONFIDENCE_THRESHOLD) + "%)";
        alert.timestamp = get_current_time();
        alert.alert_status = AlertStatus::UNHANDLED;
        alerts.push_back(alert);
        has_anomaly = true;
    }
    
    if (check_battery(drone_data)) {
        AlertRecord alert;
        alert.alert_type = "battery";
        alert.alert_id = generate_id(drone_id, alert.alert_type);
        fill_drone_info(alert);
        alert.level = AlertLevel::LOW;
        alert.reason = "Battery low (" + std::to_string(drone_data["battery"].get<double>()) + "% < " + std::to_string(BATTERY_THRESHOLD) + "%)";
        alert.timestamp = get_current_time();
        alert.alert_status = AlertStatus::UNHANDLED;
        alerts.push_back(alert);
        has_anomaly = true;
    }
    
    if (has_anomaly) {
        anomaly_counts[drone_id]++;
        if (check_continuous_anomaly(drone_id)) {
            AlertRecord alert;
            alert.alert_type = "continuous";
            alert.alert_id = generate_id(drone_id, alert.alert_type);
            fill_drone_info(alert);
            alert.level = AlertLevel::MEDIUM;
            alert.reason = "Continuous anomalies detected (" + std::to_string(anomaly_counts[drone_id]) + " consecutive reports)";
            alert.timestamp = get_current_time();
            alert.alert_status = AlertStatus::UNHANDLED;
            alerts.push_back(alert);
        }
    } else {
        anomaly_counts[drone_id] = 0;
        continuous_alert_triggered[drone_id] = false;
    }
    
    return alerts;
}

bool AlertEngine::check_temperature(const json& data) {
    if (data.contains("temperature")) {
        return data["temperature"].get<double>() > TEMP_THRESHOLD;
    }
    return false;
}

bool AlertEngine::check_smoke(const json& data) {
    if (data.contains("smoke_concentration")) {
        return data["smoke_concentration"].get<double>() > SMOKE_THRESHOLD;
    }
    return false;
}

bool AlertEngine::check_fire_confidence(const json& data) {
    if (data.contains("fire_confidence")) {
        return data["fire_confidence"].get<double>() > FIRE_CONFIDENCE_THRESHOLD;
    }
    return false;
}

bool AlertEngine::check_battery(const json& data) {
    if (data.contains("battery")) {
        return data["battery"].get<double>() < BATTERY_THRESHOLD;
    }
    return false;
}

bool AlertEngine::check_continuous_anomaly(const std::string& drone_id) {
    if (anomaly_counts[drone_id] >= CONTINUOUS_ANOMALY_THRESHOLD) {
        if (!continuous_alert_triggered[drone_id]) {
            continuous_alert_triggered[drone_id] = true;
            return true;
        }
    }
    return false;
}

std::string AlertEngine::generate_id(const std::string& drone_id, const std::string& alert_type) const {
    // 格式: ALERT-{drone_id}-{timestamp}-{alert_type}
    return "ALERT-" + drone_id + "-" + get_current_time_filename() + "-" + alert_type;
}

std::string AlertEngine::get_current_time() const {
    time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::string AlertEngine::get_current_time_filename() const {
    // 用于文件名的时间格式: YYYYMMDD_HHMMSS
    time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&now));
    return std::string(buf);
}

json AlertRecord::to_json() const {
    json j;
    j["alert_id"] = alert_id;
    j["alert_type"] = alert_type;
    j["drone_id"] = drone_id;
    j["drone_name"] = drone_name;
    j["latitude"] = latitude;
    j["longitude"] = longitude;
    j["altitude"] = altitude;
    j["battery"] = battery;
    j["temperature"] = temperature;
    j["smoke_concentration"] = smoke_concentration;
    j["fire_confidence"] = fire_confidence;
    j["drone_status"] = drone_status;
    j["level"] = static_cast<int>(level);
    j["reason"] = reason;
    j["timestamp"] = timestamp;
    j["status"] = static_cast<int>(alert_status);
    // 转换轨迹为JSON数组
    json traj = json::array();
    for (const auto& point : trajectory) {
        traj.push_back({point[0], point[1]});
    }
    j["trajectory"] = traj;
    return j;
}

void AlertEngine::save_alert_to_file(const AlertRecord& alert, const std::string& alerts_dir) {
    // 创建文件名：告警ID.json
    std::string filename = alerts_dir + "/" + alert.alert_id + ".json";
    std::ofstream file(filename);
    if (file.is_open()) {
        file << alert.to_json().dump(2);
        file.close();
        std::cout << "Alert saved: " << filename << std::endl;
    } else {
        std::cerr << "Failed to save alert: " << filename << std::endl;
    }
}

void AlertEngine::save_alerts_to_file(const std::vector<AlertRecord>& alerts, const std::string& alerts_dir) {
    for (const auto& alert : alerts) {
        save_alert_to_file(alert, alerts_dir);
    }
}