#include "Drone.h"
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <set>
#include <sys/stat.h>
#include <windows.h>

std::string server_url = "http://localhost:8080";
int update_interval = 2;

// 获取可执行文件所在目录
std::string get_exe_directory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exe_path(buffer);
    size_t pos = exe_path.find_last_of("\\/");
    if (pos != std::string::npos) {
        return exe_path.substr(0, pos);
    }
    return "";
}

// 获取配置文件路径（基于可执行文件目录）
std::string get_config_path() {
    std::string exe_dir = get_exe_directory();
    return exe_dir + "\\config.json";
}

// 获取配置文件的修改时间
std::time_t get_file_mtime(const std::string& path) {
    struct stat result;
    if (stat(path.c_str(), &result) == 0) {
        return result.st_mtime;
    }
    return 0;
}

// 从配置文件创建无人机
void create_drones_from_config(const json& config, std::map<std::string, Drone>& drones) {
    if (!config.contains("drones")) return;
    
    for (const auto& d : config["drones"]) {
        std::string id = d["drone_id"].get<std::string>();
        
        // 如果无人机已存在，不重复创建
        if (drones.find(id) != drones.end()) {
            continue;
        }
        
        std::string name = d["name"].get<std::string>();
        double lat = d["latitude"].get<double>();
        double lng = d["longitude"].get<double>();
        double alt = d["altitude"].get<double>();
        SimulationMode mode = parse_simulation_mode(d["simulation_mode"].get<std::string>());
        
        drones.emplace(id, Drone(id, name, lat, lng, alt, mode));
        std::cout << "New drone added: " << id << " (" << name << ") - mode: " << d["simulation_mode"] << std::endl;
        
        // 注册新无人机
        drones[id].register_drone(server_url);
    }
}

// 移除不在配置文件中的无人机
void remove_drones_not_in_config(const json& config, std::map<std::string, Drone>& drones) {
    if (!config.contains("drones")) return;
    
    // 获取配置文件中的所有无人机ID
    std::set<std::string> config_ids;
    for (const auto& d : config["drones"]) {
        config_ids.insert(d["drone_id"].get<std::string>());
    }
    
    // 检查当前运行的无人机
    std::vector<std::string> to_remove;
    for (auto& pair : drones) {
        if (config_ids.find(pair.first) == config_ids.end()) {
            to_remove.push_back(pair.first);
        }
    }
    
    // 移除不在配置文件中的无人机
    for (const auto& id : to_remove) {
        std::cout << "Removing drone: " << id << std::endl;
        drones.erase(id);
    }
}

int main() {
    // 获取配置文件路径
    std::string config_path = get_config_path();
    std::cout << "Config file path: " << config_path << std::endl;
    
    // 初始加载配置
    json config = load_config(config_path);
    
    if (config.empty()) {
        std::cerr << "Failed to load config.json, please ensure config file exists" << std::endl;
        return 1;
    }
    
    // 从配置获取服务器URL和更新间隔
    if (config.contains("server_url")) {
        server_url = config["server_url"].get<std::string>();
    }
    if (config.contains("update_interval")) {
        update_interval = config["update_interval"].get<int>();
    }
    
    std::cout << "Server URL: " << server_url << std::endl;
    std::cout << "Update interval: " << update_interval << " seconds" << std::endl;
    
    std::map<std::string, Drone> drones;
    
    // 创建初始无人机
    create_drones_from_config(config, drones);
    
    std::cout << "Drones registered. Starting patrol simulation..." << std::endl;
    std::cout << "Monitoring config file for changes..." << std::endl;
    
    std::time_t last_mtime = get_file_mtime(config_path);
    int count = 0;
    
    while (true) {
        // 每隔10次检查配置文件是否有变化
        if (count % 10 == 0) {
            std::time_t current_mtime = get_file_mtime(config_path);
            if (current_mtime > last_mtime) {
                std::cout << "Config file changed, reloading..." << std::endl;
                json new_config = load_config(config_path);
                if (!new_config.empty()) {
                    // 添加新无人机
                    create_drones_from_config(new_config, drones);
                    // 移除已删除的无人机
                    remove_drones_not_in_config(new_config, drones);
                    last_mtime = current_mtime;
                }
            }
        }
        
        // 更新所有无人机状态并发送数据
        for (auto& pair : drones) {
            pair.second.update_position();
            pair.second.simulate_sensor_data();
            pair.second.send_patrol_data(server_url);
        }
        
        count++;
        if (count % 15 == 0) {
            std::cout << "=== Status Report ===" << std::endl;
            std::cout << "Active drones: " << drones.size() << std::endl;
            for (auto& pair : drones) {
                auto data = pair.second.get_data();
                std::cout << "  " << pair.first << " (" << data["name"] << ") - "
                          << "Battery: " << data["battery"] << "%, "
                          << "Temp: " << data["temperature"] << "C, "
                          << "Smoke: " << data["smoke_concentration"] << "%, "
                          << "Fire: " << data["fire_confidence"] << "%" << std::endl;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(update_interval));
    }
    
    return 0;
}