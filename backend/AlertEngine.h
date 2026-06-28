#ifndef ALERT_ENGINE_H
#define ALERT_ENGINE_H

#include "json.hpp"
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

enum class AlertLevel {
    LOW,
    MEDIUM,
    HIGH
};

enum class AlertStatus {
    UNHANDLED,
    CONFIRMED,
    IGNORED
};

struct AlertRecord {
    std::string alert_id;
    std::string alert_type;        // 异常类型: temperature, smoke, fire_confidence, battery, continuous
    std::string drone_id;
    std::string drone_name;       // 无人机名称
    double latitude;
    double longitude;
    double altitude;               // 高度
    double battery;                // 电量
    double temperature;           // 温度
    double smoke_concentration;    // 烟雾浓度
    double fire_confidence;        // 火情置信度
    std::string drone_status;     // 无人机状态
    AlertLevel level;
    std::string reason;
    std::string timestamp;
    AlertStatus alert_status;      // 告警处理状态
    std::vector<std::array<double, 2>> trajectory; // 历史轨迹

    json to_json() const;
};

class AlertEngine {
public:
    AlertEngine();
    
    std::vector<AlertRecord> check_alerts(const json& drone_data, 
                                          const std::string& drone_id,
                                          double lat, double lng);

    // 保存告警到文件（包含完整无人机信息）
    void save_alert_to_file(const AlertRecord& alert, const std::string& alerts_dir);

    // 批量保存告警列表
    void save_alerts_to_file(const std::vector<AlertRecord>& alerts, const std::string& alerts_dir);

private:
    std::map<std::string, int> anomaly_counts;
    std::map<std::string, bool> continuous_alert_triggered;
    
    std::string generate_id(const std::string& drone_id, const std::string& alert_type) const;
    std::string get_current_time() const;
    std::string get_current_time_filename() const;  // 用于文件名的时间格式
    bool check_temperature(const json& data);
    bool check_smoke(const json& data);
    bool check_fire_confidence(const json& data);
    bool check_battery(const json& data);
    bool check_continuous_anomaly(const std::string& drone_id);
};

#endif