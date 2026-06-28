### 一、环境要求

| 工具    | 版本   | 用途          |
| ------- | ------ | ------------- |
| Windows | win10  |               |
| GCC/G++ | 16.1.0 | 编译 C++ 代码 |
| CMake   | 4.3.4  | 生成构建文件  |
| Make    | 4.4.1  | 生成构建文件  |

### 二、访问地址说明

1. 前端访问地址

   | 页面         | URL                               | 功能                                       |
   | ------------ | --------------------------------- | ------------------------------------------ |
   | 无人机列表页 | http://localhost:8081/index.html  | 展示所有无人机状态、电量、位置、传感器数据 |
   | 巡检地图页   | http://localhost:8081/map.html    | 展示无人机地图、历史轨迹、疑似火点         |
   | 告警中心页   | http://localhost:8081/alerts.html | 展示告警列表、支持确认/忽略操作            |

   

2. 后端接口地址

   | 接口           | 方法   | URL                        | 功能               |
   | -------------- | ------ | -------------------------- | ------------------ |
   | 获取无人机列表 | GET    | /api/drones                | 返回所有无人机信息 |
   | 获取单个无人机 | GET    | /api/drones/{id}           | 返回指定无人机详情 |
   | 无人机注册     | POST   | /api/drones/register       | 无人机初始注册     |
   | 心跳上报       | POST   | /api/drones/{id}/heartbeat | 无人机心跳检测     |
   | 巡检数据上报   | POST   | /api/drones/{id}/patrol    | 上报传感器数据     |
   | 删除无人机     | DELETE | /api/drones/{id}           | 删除指定无人机     |
   | 获取告警列表   | GET    | /api/alerts                | 返回所有告警记录   |
   | 确认告警       | PUT    | /api/alerts/{id}/confirm   | 确认告警           |
   | 忽略告警       | PUT    | /api/alerts/{id}/ignore    | 忽略告警           |

   

3. 模拟无人机验证接口

   | 功能         | 接口                            | 请求示例                                                     |
   | ------------ | ------------------------------- | ------------------------------------------------------------ |
   | 注册无人机   | POST /api/drones/register       | {"drone_id":"DRONE-001","name":"巡检无人机A"}                |
   | 心跳检测     | POST /api/drones/{id}/heartbeat | {"status":"normal"}                                          |
   | 上报巡检数据 | POST /api/drones/{id}/patrol    | {<br/>    "drone_id": "DRONE-001",<br/>    "name": "巡检无人机A",<br/>    "latitude": 39.9045,<br/>    "longitude": 116.4075,<br/>    "altitude": 99.5,<br/>    "battery": 15.2,<br/>    "temperature": 25.0,<br/>    "smoke_concentration": 10.0,<br/>    "fire_confidence": 0.0,<br/>    "status": "low_battery",<br/>    "trajectory": [[39.9042, 116.4074], [39.9043, 116.4075]]<br/>} |

   

   ```
   # 获取无人机列表
   Invoke-WebRequest -Uri "http://localhost:8080/api/drones" -UseBasicParsing
   
   # 获取单个无人机
   Invoke-WebRequest -Uri "http://localhost:8080/api/drones/DRONE-001" -UseBasicParsing
   
   # 获取告警列表
   Invoke-WebRequest -Uri "http://localhost:8080/api/alerts" -UseBasicParsing
   
   # 手动注册无人机
   $body = '{"drone_id":"DRONE-TEST","name":"测试无人机"}'
   Invoke-WebRequest -Uri "http://localhost:8080/api/drones/register" -Method POST -Body $body -ContentType "application/json"
   ```

   

### 三、启动

1. 一键启动

   ```
   cd drone-monitor
   powershell -ExecutionPolicy Bypass -File .\start-all.ps1
   ```

2. 分别启动

   ```
   方法一：
   # 终端1：启动后端服务
   cd drone-monitor
   powershell -ExecutionPolicy Bypass -File .\start-backend.ps1
   
   # 终端2：启动模拟器（需先启动后端）
   cd drone-monitor
   powershell -ExecutionPolicy Bypass -File .\start-simulator.ps1
   
   # 终端3：启动前端页面
   cd drone-monitor
   powershell -ExecutionPolicy Bypass -File .\start-frontend.ps1
   ```

   ```
   方法二：
   # 终端1：启动后端服务
   cd drone-monitor\build
   .\backend\drone-backend.exe
   
   # 终端2：启动模拟器（需先启动后端）
   cd drone-monitor\build
   .\simulator\drone-simulator.exe
   
   # 终端3：启动前端页面
   cd drone-monitor\frontend
   python -m http.server 8081
   ```

   