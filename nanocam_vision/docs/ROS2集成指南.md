# ESP32-NanoCam ROS2 集成指南

> 将 NanoCam 摄像头画面和 AI 检测结果发布到 ROS2 话题，直连 RViz2 可视化。

---

## 架构

```
NanoCam (ESP32-S3)                   PC (Linux)
┌──────────────────┐                 ┌──────────────────────┐
│  nano_ros.cpp    │   UDP:9999      │  micro-ros-agent     │
│                  │  ───────────→   │  (Docker)            │
│  /esp32_img      │                 │         ↓            │
│  /esp32_detect   │                 │    ROS2 Topics       │
└──────────────────┘                 │         ↓            │
                                     │  RViz2 / 自定义节点   │
                                     └──────────────────────┘
```

---

## 一、NanoCam 端配置

### 1. 启用 micro-ROS

`menuconfig` → `NanoCam Configuration` → 勾选 `Enable micro-ROS`

设置 Agent IP（你的 Linux PC 的 IP）:

```
CONFIG_NANOCAM_ROS_AGENT_IP="192.168.1.100"
```

### 2. 编译固件

```bash
idf.py build flash monitor
```

NanoCam 启动后会自动连接 micro-ros-agent。

---

## 二、PC 端启动

### 1. 启动 micro-ros-agent

```bash
docker run -it --rm --net=host \
  microros/micro-ros-agent:humble udp4 --port 9999 -v4
```

### 2. 验证连接

```bash
ros2 topic list
# 应该看到:
# /esp32_img
# /esp32_detect
# /parameter_events
```

### 3. 查看话题

```bash
ros2 topic echo /esp32_detect     # AI 检测结果
ros2 topic hz /esp32_img          # 图像帧率
ros2 topic info /esp32_img        # 消息类型
```

---

## 三、RViz2 可视化

1. 启动 RViz2: `rviz2`
2. 添加 `Image` 显示 → topic 选 `/esp32_img`
3. Fixed Frame 设为 `esp32_cam_frame`
4. 即可看到实时画面

---

## 四、MediaPipe 高级 CV

11 个 MediaPipe Demo 已移植到项目中:

```bash
# NanoCam 原生 ROS2 节点 (tools/ros_nanocam/)
python3 nanocam_hand_detector.py     # 手部21点 → /nanocam_gesture
python3 nanocam_pose_detector.py     # 人体33点 → /nanocam_pose
python3 nanocam_face_mesh.py         # 面部468点
python3 nanocam_objectron.py         # 3D物体检测
python3 nanocam_gesture_ctrl.py      # 手势→机器人控制

# 全部 11 个 Demo (tools/ros_mediapipe/)
python3 01_HandDetector.py           # 手部检测
python3 02_PoseDetector.py           # 姿态检测
python3 03_Holistic.py               # 全身检测
python3 04_FaceMesh.py               # 面部网格
python3 05_FaceEyeDetection.py       # 面部+眼部
python3 06_FaceLandmarks.py          # 面部特征点
python3 07_FaceDetection.py          # 面部检测
python3 08_Objectron.py              # 3D物体
python3 09_VirtualPaint.py           # 虚拟画笔
python3 10_HandCtrl.py               # 手指控制
python3 11_GestureRecognition.py     # 手势识别
```

---

## 五、故障排除

| 问题 | 解决 |
|------|------|
| topic 列表为空 | 检查 NanoCam 和 PC 是否同一网段 |
| 图像不显示 | RViz2 的 Fixed Frame 设为 `esp32_cam_frame` |
| Agent 连接失败 | Docker 是否用了 `--net=host` |
| 端口冲突 | 9999 端口不能有其他程序占用 |

---

## 六、自定义 ROS2 节点

```python
import rclpy
from sensor_msgs.msg import Image
from std_msgs.msg import String

class NanoCamSubscriber(rclpy.node.Node):
    def __init__(self):
        super().__init__('nanocam_sub')
        self.img_sub = self.create_subscription(Image, '/esp32_img', self.on_image, 10)
        self.det_sub = self.create_subscription(String, '/esp32_detect', self.on_detect, 10)

    def on_image(self, msg):
        self.get_logger().info(f'Frame: {msg.width}x{msg.height}')

    def on_detect(self, msg):
        self.get_logger().info(f'Detect: {msg.data}')
```
