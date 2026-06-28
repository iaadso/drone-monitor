#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

struct DroneInfo {
    std::string drone_id;
    std::string name;
    std::string status;
    double battery;
    double latitude;
    double longitude;
    double altitude;
    double temperature;
    double smoke_concentration;
    double fire_confidence;
    std::string last_heartbeat;
    std::vector<std::pair<double, double>> trajectory;

    json to_json() const;
};

class DataStore {
public:
    DataStore();
    
    void save_drone(const DroneInfo& drone);
    DroneInfo get_drone(const std::string& drone_id);
    std::vector<DroneInfo> get_all_drones();
    bool delete_drone(const std::string& drone_id);
    
    void save_alert(const json& alert);
    std::vector<json> get_all_alerts();
    bool update_alert_status(const std::string& alert_id, int status);
    
private:
    std::string drones_file;
    std::string alerts_file;
    
    void ensure_dir();
    json load_json(const std::string& file);
    void save_json(const std::string& file, const json& data);
};

#endif