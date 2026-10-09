# 第21章: ROS2 + MediaPipe 高级CV

> 难度: ★★★★ | 产出: 手部/姿态/面部/手势识别

## NanoCam 原生 ROS2 节点 (6个)

| 节点 | 功能 | 发布 Topic |
|------|------|-----------|
| `nanocam_img_viewer.py` | 实时画面 | — |
| `nanocam_hand_detector.py` | 手部21点追踪 | `/nanocam_gesture` |
| `nanocam_pose_detector.py` | 人体33点骨骼 | `/nanocam_pose` |
| `nanocam_face_mesh.py` | 面部468点 | — |
| `nanocam_objectron.py` | 3D物体检测 | — |
| `nanocam_gesture_ctrl.py` | 手势→机器人控制 | `/cmd_vel` |

## ROS2 完整 Demo 集 (11个)

| # | Demo | 功能 | 命令 |
|:--:|------|------|------|
| 1 | HandDetector | 手部21个关键点 | `python3 01_HandDetector.py` |
| 2 | PoseDetector | 人体33个关键点 | `python3 02_PoseDetector.py` |
| 3 | Holistic | 全身+手部+面部 | `python3 03_Holistic.py` |
| 4 | FaceMesh | 468个面部关键点 | `python3 04_FaceMesh.py` |
| 5 | FaceEyeDetection | 面部+眼部检测 | `python3 05_FaceEyeDetection.py` |
| 6 | FaceLandmarks | 面部特征点 | `python3 06_FaceLandmarks.py` |
| 7 | FaceDetection | 面部检测 | `python3 07_FaceDetection.py` |
| 8 | Objectron | 3D物体检测 | `python3 08_Objectron.py` |
| 9 | VirtualPaint | 虚拟画笔 | `python3 09_VirtualPaint.py` |
| 10 | HandCtrl | 手指控制 | `python3 10_HandCtrl.py` |
| 11 | GestureRecognition | 手势识别 | `python3 11_GestureRecognition.py` |

代码位置: `tools/ros_nanocam/` + `tools/ros_mediapipe/`

```bash
# NanoCam 原生 ROS2 节点 (tools/ros_nanocam/)
python3 nanocam_hand_detector.py     # 01 手部21点 → /nanocam_gesture
python3 nanocam_pose_detector.py     # 02 人体33点 → /nanocam_pose
python3 nanocam_face_mesh.py         # 04 面部468点
python3 nanocam_objectron.py         # 08 3D物体检测
python3 nanocam_gesture_ctrl.py      # 手势 → /cmd_vel 机器人控制

# 完整 11 个 MediaPipe Demo (tools/ros_mediapipe/)
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

## 下一步
→ [第22章: ROS 导航 SLAM](Ch22_ROS导航SLAM.md)
