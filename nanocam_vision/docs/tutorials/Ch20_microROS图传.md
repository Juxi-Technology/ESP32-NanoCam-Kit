# 第20章: micro-ROS 图传

> 难度: ★★★ | 产出: RViz2 实时画面

## 原理

NanoCam 通过 micro-ROS client 将摄像头画面发布到 ROS2 topic `/esp32_img`,AI检测结果发布到 `/esp32_detect`。

## 步骤

### 17.1 Docker 启动 agent
```bash
docker run -it --rm --net=host   microros/micro-ros-agent:humble udp4 --port 9999 -v4
```

### 17.2 验证连接
```bash
ros2 topic list
# /esp32_img
# /esp32_detect
```

### 17.3 RViz2 显示
1. `rviz2`
2. Add → Image → Topic: `/esp32_img`
3. Fixed Frame: `esp32_cam_frame`

## 下一步
→ [第21章: MediaPipe高级CV](Ch21_MediaPipe高级CV.md)
