#include "Drone.h"
#include "cpp-httplib.h"
#include <ctime>
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>

// 状态枚举转字符串
std::string drone_status_to_string(DroneStatus status) {
    switch (status) {
        case DroneStatus::ONLINE: return "online";
        case DroneStatus::OFFLINE: return "offline";
        case DroneStatus::CRUISING: return "cruising";
        case DroneStatus::RETURNING: return "returning";
        default: return "online";
    }
}

std::string Drone::get_status_string() const {
    return drone_status_to_string(drone_status);
}

// 基础构造函数
Drone::Drone(const std::string& id, double lat, double lng) 
    : drone_id(id), drone_name(id), latitude(lat), longitude(lng), 
      altitude(100.0), battery(100.0), temperature(25.0), 
      smoke_concentration(10.0), fire_confidence(5.0),
      sim_mode(SimulationMode::NORMAL), patrol_index(0), 
      drone_status(DroneStatus::CRUISING) {
    generate_patrol_route();
}

// 扩展构造函数
Drone::Drone(const std::string& id, const std::string& name, double lat, double lng, 
             double alt, SimulationMode mode) 
    : drone_id(id), drone_name(name), latitude(lat), longitude(lng),
      altitude(alt), battery(100.0), temperature(25.0),
      smoke_concentration(10.0), fire_confidence(5.0),
      sim_mode(mode), patrol_index(0), drone_status(DroneStatus::CRUISING) {
    generate_patrol_route();
}

void Drone::generate_patrol_route() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 0.002);
    
    for (int i = 0; i < 20; i++) {
        PatrolPoint pt;
        pt.latitude = latitude + d(gen);
        pt.longitude = longitude + d(gen);
        patrol_route.push_back(pt);
    }
}

void Drone::register_drone(const std::string& server_url) {
    httplib::Client cli(server_url);
    cli.set_connection_timeout(30);
    cli.set_read_timeout(30);
    
    json data;
    data["drone_id"] = drone_id;
    data["name"] = drone_name;
    data["latitude"] = latitude;
    data["longitude"] = longitude;
    data["altitude"] = altitude;
    data["battery"] = battery;
    data["temperature"] = temperature;
    data["smoke_concentration"] = smoke_concentration;
    data["fire_confidence"] = fire_confidence;
    data["timestamp"] = get_current_time();
    
    std::cout << "Registering drone: " << drone_id << " (" << drone_name << ") to " << server_url << std::endl;
    auto res = cli.Post(("/api/drones/" + drone_id).c_str(), 
                        data.dump(), "application/json");
    
    if (res) {
        std::cout << "Register response: " << res->status << std::endl;
    } else {
        std::cout << "Register failed: " << static_cast<int>(res.error()) << std::endl;
    }
}

void Drone::send_heartbeat(const std::string& server_url) {
    httplib::Client cli(server_url);
    cli.set_connection_timeout(30);
    cli.set_read_timeout(30);
    
    // 根据电量自动更新状态
    if (battery < 20.0) {
        drone_status = DroneStatus::RETURNING;  // 电量低于20%返航
    } else {
        drone_status = DroneStatus::CRUISING;   // 正常巡航
    }
    
    // 心跳检测：只发送关键字段（轻量级）
    json data;
    data["drone_id"] = drone_id;
    data["battery"] = battery;
    data["latitude"] = latitude;
    data["longitude"] = longitude;
    data["altitude"] = altitude;
    data["status"] = get_status_string();
    data["timestamp"] = get_current_time();
    
    auto res = cli.Post(("/api/drones/" + drone_id).c_str(),
                        data.dump(), "application/json");
}

void Drone::send_patrol_data(const std::string& server_url) {
    httplib::Client cli(server_url);
    cli.set_connection_timeout(30);
    cli.set_read_timeout(30);
    
    json data = get_data();
    
    auto res = cli.Post(("/api/drones/" + drone_id).c_str(),
                        data.dump(), "application/json");
    
    if (res && res->status == 200) {
        std::cout << "Drone " << drone_id << " (" << drone_name << ") data sent OK" << std::endl;
    } else {
        std::cout << "Drone " << drone_id << " send failed: " << (res ? std::to_string(res->status) : std::to_string(static_cast<int>(res.error()))) << std::endl;
    }
}

void Drone::update_position() {
    if (patrol_route.empty()) return;
    
    patrol_index = (patrol_index + 1) % patrol_route.size();
    auto target = patrol_route[patrol_index];
    
    double lat_diff = target.latitude - latitude;
    double lng_diff = target.longitude - longitude;
    latitude += lat_diff * 0.1;
    longitude += lng_diff * 0.1;
    
    altitude = 80.0 + std::sin(patrol_index * 0.5) * 20.0;
    
    // 根据模式调整电量消耗
    if (sim_mode == SimulationMode::LOW_BATTERY) {
        battery -= 0.5;  // 电量快速下降
    } else {
        battery -= 0.05;
    }
    if (battery < 0) battery = 0;
}

void Drone::simulate_sensor_data() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> temp_dist(25, 5);
    std::normal_distribution<> smoke_dist(10, 3);
    std::normal_distribution<> fire_dist(5, 3);
    
    // 基础随机数据
    temperature = std::max(15.0, std::min(100.0, temp_dist(gen)));
    smoke_concentration = std::max(0.0, std::min(100.0, smoke_dist(gen)));
    fire_confidence = std::max(0.0, std::min(100.0, fire_dist(gen)));
    
    // 根据模拟模式生成特定异常数据
    apply_simulation_mode();
    
    // 根据电量自动更新状态
    if (battery < 20.0) {
        drone_status = DroneStatus::RETURNING;  // 电量低于20%返航
    } else {
        drone_status = DroneStatus::CRUISING;   // 正常巡航
    }
}

void Drone::apply_simulation_mode() {
    switch (sim_mode) {
        case SimulationMode::LOW_BATTERY:
            // 电量持续低于20%
            battery = std::max(5.0, std::min(19.0, battery));
            break;
            
        case SimulationMode::HIGH_TEMPERATURE:
            // 温度持续高于阈值(60度)
            temperature = 65.0 + (rand() % 10);
            break;
            
        case SimulationMode::HIGH_SMOKE:
            // 烟雾浓度持续高于阈值(50%)
            smoke_concentration = 55.0 + (rand() % 20);
            break;
            
        case SimulationMode::HIGH_FIRE_CONFIDENCE:
            // 火情置信度根据温度和烟雾浓度动态计算
            // 规则：温度>60度，烟雾>60%，火情置信度基础为60
            // 温度和烟雾越高，火情置信度越高
            temperature = 85.0 + (rand() % 15);  // 温度85-100度
            smoke_concentration = 70.0 + (rand() % 30);  // 烟雾70-100%
            // 动态计算火情置信度：温度每超1度+2.5分，烟雾每超1%+1分
            fire_confidence = std::min(100.0, 
                std::max(0.0, (temperature - 60) * 2.5 + (smoke_concentration - 60) * 1.0));
            break;
            
        case SimulationMode::NORMAL:
            // 正常状态，数据在正常范围内波动
            break;
    }
}

json Drone::get_data() const {
    json data;
    data["drone_id"] = drone_id;
    data["name"] = drone_name;
    data["status"] = get_status_string();
    data["latitude"] = latitude;
    data["longitude"] = longitude;
    data["altitude"] = altitude;
    data["battery"] = battery;
    data["temperature"] = temperature;
    data["smoke_concentration"] = smoke_concentration;
    data["fire_confidence"] = fire_confidence;
    data["timestamp"] = get_current_time();
    return data;
}

std::string Drone::get_current_time() const {
    time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

// 配置文件加载函数
json load_config(const std::string& config_path) {
    std::ifstream f(config_path);
    if (f.is_open()) {
        json config;
        f >> config;
        return config;
    }
    std::cerr << "Failed to load config: " << config_path << std::endl;
    return json();
}

// 解析模拟模式
SimulationMode parse_simulation_mode(const std::string& mode_str) {
    if (mode_str == "low_battery") return SimulationMode::LOW_BATTERY;
    if (mode_str == "high_temperature") return SimulationMode::HIGH_TEMPERATURE;
    if (mode_str == "high_smoke") return SimulationMode::HIGH_SMOKE;
    if (mode_str == "high_fire_confidence") return SimulationMode::HIGH_FIRE_CONFIDENCE;
    return SimulationMode::NORMAL;
}