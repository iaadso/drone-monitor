#include "DataStore.h"
#include <fstream>
#include <filesystem>

DataStore::DataStore() {
    drones_file = "data/drones.json";
    alerts_file = "data/alerts.json";
    ensure_dir();
}

void DataStore::ensure_dir() {
    std::filesystem::create_directories("data");
}

json DataStore::load_json(const std::string& file) {
    std::ifstream ifs(file, std::ios::binary);
    if (ifs.is_open()) {
        json j;
        ifs >> j;
        return j;
    }
    return json::array();
}

void DataStore::save_json(const std::string& file, const json& data) {
    std::ofstream ofs(file, std::ios::binary);
    ofs << std::setw(2) << data << std::endl;
}

void DataStore::save_drone(const DroneInfo& drone) {
    json drones = load_json(drones_file);
    bool found = false;
    
    for (auto& d : drones) {
        if (d["drone_id"] == drone.drone_id) {
            d = drone.to_json();
            found = true;
            break;
        }
    }
    
    if (!found) {
        drones.push_back(drone.to_json());
    }
    
    save_json(drones_file, drones);
}

DroneInfo DataStore::get_drone(const std::string& drone_id) {
    json drones = load_json(drones_file);
    for (const auto& d : drones) {
        if (d["drone_id"] == drone_id) {
            DroneInfo info;
            info.drone_id = d["drone_id"];
            info.name = d.contains("name") ? d["name"].get<std::string>() : d["drone_id"].get<std::string>();
            info.status = d["status"];
            info.battery = d["battery"];
            info.latitude = d["latitude"];
            info.longitude = d["longitude"];
            info.altitude = d["altitude"];
            info.temperature = d.contains("temperature") ? d["temperature"].get<double>() : 25.0;
            info.smoke_concentration = d.contains("smoke_concentration") ? d["smoke_concentration"].get<double>() : 10.0;
            info.fire_confidence = d.contains("fire_confidence") ? d["fire_confidence"].get<double>() : 5.0;
            info.last_heartbeat = d["last_heartbeat"];
            
            if (d.contains("trajectory")) {
                for (const auto& pt : d["trajectory"]) {
                    info.trajectory.emplace_back(pt[0], pt[1]);
                }
            }
            return info;
        }
    }
    return DroneInfo();
}

std::vector<DroneInfo> DataStore::get_all_drones() {
    std::vector<DroneInfo> result;
    json drones = load_json(drones_file);
    for (const auto& d : drones) {
        DroneInfo info;
        info.drone_id = d["drone_id"];
        info.name = d.contains("name") ? d["name"].get<std::string>() : d["drone_id"].get<std::string>();
        info.status = d["status"];
        info.battery = d["battery"];
        info.latitude = d["latitude"];
        info.longitude = d["longitude"];
        info.altitude = d["altitude"];
        info.temperature = d.contains("temperature") ? d["temperature"].get<double>() : 25.0;
        info.smoke_concentration = d.contains("smoke_concentration") ? d["smoke_concentration"].get<double>() : 10.0;
        info.fire_confidence = d.contains("fire_confidence") ? d["fire_confidence"].get<double>() : 5.0;
        info.last_heartbeat = d["last_heartbeat"];
        
        if (d.contains("trajectory")) {
            for (const auto& pt : d["trajectory"]) {
                info.trajectory.emplace_back(pt[0], pt[1]);
            }
        }
        
        result.push_back(info);
    }
    return result;
}

bool DataStore::delete_drone(const std::string& drone_id) {
    json drones = load_json(drones_file);
    json new_drones = json::array();
    bool found = false;
    
    for (const auto& d : drones) {
        if (d["drone_id"] != drone_id) {
            new_drones.push_back(d);
        } else {
            found = true;
        }
    }
    
    if (found) {
        save_json(drones_file, new_drones);
    }
    
    return found;
}

void DataStore::save_alert(const json& alert) {
    json alerts = load_json(alerts_file);
    alerts.push_back(alert);
    save_json(alerts_file, alerts);
}

std::vector<json> DataStore::get_all_alerts() {
    return load_json(alerts_file);
}

bool DataStore::update_alert_status(const std::string& alert_id, int status) {
    json alerts = load_json(alerts_file);
    for (auto& a : alerts) {
        if (a["alert_id"] == alert_id) {
            a["status"] = status;
            save_json(alerts_file, alerts);
            return true;
        }
    }
    return false;
}

json DroneInfo::to_json() const {
    json j;
    j["drone_id"] = drone_id;
    j["name"] = name;
    j["status"] = status;
    j["battery"] = battery;
    j["latitude"] = latitude;
    j["longitude"] = longitude;
    j["altitude"] = altitude;
    j["temperature"] = temperature;
    j["smoke_concentration"] = smoke_concentration;
    j["fire_confidence"] = fire_confidence;
    j["last_heartbeat"] = last_heartbeat;
    
    json traj = json::array();
    for (const auto& pt : trajectory) {
        json p;
        p.push_back(pt.first);
        p.push_back(pt.second);
        traj.push_back(p);
    }
    j["trajectory"] = traj;
    
    return j;
}