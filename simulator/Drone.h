#ifndef DRONE_H
#define DRONE_H

#include "json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

// 模拟模式枚举
enum class SimulationMode {
    NORMAL,
    LOW_BATTERY,
    HIGH_TEMPERATURE,
    HIGH_SMOKE,
    HIGH_FIRE_CONFIDENCE
};

// 无人机状态枚举
enum class DroneStatus {
    ONLINE,      // 在线
    OFFLINE,     // 离线
    CRUISING,    // 巡航中
    RETURNING    // 返航中
};

struct PatrolPoint {
    double latitude;
    double longitude;
};

class Drone {
public:
    // 默认构造函数
    Drone() : drone_id(""), drone_name(""), latitude(0), longitude(0), 
              altitude(100.0), battery(100.0), temperature(25.0), 
              smoke_concentration(10.0), fire_confidence(5.0),
              sim_mode(SimulationMode::NORMAL), patrol_index(0), 
              drone_status(DroneStatus::CRUISING) {}
    
    // 基础构造函数
    Drone(const std::string& id, double lat, double lng);
    
    // 扩展构造函数（支持名称和模拟模式）
    Drone(const std::string& id, const std::string& name, double lat, double lng, 
          double alt, SimulationMode mode);
    
    void register_drone(const std::string& server_url);
    void send_heartbeat(const std::string& server_url);
    void send_patrol_data(const std::string& server_url);
    
    void update_position();
    void simulate_sensor_data();
    json get_data() const;
    
    const std::string& get_id() const { return drone_id; }
    const std::string& get_name() const { return drone_name; }
    SimulationMode get_mode() const { return sim_mode; }
    DroneStatus get_status() const { return drone_status; }
    void set_status(DroneStatus status) { drone_status = status; }
    std::string get_status_string() const;
    
private:
    std::string drone_id;
    std::string drone_name;
    double latitude;
    double longitude;
    double altitude;
    double battery;
    double temperature;
    double smoke_concentration;
    double fire_confidence;
    SimulationMode sim_mode;
    DroneStatus drone_status;
    int patrol_index;
    
    std::vector<PatrolPoint> patrol_route;
    
    void generate_patrol_route();
    std::string get_current_time() const;
    
    // 根据模拟模式生成特定数据
    void apply_simulation_mode();
};

// 配置文件加载函数
json load_config(const std::string& config_path);
SimulationMode parse_simulation_mode(const std::string& mode_str);
std::string drone_status_to_string(DroneStatus status);

#endif