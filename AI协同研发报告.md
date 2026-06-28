# AI协同研发报告

**项目名称：** 无人机监控系统  
**研发周期：** 2026-06-28 ~ 2026-06-29  
**AI工具：** Trae AI (基于GLM-5模型)  
**报告编写日期：** 2026-06-29  

---

## 一、项目概述

### 1.1 项目背景

本项目是一个无人机巡检监控系统，用于实时监控无人机状态、传感器数据（火情检测），并提供告警功能。系统包含三个主要组件：

- **无人机模拟器（C++）**：模拟多架无人机，动态生成传感器数据
- **后端服务（C++）**：接收无人机数据，进行告警检测，数据持久化
- **前端页面（HTML/CSS/JavaScript）**：实时展示无人机状态、地图、告警信息

### 1.2 系统架构

![](C:\Users\10426\Desktop\myworkspase\test\drone-monitor\交付文档\拓补图.png)

### 1.3 技术栈

| 组件 | 技术选型 | 说明 |
|------|---------|------|
| 后端 | C++ + cpp-httplib + nlohmann/json | 轻量级HTTP服务器，JSON解析 |
| 前端 | HTML5 + CSS3 + JavaScript + Leaflet.js | 交互式地图展示 |
| 构建 | CMake + MinGW | 跨平台构建系统 |
| 协议 | HTTP REST API | 前后端数据交互 |

---

## 二、关键Prompt记录

### 2.1 环境检测相关

**Prompt 1：**
```
帮我建一个Windows的程序，用于测试电脑是否有环境，要求有makefile用于编译
```

**AI响应：**
- 提供了GCC/G++、Make、CMake、Python等工具的安装命令
- 使用Chocolatey包管理器安装开发环境
- 创建了Makefile和CMakeLists.txt构建文件

**关键Skill：** 无

---

**Prompt 2：**
```
choco install mingw -y 安装GCC/G++，这个我电脑安装了为什么没有检测出来呢
```

**AI响应：**
- 诊断了PATH环境变量配置问题
- 建议检查编译器路径是否正确添加到系统PATH
- 提供了手动验证GCC安装的命令

**关键Skill：** 无

---

### 2.2 系统架构设计

**Prompt 3：**
```
我需要三个程序：
1. 无人机模拟软件 - 通过HTTP上报数据
2. 后端程序 - 接收数据，告警检测
3. 前端页面 - 展示无人机状态、地图、告警

要求无人机模拟器和后端程序使用C++
```

**AI响应：**
- 设计了完整的系统架构
- 提供了详细的技术选型说明
- 制定了API接口规范（注册、心跳、巡检数据）
- 定义了告警规则（温度、烟雾、火情置信度、电量阈值）

**关键Skill：** 无

---

### 2.3 功能实现相关

**Prompt 4：**
```
帮我把无人机模拟器修改一下，要求增加一个配置文件，配置文件可以增加和删除无人机，配置文件需要配置无人机的编号和设备名称，无人机模拟器需要动态读取配置文件，我可能会动态在配置文件增加和删除无人机

要求一个无人机电量低于20%，一个无人机区域温度高于阈值，
一个无人机烟雾浓度高于阈值，一个无人机疑似火情置信度高于阈值，
一个无人机一切正常
```

**AI响应：**
- 设计了JSON格式的配置文件结构
- 实现了模拟器端动态配置加载功能
- 添加了文件系统监控，自动检测配置文件变化
- 定义了5种模拟模式：low_battery, high_temperature, high_smoke, high_fire_confidence, normal

**关键Prompt补充：**
```
火情置信度要求按照区域温度和烟雾浓度来判断，
区域温度超过60度，烟雾浓度超过60火情置信度为60，以此来动态确定火情置信度
```

**AI响应：**
- 修改了火情置信度计算公式：
  - 当温度>60℃且烟雾浓度>60%时，火情置信度=60
  - 否则根据温度和烟雾浓度动态计算

**关键Skill：** 无

---

### 2.4 前端优化相关

**Prompt 5：**
```
地图能否导入世界地图，然后前端地图上的无人机根据无人机模拟器发出的坐标真实反馈到地图上
```

**AI响应：**
- 集成了Leaflet.js地图库
- 使用高德地图瓦片服务实现世界地图显示
- 支持街道视图和卫星视图切换
- 无人机位置实时更新到地图上

**关键Skill：** 无

---

**Prompt 6：**
```
告警中心页面没有数据显示，帮我修复
```

**AI响应：**
- 诊断了告警数据字段名不匹配问题
- 后端使用`alert_status`字段，前端期望`status`字段
- 修复了AlertEngine.cpp中的字段名称

**关键Skill：** 无

---

**Prompt 7：**
```
检查一下为什么前端有时会卡死，优化前端逻辑
```

**AI响应：**
- 诊断发现告警目录积累大量（数千个）告警文件
- 后端API返回全部数据导致前端处理过载
- 清理了旧告警文件
- 修改后端告警API添加分页/限制（只返回最新100条）

**关键Skill：** TRAE-debugger

---

### 2.5 无人机状态管理

**Prompt 8：**
```
无人机状态有在线、离线、巡航中、返航中，这个状态需要在无人机列表的每个无人机中展示，落盘数据的文件名称格式为ALERT-飞机ID-异常时间-异常状态.json
```

**AI响应：**
- 添加了DroneStatus枚举（ONLINE, OFFLINE, CRUISING, RETURNING）
- 实现了drone_status_to_string()状态转换函数
- 修改了状态自动切换逻辑（电量<20%时为返航中）
- 修改了告警文件命名格式：ALERT-{drone_id}-{timestamp}-{alert_type}.json

**关键Skill：** 无

---

### 2.6 数据接口优化

**Prompt 9：**
```
心跳检测不需要这么多字段，只需要保留关键字段
```

**AI响应：**
- 建议心跳检测和巡检数据分离
- 心跳检测（轻量级）：只发送关键字段（电量、位置、状态）
- 巡检数据（完整）：发送所有传感器数据，触发告警检测

**关键Skill：** 无

---

## 三、AI辅助完成模块说明

### 3.1 模块划分

| 模块 | AI辅助程度 | 主要工作内容 |
|------|-----------|-------------|
| 环境检测工具 | 80% | 编写环境检测脚本，生成Makefile |
| 无人机模拟器 | 70% | AI辅助编写Drone类、状态管理、数据生成 |
| 后端服务 | 65% | AI辅助编写HTTP接口、告警引擎、数据存储 |
| 前端页面 | 90% | AI辅助编写HTML/CSS/JS、地图集成 |

### 3.2 AI辅助详细说明

#### 3.2.1 环境检测（80% AI辅助）

**用户提供需求：**

```
帮我建一个Windows的程序，用于测试电脑是否有环境，要求有makefile用于编译
```

**AI完成工作：**
- 编写环境检测脚本（Python）
- 创建Makefile构建规则
- 配置CMakeLists.txt
- 编写依赖库下载和配置说明

**关键代码片段：**
```cpp
// 环境检测关键逻辑（Python）
def check_compiler():
    compilers = {
        'GCC': ['gcc', '--version'],
        'G++': ['g++', '--version'],
        'MSVC': ['cl', '/?'],
        'Clang': ['clang', '--version']
    }
    # 检测每个编译器...
```

---

#### 3.2.2 无人机模拟器（70% AI辅助）

**用户提供需求：**
```
帮我把无人机模拟器修改一下，要求增加一个配置文件...
```

**AI完成工作：**
- 设计配置文件JSON结构
- 实现Drone类状态管理
- 添加模拟器端HTTP客户端
- 实现动态配置加载
- 设计5种模拟模式

**关键代码片段：**
```cpp
// Drone.h - 状态枚举
enum class DroneStatus {
    ONLINE,      // 在线
    OFFLINE,     // 离线
    CRUISING,    // 巡航中
    RETURNING    // 返航中
};

// Drone.cpp - 状态转换
void Drone::update_status() {
    if (battery < 20.0) {
        drone_status = DroneStatus::RETURNING;
    } else {
        drone_status = DroneStatus::CRUISING;
    }
}
```

---

#### 3.2.3 后端服务（65% AI辅助）

**用户提供需求：**
```
后端程序通过HTTP接收无人机数据，然后根据规则判断对火情进行预警...
```

**AI完成工作：**
- 设计REST API接口
- 实现告警规则引擎
- 实现CORS跨域支持
- 实现数据持久化
- 实现路径解析支持

**关键代码片段：**
```cpp
// AlertEngine.cpp - 告警检测
void AlertEngine::check_alerts(const DroneInfo& drone) {
    // 温度异常检测
    if (drone.temperature > TEMP_THRESHOLD) {
        generate_alert(drone, "temperature", "区域温度异常");
    }
    // 烟雾浓度异常检测
    if (drone.smoke_concentration > SMOKE_THRESHOLD) {
        generate_alert(drone, "smoke", "烟雾浓度异常");
    }
    // 火情置信度检测
    if (drone.fire_confidence > FIRE_THRESHOLD) {
        generate_alert(drone, "fire_confidence", "火情置信度异常");
    }
    // 低电量检测
    if (drone.battery < BATTERY_THRESHOLD) {
        generate_alert(drone, "battery", "电量低于20%");
    }
}
```

---

#### 3.2.4 前端页面（90% AI辅助）

**用户提供需求：**
```
前端页面要求有三页：
1. 无人机列表页 - 展示所有无人机状态、电量、当前位置、最后心跳时间
2. 巡检地图页 - 展示无人机当前位置、历史轨迹、疑似火点位置
3. 告警中心页 - 展示所有火情告警
```

**AI完成工作：**
- 设计页面布局和导航
- 实现无人机列表展示
- 集成Leaflet.js地图
- 实现告警列表展示
- 添加状态标识和告警图标
- 实现数据自动刷新

**关键代码片段：**
```javascript
// app.js - 无人机列表渲染
function renderDroneList(drones) {
    const container = document.getElementById('drone-list');
    container.innerHTML = '';
    
    drones.forEach(drone => {
        const statusClass = getStatusClass(drone.status);
        const batteryClass = drone.battery < 20 ? 'battery-low' : '';
        
        const card = `
            <div class="drone-card">
                <div class="drone-header">
                    <h3>${drone.name}</h3>
                    <span class="status-badge ${statusClass}">${getStatusText(drone.status)}</span>
                </div>
                <div class="drone-info">
                    <div class="info-item">
                        <span class="label">电量:</span>
                        <div class="battery-bar ${batteryClass}">
                            <div class="battery-fill" style="width: ${drone.battery}%"></div>
                        </div>
                        <span class="value">${drone.battery.toFixed(1)}%</span>
                    </div>
                    <!-- 其他信息项 -->
                </div>
            </div>
        `;
        container.innerHTML += card;
    });
}
```

---

## 四、Bug记录与修复

### 4.1 编译错误：找不到默认构造函数

**Bug描述：**
```
error: no matching function for call to 'Drone::Drone()'
```

**AI定位过程：**
1. 分析错误信息，发现模拟器代码使用`drones.emplace(id, Drone(...))`构造无人机
2. 检查Drone类定义，发现只定义了带参构造函数
3. 确认问题原因：Drone类没有默认构造函数

**最终修复方案：**
```cpp
// 修改前（模拟器main.cpp）
drones.emplace(id, Drone(...));

// 修改后
drones.try_emplace(id, Drone(...));
```

**修复依据：** `try_emplace`会避免不必要的拷贝构造，比`emplace`更安全

---

### 4.2 中文名称显示乱码

**Bug描述：**
无人机名称在前端显示为问号"????"

**AI定位过程：**
1. 检查前端HTML编码声明：`<meta charset="UTF-8">` ✓
2. 检查后端JSON响应编码：设置`charset=utf-8` ✓
3. 检查文件读写编码：确认使用UTF-8
4. **发现问题**：错误添加了`gbk_to_utf8`转换函数，将UTF-8数据当作GBK转换

**最终修复方案：**
```cpp
// 删除错误的编码转换函数
// 原代码中有gbk_to_utf8()函数将UTF-8误判为GBK转换，导致中文丢失

// 修复：直接使用UTF-8数据，不做任何转换
// 确保JSON文件以二进制模式读写
```

---

### 4.3 告警中心无数据显示

**Bug描述：**
告警中心页面加载后显示"暂无告警"

**AI定位过程：**
1. 检查前端API调用：`GET /api/alerts` ✓
2. 检查后端API实现：存在告警读取逻辑 ✓
3. 检查数据文件：告警JSON文件存在 ✓
4. **发现问题**：后端使用`alert_status`字段，前端期望`status`字段，字段名不匹配

**最终修复方案：**
```cpp
// AlertEngine.cpp - 修改字段名
// 修改前
j["alert_status"] = status_to_string(alert.status);

// 修改后
j["status"] = status_to_string(alert.status);
```

---

### 4.4 前端页面卡死

**Bug描述：**
前端页面加载时浏览器卡死，无法响应

**AI定位过程：**
1. 使用TRAE-debugger进行性能分析
2. 检查网络请求：API返回数据量过大
3. 检查告警目录：积累数千个告警文件（>40000个）
4. **发现问题**：后端API返回全部告警数据，前端处理过载导致卡死

**最终修复方案：**
```cpp
// main.cpp - 添加告警数据限制
svr.Get("/api/alerts", [&](const Request& req, Response& res) {
    // 限制返回最新的100条告警
    const int MAX_ALERTS = 100;
    
    auto all_alerts = alert_engine.get_all_alerts();
    json result = json::array();
    
    int count = 0;
    for (auto it = all_alerts.rbegin(); 
         it != all_alerts.rend() && count < MAX_ALERTS; 
         ++it, ++count) {
        result.push_back(it->to_json());
    }
    
    res.set_content(result.dump(), "application/json");
});
```

**额外清理：**
- 清理了旧告警文件，只保留最新告警
- 修改告警文件命名格式，避免文件数量过多

---

### 4.5 删除无人机失败

**Bug描述：**
前端页面删除无人机按钮无响应

**AI定位过程：**
1. 检查前端DELETE请求代码：实现正常 ✓
2. 检查后端DELETE接口：存在实现 ✓
3. 检查网络请求：浏览器控制台显示CORS错误
4. **发现问题**：后端CORS配置缺少对OPTIONS预检请求的处理

**最终修复方案：**
```cpp
// main.cpp - 添加OPTIONS请求处理
svr.Options("/api/.*", [](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = 200;
});
```

---

### 4.6 心跳超时检测不准确

**Bug描述：**
无人机离线后状态仍显示为"在线"

**AI定位过程：**
1. 检查前端心跳超时判断逻辑
2. **发现问题**：前端只检查心跳时间间隔，未考虑服务重启后时间重置问题

**最终修复方案：**
```cpp
// app.js - 修改心跳超时判断
function updateDroneStatus(drone) {
    const lastHeartbeat = new Date(drone.last_heartbeat);
    const now = new Date();
    const diffSeconds = (now - lastHeartbeat) / 1000;
    
    // 超过5分钟（300秒）判定为离线
    if (diffSeconds > 300) {
        drone.status = 'offline';
    }
}
```

---

## 五、经验教训

### 5.1 AI辅助开发优势

| 优势 | 说明 |
|------|------|
| 快速原型 | 可以快速生成代码框架，节省开发时间 |
| 知识补充 | AI可以提供跨领域的技术建议 |
| 问题诊断 | AI可以帮助定位难以发现的Bug |
| 代码优化 | AI可以提供代码重构和优化建议 |
| 文档生成 | AI可以自动生成技术文档 |

### 5.2 AI辅助开发局限

| 局限 | 说明 |
|------|------|
| 上下文理解 | 复杂的业务逻辑需要人工澄清 |
| 代码质量 | AI生成的代码需要人工审核 |
| 调试能力 | 运行时问题需要人工介入 |
| 经验判断 | AI缺乏实际项目经验 |

### 5.3 改进建议

1. **建立Prompt库**：积累常用Prompt，提高复用效率
2. **代码审查流程**：AI生成的代码必须经过人工审核
3. **自动化测试**：为关键功能编写单元测试，确保代码正确性
4. **文档同步**：代码变更时同步更新文档
5. **知识沉淀**：定期总结AI辅助开发的经验教训

---

**报告编写人：** 田雪飞  
**日期：** 2026-06-29  
**版本：** v1.0  
