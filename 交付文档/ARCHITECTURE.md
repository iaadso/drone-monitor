### 一、框架拓补图（图片在当前文件夹下）

![拓补图](C:\Users\10426\Desktop\myworkspase\test\drone-monitor\交付文档\拓补图.png)

### 二、流程说明

1. 无人机模拟端 → 平台后端

   ```
   1. 注册无人机：POST /api/drones/{drone_id}
      数据：
      {
       "drone_id": "DRONE-001",
       "name": "北京巡检无人机",
       "latitude": 39.9042,
       "longitude": 116.4074,
       "altitude": 100.0,
       "battery": 85.5,
       "temperature": 25.3,
       "smoke_concentration": 8.2,
       "fire_confidence": 5.0,
       "timestamp": "2026-06-29 15:30:45"
      }
   
   2. 心跳上报：POST /api/drones/{drone_id}
      数据：
      {
       "drone_id": "DRONE-001",
       "battery": 82.3,
       "latitude": 39.9052,
       "longitude": 116.4084,
       "altitude": 95.6,
       "status": "cruising",
       "timestamp": "2026-06-29 15:31:50"
      }
   
   3. 巡检数据：POST /api/drones/{drone_id}
      数据：
      {
       "drone_id": "DRONE-001",
       "name": "北京巡检无人机",
       "status": "cruising",
       "latitude": 39.9062,
       "longitude": 116.4094,
       "altitude": 90.3,
       "battery": 81.5,
       "temperature": 24.8,
       "smoke_concentration": 9.1,
       "fire_confidence": 6.2,
       "timestamp": "2026-06-29 15:32:55"
      }
   ```

   

2. 平台后端 → 前端页面

   ```
   1. 获取无人机列表：GET /api/drones
      响应：[{drone_id, name, status, battery, sensors, last_heartbeat...}]
   
   2. 获取告警列表：GET /api/alerts (限制100条)
      响应：[{alert_id, alert_type, drone_id, reason, level, status...}]
   
   3. 更新告警状态：PUT /api/alerts/{alert_id}
      数据：{status: 1/2} (已确认/已忽略)
   
   4. 删除无人机：DELETE /api/drones/{drone_id}
   ```

   

3. 告警生成流程

   ```
   平台后端 AlertEngine → 检测规则 → 生成告警 → 存储文件
   
   规则：
     - temperature > 60°C → temperature异常
     - smoke_concentration > 50 → smoke异常
     - fire_confidence > 80% → fire_confidence异常
     - battery < 20% → battery异常
     - 连续3次异常 → continuous异常
   ```

   

4. 状态判定

   ```
   无人机状态：
     - cruising (巡航中)：电量 ≥ 20%
     - returning (返航中)：电量 < 20%
     - offline (离线)：心跳超时 > 5分钟
     - online (在线)：初始状态
   ```

   

### 三、提供接口

1. 无人机接口接入

   | 接口                 | 方法 | 功能               |
   | -------------------- | ---- | ------------------ |
   | /api/drones/register | POST | 注册新无人机       |
   | /api/drones/:id      | POST | 上报无人机数据     |
   | /api/drones          | GET  | 获取所有无人机列表 |
   | /api/drones/:id      | GET  | 获取单个无人机详情 |

   

2. 地图修改

   ```
   手动修改代码：修改map.html中的地图URL
   ```

   