# 第三方组件与许可证说明

本仓库在自有代码之外，包含或依赖以下第三方开源组件。各组件的版权归其原作者所有，遵循其各自的开源许可证。分发本项目时请一并保留相应声明。

## 随仓库分发的组件（vendored）

| 组件 | 版权方 | 许可证 | 位置 |
|------|--------|--------|------|
| esp-dl | Espressif Systems | Apache-2.0 | `firmware/nanocam_vision/components/esp-dl/`（含 LICENSE） |
| micro_ros_espidf_component | micro-ROS 项目（eProsima 等） | Apache-2.0 | `firmware/nanocam_vision/components/micro_ros_espidf_component/`（含预编译 `libmicroros.a`） |
| esp-code-scanner | Espressif Systems | 见组件注册中心页面（以预编译库形式分发） | `firmware/nanocam_vision/components/esp-code-scanner/`（https://components.espressif.com/components/espressif/esp_code_scanner ） |
| libnewlib_iconv.a | Espressif GCC 工具链（newlib） | GPL-3.0 及 GCC Runtime Library Exception | `firmware/nanocam_vision/components/esp-code-scanner/lib/` |

## 通过 IDF Component Manager 拉取的组件（不随仓库分发）

| 组件 | 版权方 | 许可证 |
|------|--------|--------|
| esp32-camera | Espressif Systems | Apache-2.0 |

## 派生项目

| 项目 | 版权方 | 许可证 | 位置 |
|------|--------|--------|------|
| xiaozhi-esp32（小智 AI） | Xiaoxia（虾哥） | MIT | `firmware/nanocam_xiaozhi/`、`firmware/nanocam_espclaw/`（各自保留上游 LICENSE） |

> 以上信息供参考，具体以各组件目录内的 LICENSE / 版权声明文件为准。
