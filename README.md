# MBRtool

MBRtool 是一款功能强大的磁盘主引导记录（MBR）管理与分析工具。它结合了直观的 QML 用户界面与高效的 C++ 后端逻辑，并集成了内核级驱动，旨在为开发者和系统研究人员提供对物理磁盘的底层访问能力。

---

## 🛠 主要功能

* **MBR 备份与恢复**：快速保存 MBR 镜像，并在需要时进行安全回写。
* **高级 Hex 编辑器**：支持大文件读取、字节级编辑，并能直接定位到指定的物理磁盘扇区。
* **指令反汇编**：集成反汇编引擎，实时解析引导代码逻辑。
* **驱动级物理写入 (v2.0 新增)**：通过内置驱动环境，绕过操作系统对物理磁盘 0 扇区的写入限制。
* **磁盘环境自动配置**：支持驱动程序的自动下载、解压及环境初始化。

---

## 🚀 更新日志 (v1.0.0 -> v2.0.2026.4.18)

### 🌟 核心升级
* **引入 `ntWiter` 驱动引擎**：新增了对 Windows 内核驱动的调用能力，实现了真正的物理磁盘写入功能。
* **驱动环境自动化**：支持自动准备驱动运行环境，包括驱动文件的下载与 ZIP 格式解压。
* **磁盘目标锁定**：`HexEditor` 模块新增 `setTargetDisk` 方法，支持用户指定特定的物理磁盘号进行操作。

### 🔧 架构优化
* **QML 交互增强**：显式注册 `DisassemblyWindows` 类，并在 `QQmlApplicationEngine` 中暴露了更多后端实例（如 `ntWriterBackend`）。
* **数据结构定义**：新增 `Public.h`，定义了标准的 `IOCTL_DISK_WRITE_COMMAND` 及内核通信结构体 `DISK_WRITE_PARAMS`。
* **内存管理**：优化了 `WindowManager` 对 QML 引擎实例的生命周期管理。

---

## 🏗 技术架构

### 1. 前端层 (UI/UX)
* **框架**：Qt Quick / QML
* **风格**：QQuickStyle "Fusion" 样式，保证跨设备的一致性体验。
* **组件化**：将 Hex 编辑、反汇编窗口等功能模块化，提升了代码的复用性。

### 2. 逻辑处理层 (C++ Backend)
* **核心模块**：
    * `FileReader`: 处理文件 IO 操作。
    * `HexEditor`: 管理二进制缓冲数据。
    * `DisassemblyWindows`: 处理机器码的反汇编逻辑。
* **数据绑定**：通过 `setContextProperty` 将后端 C++ 对象直接映射到 QML 全局命名空间。

### 3. 系统驱动层 (Kernel Interaction)
* **通信协议**：基于 Windows 设备控制 API (`DeviceIoControl`)，配合自定义 `IOCTL` 控制码。
* **驱动模型**：采用 Windows 内核模式驱动（KMDF），通过 `ntWiter` 类管理驱动的启动、停止与数据传输。

---

## 📦 编译与运行

1.  **环境要求**：
    * Qt 6.x (带有 QML/Quick 模块)
    * MSVC 编译器 (用于 Windows 平台编译)
    * CMake 3.16+
2.  **构建流程**：
    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
    ```
3.  **权限说明**：由于涉及驱动加载与物理磁盘写入，程序运行需要**管理员权限**。

---

## 📄 开源协议

本项目遵循仓库内的 LICENSE 文件说明。

---
*Created by t-r-r114*
