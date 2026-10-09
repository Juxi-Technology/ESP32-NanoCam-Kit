# 第22章: ROS 导航与 SLAM

> 难度: ★★★★ | 产出: 机器人自主导航

## 原理

通过激光雷达或深度相机采集环境数据，使用 Cartographer / Gmapping 算法构建 2D 栅格地图，再利用 DWB 路径规划器实现自主导航。

## 脚本清单 (17个)

| 脚本 | 功能 |
|------|------|
| `cartographer_launch.py` | Cartographer 2D SLAM 建图 |
| `map_cartographer_launch.py` | Cartographer 地图模式启动 |
| `map_gmapping_launch.py` | Gmapping 粒子滤波建图 |
| `navigation_dwb_launch.py` | DWB 局部路径规划 |
| `display_launch.py` | 导航可视化 |
| `occupancy_grid_launch.py` | 占据栅格地图 |
| `multi_nav_display_launch.py` | 多机器人导航显示 |
| `save_map_launch.py` | 保存地图到文件 |
| `app_send_goal.py` | 发送导航目标点 |
| `robot_pose_publisher_launch.py` | 机器人位姿发布 |
| `laserscan_to_point_publish.py` | 激光雷达→点云转换 |

```bash
cd tools/ros_packages/
python3 cartographer_launch.py
python3 navigation_dwb_launch.py
python3 app_send_goal.py
```

代码位置: `tools/ros_packages/`

## 下一步
→ [第23章: ROS 相机标定与巡线](Ch23_ROS相机标定巡线.md)
