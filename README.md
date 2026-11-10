# YOLO11 目标检测系统（训练 + Qt 桌面端联调）

基于 Ultralytics **YOLO11n** 的自定义目标检测课程项目，实现完整闭环：

> 视频抽帧构建数据集 → YOLO11n 训练 → Qt 桌面客户端传图 → Python 后端检测 → 检测结果（类别 / 置信度 / 框坐标）以 JSON 回传 Qt 显示

## 系统架构

```
┌──────────────────┐    TCP/UDP     ┌─────────────────────┐
│   Qt 桌面客户端   │ ─────────────▶ │  Python 检测服务     │
│   (C++ / Qt5)    │ ◀───────────── │  test.py + YOLO11n  │
└──────────────────┘   JSON 结果    └─────────────────────┘
```

- **通信协议**：TCP 传输图片（4 字节小端序声明文件大小，规避粘包）；检测框逐个回传 JSON
- **检测结果字段**：`cls_idx`（类别索引）、`cls_name`（类别名）、`conf`（置信度）、`lx/ly/rx/ry`（检测框左上/右下坐标）

## 目录结构

```
├── train1.py          # 训练脚本：加载 yolo11n.pt 预训练权重，50 epochs
├── test.py            # 检测服务：接收 Qt 传图 → YOLO 推理 → JSON 回传
├── bvn.yaml           # 数据集配置：两类自定义目标（mingren / daitu）
├── qt_app/            # Qt 客户端源码（.cpp/.h/.ui/.qrc/.pro）
└── runs/              # 训练过程产物（损失曲线、指标曲线等）
```

## 快速开始

```bash
# 1. 安装依赖
pip install ultralytics

# 2. 训练（需先按 bvn.yaml 的目录结构放置数据集）
python train1.py

# 3. 启动检测服务（等待 Qt 客户端连接，IP/端口在 test.py 底部修改）
python test.py

# 4. 用 Qt Creator 打开 qt_app/untitled.pro 编译运行客户端
```

> 训练完成后最优权重保存在 `runs/detect/train/weights/best.pt`（因体积原因未随仓库上传）。

## 关键实现点

- **视频抽帧**：OpenCV 按固定间隔从视频抽取帧图片，快速构建数据集（见 `train1.py` 注释部分）
- **迁移训练**：加载官方 `yolo11n.pt` 预训练权重，在自定义两类数据上微调 50 epochs
- **TCP 粘包处理**：先接收 4 字节声明图片大小，再按大小循环接收完整图片
- **多目标回传**：逐框序列化 JSON 并间隔发送，避免流式套接字粘包

## 环境

- Python 3.x + ultralytics (YOLO11) + OpenCV
- Qt 5 / Qt Creator（客户端）

## License

训练与检测代码为课程学习用途编写；Ultralytics 框架遵循 AGPL-3.0。
