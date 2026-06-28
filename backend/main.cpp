#include "cpp-httplib.h"
#include "json.hpp"
#include "AlertEngine.h"
#include "DataStore.h"
#include <iostream>
#include <sstream>
#include <time.h>
#include <filesystem>
#include <windows.h>

using json = nlohmann::json;
using namespace httplib;

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

// 将相对路径转换为绝对路径（基于可执行文件所在目录）
std::string resolve_path(const std::string& path) {
    if (path.empty()) return "";
    
    // 如果已经是绝对路径，直接返回
    if (path.length() >= 2 && path[1] == ':') {
        return path;
    }
    if (path[0] == '\\' || path[0] == '/') {
        return path;
    }
    
    // 相对路径，基于可执行文件目录
    std::string exe_dir = get_exe_directory();
    if (!exe_dir.empty()) {
        return exe_dir + "\\" + path;
    }
    return path;
}

std::string get_path_param(const Request& req, const std::string& key) {
    auto it = req.path_params.find(key);
    if (it != req.path_params.end()) {
        return it->second;
    }
    return "";
}

int main() {
    Server svr;
    DataStore data_store;
    AlertEngine alert_engine;

    // 获取可执行文件所在目录
    std::string exe_dir = get_exe_directory();
    std::cout << "Executable directory: " << exe_dir << std::endl;
    
    // 读取配置文件
    int server_port = 8080;
    std::string alerts_dir = "alerts";
    bool cors_enabled = true;

    std::ifstream config_file("config.json");
    if (config_file.is_open()) {
        try {
            json config = json::parse(config_file);
            if (config.contains("server_port")) {
                server_port = config["server_port"].get<int>();
            }
            if (config.contains("alerts_dir")) {
                alerts_dir = config["alerts_dir"].get<std::string>();
            }
            if (config.contains("cors_enabled")) {
                cors_enabled = config["cors_enabled"].get<bool>();
            }
            std::cout << "Loaded config: port=" << server_port 
                      << ", alerts_dir=" << alerts_dir << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Config parse error: " << e.what() << std::endl;
        }
        config_file.close();
    } else {
        std::cout << "No config file found, using defaults: port=" << server_port << std::endl;
    }
    
    // 转换相对路径为绝对路径
    alerts_dir = resolve_path(alerts_dir);
    std::cout << "Resolved alerts_dir: " << alerts_dir << std::endl;
    
    // 创建告警目录（如果不存在）
    std::filesystem::create_directories(alerts_dir);

    // 注册新无人机接口
    svr.Post("/api/drones/register", [&](const Request& req, Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string drone_id = body["drone_id"].get<std::string>();
            
            // 检查是否已存在
            auto existing = data_store.get_drone(drone_id);
            if (!existing.drone_id.empty()) {
                res.status = 409;
                json response = {
                    {"status", "error"},
                    {"message", "Drone already exists"},
                    {"drone_id", drone_id}
                };
                res.set_content(response.dump(), "application/json");
                std::cout << "Registration failed: drone " << drone_id << " already exists" << std::endl;
                return;
            }
            
            // 创建新无人机
            DroneInfo drone;
            drone.drone_id = drone_id;
            drone.name = body.contains("name") ? body["name"].get<std::string>() : drone_id;
            drone.status = "registered";
            drone.battery = 100.0;
            drone.temperature = 25.0;
            drone.smoke_concentration = 10.0;
            drone.fire_confidence = 5.0;
            
            // 从请求中获取初始位置，如果没有则使用默认值
            if (body.contains("latitude") && body.contains("longitude")) {
                drone.latitude = body["latitude"].get<double>();
                drone.longitude = body["longitude"].get<double>();
            } else {
                // 默认位置（北京中心区域）
                drone.latitude = 39.9042;
                drone.longitude = 116.4074;
            }
            
            drone.altitude = body.contains("altitude") ? body["altitude"].get<double>() : 100.0;
            
            time_t now = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            drone.last_heartbeat = buf;
            
            // 添加初始轨迹点
            drone.trajectory.emplace_back(drone.latitude, drone.longitude);
            
            data_store.save_drone(drone);
            
            std::cout << "New drone registered: " << drone_id << " at (" << drone.latitude << ", " << drone.longitude << ")" << std::endl;
            
            res.status = 200;
            json response = {
                {"status", "success"},
                {"drone_id", drone_id},
                {"message", "Drone registered successfully"},
                {"latitude", drone.latitude},
                {"longitude", drone.longitude}
            };
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            std::cerr << "Error registering drone: " << e.what() << std::endl;
            res.status = 400;
            json response = {
                {"status", "error"},
                {"message", "Invalid request: " + std::string(e.what())}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    // 删除无人机接口
    svr.Delete("/api/drones/:id", [&](const Request& req, Response& res) {
        try {
            auto drone_id = get_path_param(req, "id");
            if (drone_id.empty()) {
                res.status = 400;
                res.set_content(R"({"status":"error","message":"Drone ID required"})", "application/json");
                return;
            }
            
            bool deleted = data_store.delete_drone(drone_id);
            if (deleted) {
                std::cout << "Drone deleted: " << drone_id << std::endl;
                res.status = 200;
                json response = {
                    {"status", "success"},
                    {"message", "Drone deleted successfully"},
                    {"drone_id", drone_id}
                };
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 404;
                json response = {
                    {"status", "error"},
                    {"message", "Drone not found"},
                    {"drone_id", drone_id}
                };
                res.set_content(response.dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid request"})", "application/json");
        }
    });

    svr.Post("/api/drones/:id", [&](const Request& req, Response& res) {
        try {
            auto drone_id = get_path_param(req, "id");
            if (drone_id.empty()) {
                res.status = 400;
                res.set_content(R"({"status":"error","message":"Drone ID required"})", "application/json");
                return;
            }
            auto body = json::parse(req.body);
            
            std::cout << "Received data from drone: " << drone_id << std::endl;
            
            double lat = body["latitude"].get<double>();
            double lng = body["longitude"].get<double>();
            double alt = body["altitude"].get<double>();
            double battery = body["battery"].get<double>();
            
            // 判断是否包含传感器数据（用于区分心跳检测和巡检数据上报）
            bool has_sensor_data = body.contains("temperature") && 
                                    body.contains("smoke_concentration") && 
                                    body.contains("fire_confidence");
            
            double temperature = has_sensor_data ? body["temperature"].get<double>() : 25.0;
            double smoke = has_sensor_data ? body["smoke_concentration"].get<double>() : 10.0;
            double fire = has_sensor_data ? body["fire_confidence"].get<double>() : 5.0;
            
            std::string name = body.contains("name") ? body["name"].get<std::string>() : drone_id;
            std::string status = body.contains("status") ? body["status"].get<std::string>() : "online";
            
            DroneInfo drone;
            drone.drone_id = drone_id;
            drone.name = name;
            drone.status = status;
            drone.battery = battery;
            drone.latitude = lat;
            drone.longitude = lng;
            drone.altitude = alt;
            drone.temperature = temperature;
            drone.smoke_concentration = smoke;
            drone.fire_confidence = fire;
            
            time_t now = time(nullptr);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
            drone.last_heartbeat = buf;
            
            auto existing = data_store.get_drone(drone_id);
            drone.trajectory = existing.trajectory;
            drone.trajectory.emplace_back(lat, lng);
            if (drone.trajectory.size() > 50) {
                drone.trajectory.erase(drone.trajectory.begin());
            }
            
            data_store.save_drone(drone);
            
            // 只有巡检数据上报时才触发告警检测
            if (has_sensor_data) {
                auto alerts = alert_engine.check_alerts(body, drone_id, lat, lng);
                for (const auto& alert : alerts) {
                    data_store.save_alert(alert.to_json());
                    // 保存告警到文件（包含完整无人机信息）
                    alert_engine.save_alert_to_file(alert, alerts_dir);
                    std::cout << "Generated alert: " << alert.alert_id << " - " << alert.reason << std::endl;
                }
            }
            
            res.status = 200;
            res.set_content(R"({"status":"success","received":true})", "application/json");
        } catch (const std::exception& e) {
            std::cerr << "Error processing drone data: " << e.what() << std::endl;
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid request"})", "application/json");
        }
    });

    svr.Get("/api/drones", [&](const Request& req, Response& res) {
        auto drones = data_store.get_all_drones();
        json result = json::array();
        for (const auto& d : drones) {
            result.push_back(d.to_json());
        }
        res.set_content(result.dump(), "application/json");
    });

    svr.Get("/api/drones/:id", [&](const Request& req, Response& res) {
        auto drone_id = get_path_param(req, "id");
        if (drone_id.empty()) {
            res.status = 400;
            res.set_content(R"({"error":"Drone ID required"})", "application/json");
            return;
        }
        auto drone = data_store.get_drone(drone_id);
        if (!drone.drone_id.empty()) {
            res.set_content(drone.to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Drone not found"})", "application/json");
        }
    });

    svr.Get("/api/alerts", [&](const Request& req, Response& res) {
        try {
            auto alerts = data_store.get_all_alerts();
            
            json result = json::array();
            int count = 0;
            int limit = 100;
            
            for (auto it = alerts.rbegin(); it != alerts.rend() && count < limit; ++it) {
                result.push_back(*it);
                count++;
            }
            
            res.set_content(result.dump(), "application/json");
        } catch (const std::exception& e) {
            std::cerr << "Error getting alerts: " << e.what() << std::endl;
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Internal server error"})", "application/json");
        }
    });

    svr.Put("/api/alerts/:id", [&](const Request& req, Response& res) {
        try {
            auto alert_id = get_path_param(req, "id");
            if (alert_id.empty()) {
                res.status = 400;
                res.set_content(R"({"status":"error","message":"Alert ID required"})", "application/json");
                return;
            }
            auto body = json::parse(req.body);
            int status = body["status"].get<int>();
            
            bool updated = data_store.update_alert_status(alert_id, status);
            if (updated) {
                res.status = 200;
                res.set_content(R"({"status":"success","updated":true})", "application/json");
            } else {
                res.status = 404;
                res.set_content(R"({"error":"Alert not found"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid request"})", "application/json");
        }
    });

    // 添加CORS支持（如果启用）
    if (cors_enabled) {
        svr.set_pre_routing_handler([](const Request& req, Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.set_header("Content-Type", "application/json; charset=utf-8");
            if (req.method == "OPTIONS") {
                res.status = 200;
                return Server::HandlerResponse::Handled;
            }
            return Server::HandlerResponse::Unhandled;
        });
    }

    svr.Get("/", [&](const Request& req, Response& res) {
        res.set_content(R"(
            <html><body>
            <h1>Drone Monitoring System</h1>
            <p>API Endpoints:</p>
            <ul>
                <li>POST /api/drones/:id - Upload drone data</li>
                <li>GET /api/drones - Get all drones</li>
                <li>GET /api/drones/:id - Get drone by ID</li>
                <li>GET /api/alerts - Get all alerts</li>
                <li>PUT /api/alerts/:id - Update alert status</li>
            </ul>
            </body></html>
        )", "text/html");
    });

    std::cout << "Server running on http://0.0.0.0:" << server_port << std::endl;
    svr.listen("0.0.0.0", server_port);

    return 0;
}