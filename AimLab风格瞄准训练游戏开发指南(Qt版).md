# AimLab 风格瞄准训练游戏 — 开发实践指南

> 基于 C++ / Qt，从零构建 3D 射击瞄准训练游戏

---

## 目录

- [一、项目概述](#一项目概述)
- [二、技术栈总览](#二技术栈总览)
- [三、阶段一：开发环境搭建](#三阶段一开发环境搭建)
- [四、阶段二：图形与渲染](#四阶段二图形与渲染)
- [五、阶段三：输入与交互](#五阶段三输入与交互)
- [六、阶段四：目标系统](#六阶段四目标系统)
- [七、阶段五：完善与优化](#七阶段五完善与优化)
- [八、总体路线图](#八总体路线图)
- [九、关键提醒](#九关键提醒)

---

## 一、项目概述

本项目目标是使用 C++ 从零构建一款类似 **AimLab** 的 3D 射击瞄准训练游戏。核心玩法为：

- 在 3D 立体空间中生成移动目标球
- 玩家通过鼠标控制视角瞄准目标
- 系统检测命中并实时反馈得分与精度数据

本项目可作为 C++ Qt 图形化学习项目，代码量可控（500~1500 行可完成原型），同时覆盖了图形学、输入处理、物理检测等核心技术。

---

## 二、技术栈总览

### 核心依赖

| 库 / 技术 | 版本 | 用途 |
|-----------|------|------|
| C++ | C++17+ | 编程语言 |
| Qt Creator | 6.0+ | IDE 与 UI 设计器 |
| Qt | 6.5+ | 窗口、输入、音频、图形 |
| Qt3D | 6.5+ | 3D 场景渲染（基于 OpenGL） |

### 可选依赖

| 库 / 技术 | 用途 |
|-----------|------|
| Qt Multimedia | 音频播放（背景音乐、音效） |
| nlohmann/json | JSON 数据存储（存档系统） |

### 项目目录结构

```
AimTrainer/
├── CMakeLists.txt              ← CMake 配置文件
├── AimTrainer.pro              ← Qt 项目文件
├── src/
│   ├── main.cpp                ← 程序入口
│   ├── mainwindow.cpp / h      ← 主窗口
│   ├── gamewidget.cpp / h      ← 游戏Widget
│   ├── gameengine.cpp / h      ← 游戏逻辑主循环
│   ├── cameracontroller.cpp / h ← 摄像机控制
│   ├── target.cpp / h          ← 目标系统
│   ├── targetspawner.cpp / h   ← 目标生成器
│   ├── scoremanager.cpp / h    ← 计分系统
│   ├── audiohandler.cpp / h    ← 音频处理
│   └── aimanalyzer.cpp / h     ← 瞄准分析
├── qml/                         ← QML 界面（可选）
│   └── main.qml
└── resources/                  ← 资源文件
     ├── shaders/
     └── sounds/
```

---

## 三、阶段一：开发环境搭建

> 目标：配置好开发工具，创建项目骨架，验证 Qt 窗口能正常运行。

### 3.1 安装 Qt Creator

1. 下载地址：[https://www.qt.io/download-qt-installer](https://www.qt.io/download-qt-installer)
2. 注册 Qt 账号（免费）
3. 安装时勾选：
   - **Qt 6.5+**（或最新版）
   - **Qt Creator**（IDE）
   - **Qt 3D** 模块（3D 渲染）
   - **Qt Multimedia**（音频）
   - **MinGW 编译器**（Windows）
4. 验证：新建 Qt Widgets Application，编译运行一个空白窗口

### 3.2 创建第一个 Qt 项目

使用 Qt Creator 新建项目：

```
文件 → 新建文件或项目 → Application → Qt Widgets Application
→ 设置项目名称 AimTrainer → 选择 Kit（MinGW）→ 完成
```

### 3.3 配置 Qt3D 模块

在 `.pro` 文件中添加：

```pro
QT += core gui opengl widget3d multimedia

# 如果使用 Qt3D Core
QT += 3dcore 3drender 3dinput 3dlogic

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets
```

### 3.4 第一个可运行程序

修改 `mainwindow.cpp`，创建一个带有 OpenGL 背景的窗口：

```cpp
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Aim Trainer");
        resize(1280, 720);
    }
};
```

### 里程碑一

> Qt 项目能正常编译运行，显示一个窗口，窗口可正常关闭。

---

## 四、阶段二：图形与渲染

> 目标：在 3D 空间中渲染场景，掌握摄像机与矩阵变换，绘制准星。

### 4.1 Qt3D 渲染框架

Qt3D 是 Qt 自带的 3D 渲染模块，封装了 OpenGL，使用更简单：

```
Qt3D 核心组件：
├── Qt3DCore::QEntity         → 3D 场景中的实体
├── Qt3DRender::QCamera       → 摄像机
├── Qt3DRender::QMaterial      → 材质
├── Qt3DRender::QMesh          → 网格（模型）
└── Qt3DLogic::QFrameAction   → 每帧更新逻辑
```

### 4.2 创建 3D 场景

```cpp
// 创建场景根节点
Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

// 创建摄像机
Qt3DRender::QCamera *camera = new Qt3DRender::QCamera(rootEntity);
camera->setPosition(QVector3D(0, 2, 10));
camera->setViewCenter(QVector3D(0, 0, 0));

// 创建渲染设置
Qt3DRender::QRenderSettings *renderSettings = new Qt3DRender::QRenderSettings(rootEntity);
```

### 4.3 摄像机与矩阵变换

Qt3D 提供了更简便的摄像机控制：

```cpp
// 透视投影
camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);

// 第一人称视角控制
Qt3DCore::QEntity *cameraRig = new Qt3DCore::QEntity(rootEntity);
camera->setParent(cameraRig);

// 自定义摄像机控制器（继承 Qt3DInput::QAbstractController）
```

摄像机设计要点：
- 玩家视角，固定在原点
- 向上下旋转（Pitch，限制 ±89°）
- 向左右旋转（Yaw）
- 视野角度 FOV 建议设置在 80°~100°

### 4.4 渲染 3D 场景

- **地面网格**：使用 `Qt3DRender::QCylinderMesh` 或自定义网格
- **背景**：设置清屏颜色为深色
- **光照**：`Qt3DRender::QDirectionalLight` 或 `QPointLight`

### 4.5 渲染准星（Crosshair）

准星是 FPS 游戏的必备元素：

- 在 2D 屏幕空间绘制，覆盖在 3D 场景之上
- 固定在屏幕正中央
- 样式：简单的十字线条，使用 Qt 的 `QPainter` 绘制
- 颜色：白色，可添加阴影提高对比度

```cpp
// 在 QWidget 上绘制准星
void GameWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(QPen(Qt::white, 2));

    int cx = width() / 2;
    int cy = height() / 2;

    // 十字准星
    painter.drawLine(cx - 15, cy, cx + 15, cy);
    painter.drawLine(cx, cy - 15, cx, cy + 15);
}
```

### 4.6 摄像机控制（键盘）

在验证渲染阶段，先用键盘控制摄像机朝向：

- W/S/A/D 键控制前后左右移动
- 上下左右方向键控制视角旋转

### 里程碑二

> 进入一个 3D 空间场景，地面有网格感，准星固定在屏幕中央，可用键盘或鼠标控制视角旋转。

---

## 五、阶段三：输入与交互

> 目标：实现鼠标锁定、增量追踪、射线投射命中检测和命中反馈。这是 AimLab 体验的核心。

### 5.1 鼠标捕获（Mouse Grabbing）

Qt 中实现鼠标捕获：

```cpp
// 进入全屏并捕获鼠标
setWindowState(Qt::WindowFullScreen);
setMouseGrabPolicy(Qt::.grab);

void GameWidget::mouseMoveEvent(QMouseEvent *event) {
    QPoint delta = event->pos() - m_lastMousePos;
    // 处理鼠标移动
    m_lastMousePos = event->pos();
}
```

### 5.2 鼠标增量追踪（Delta Mouse）

这是 AimLab 核心技术之一：

```cpp
void GameWidget::mouseMoveEvent(QMouseEvent *event) {
    // 获取鼠标相对移动量
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    // 累积到玩家的"瞄准轨迹"中
    m_totalMouseX += dx;
    m_totalMouseY += dy;

    lastPos = event->pos();
}
```

**用途**：
- 控制摄像机旋转
- 记录玩家的鼠标路径
- 分析 Flick（闪瞄）距离、Tracking 精度等

### 5.3 视角旋转（鼠标控制）

```cpp
void GameWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x();
    int dy = event->y();

    // 计算旋转角度
    float yaw = dx * sensitivity;
    float pitch = dy * sensitivity;

    // 应用旋转（限制 pitch）
    m_yaw += yaw;
    m_pitch = qBound(-89.0f, m_pitch + pitch, 89.0f);

    // 更新摄像机朝向
    updateCameraDirection();
}
```

### 5.4 射线投射命中检测（Raycasting）

从摄像机发射射线检测目标命中：

```cpp
// 从鼠标位置发射射线
Qt3DRender::QRay3D createRayFromMouse(QPoint mousePos) {
    QVector3D rayOrigin = camera->position();
    QVector3D rayDirection = camera->viewVector(); // 可根据鼠标位置调整
    return Qt3DRender::QRay3D(rayOrigin, rayDirection);
}

// 或者使用 Qt3DRender::QScreenRayCaster
```

**简化实现**：
1. 获取摄像机位置和朝向
2. 射线方程：`ray(t) = cameraPos + t * cameraDir`
3. 遍历所有目标球，计算球体-射线交点
4. 如果存在交点且 `t > 0`，则命中

### 5.5 命中反馈

命中目标后即时反馈：

- **视觉反馈**：目标颜色变化、缩小消失、粒子效果
- **听觉反馈**：播放命中音效
- **数值反馈**：显示命中得分、反应时间

### 5.6 鼠标点击检测

```cpp
void GameWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 执行射线投射检测
        Target *hit = performRaycast();
        if (hit != nullptr) {
            onTargetHit(hit);
        }
    }
}
```

### 里程碑三

> 鼠标捕获后移动鼠标可以自由旋转视角，瞄准目标球并点击鼠标左键，能检测到命中并给出视觉和听觉反馈。

---

## 六、阶段四：目标系统

> 目标的行为直接决定训练效果。这是 AimLab 的灵魂。

### 6.1 目标球渲染

使用 Qt3D 渲染目标球：

```cpp
// 创建目标实体
Qt3DCore::QEntity *targetEntity = new Qt3DCore::QEntity(rootEntity);

// 球体网格
Qt3DRender::QMesh *sphereMesh = new Qt3DRender::QMesh();
sphereMesh->setSource(QUrl("qrc:/meshes/sphere.obj"));

// 材质
Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial();
targetEntity->addComponent(sphereMesh);
targetEntity->addComponent(material);
```

目标结构体设计：

```cpp
struct Target {
    int id;
    QVector3D position;      // 目标位置
    QVector3D velocity;       // 移动速度
    float radius;             // 目标半径
    QColor color;             // 目标颜色
    TargetState state;        // 状态：存活、已命中、消失
    Qt3DCore::QEntity *entity; // Qt3D 实体
};
```

### 6.2 目标生成器

```cpp
class TargetSpawner : public QObject {
    Q_OBJECT
public:
    TargetSpawner(Qt3DCore::QEntity *parent);

    void spawnTarget();           // 生成一个目标
    void update(float deltaTime);  // 每帧更新目标位置

private:
    QList<Target*> m_activeTargets;
    int m_targetIdCounter;
};
```

目标随机生成在玩家前方的锥形区域内：
- 距离范围：5~20 米
- 角度范围：FOV 以内（±45°）
- 高度范围：0.5~3 米

### 6.3 目标移动轨迹

设计以下几种基础运动模式：

| 模式 | 描述 | 适用训练类型 |
|------|------|-------------|
| **静止** | 目标固定不动 | 基础瞄准 |
| **线性移动** | 直线从 A 点到 B 点 | Flick（闪瞄）训练 |
| **往复运动** | 在两点之间来回移动 | Tracking（追踪）训练 |
| **随机跳跃** | 随机传送到新位置 | 反应训练 |
| **圆周运动** | 绕某点做圆周轨迹 | Tracking 训练 |
| **Z 字形移动** | 折线形移动 | 综合训练 |

**目标生命管理**：
- 每个目标有存活时间（2~5 秒）
- 超时未命中则目标消失
- 已命中目标立即从场景移除

### 6.4 瞄准追踪算法

AimLab 的核心打分逻辑：

| 指标 | 含义 | 计算方式 |
|------|------|---------|
| **Flick** | 闪瞄能力 | 命中时，从上一个目标位置到当前目标的鼠标移动总距离 |
| **Tracking** | 追踪能力 | 在移动目标上保持鼠标跟随的时间 |
| **Clicking** | 点击速度 | 连续点击的时间间隔评估 |

### 6.5 计时器与计分系统

```cpp
class ScoreManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int hits READ hits NOTIFY hitsChanged)

public:
    void recordHit(float reactionTimeMs, float mouseDistance);
    void startRound(int durationSeconds);
    void endRound();

signals:
    void scoreChanged();
    void roundEnded(const GameStats &stats);
};
```

- **回合制**：每局 30 秒或 60 秒
- **统计项**：命中数量、生成总数、命中率、总得分、平均反应时间
- **得分公式**（参考 AimLab）：

```
基础得分 = 1000
速度加成 = max(0, 1000 - 反应时间_ms)
距离加成 = max(0, 500 - 鼠标移动距离 / 10)
本回合得分 = 基础得分 + 速度加成 + 距离加成
```

### 6.6 难度分级

| 难度 | 目标大小 | 移动速度 | 目标数量 | 存活时间 |
|------|---------|---------|---------|---------|
| 简单 | 1.0x | 慢 | 少 | 长 |
| 中等 | 0.7x | 中 | 中 | 中 |
| 困难 | 0.5x | 快 | 多 | 短 |

### 里程碑四

> 目标球在 3D 空间中生成并按预定轨迹移动，鼠标控制视角瞄准，点击命中目标后得分增加，有计时器倒计时，一局结束后显示统计结果。

---

## 七、阶段五：完善与优化

> 目标：从"能玩"变成"好玩"。完善音效、UI、多模式和可视化。

### 7.1 音效系统

使用 Qt Multimedia 集成音频：

```cpp
#include <QMediaPlayer>
#include <QAudioOutput>

QMediaPlayer *player = new QMediaPlayer;
QAudioOutput *audioOutput = new QAudioOutput;
player->setAudioOutput(audioOutput);

// 播放音效
player->setSource(QUrl("qrc:/sounds/hit.wav"));
player->play();
```

- **背景音乐**：节奏感强的电子音乐
- **命中音效**：清脆的击中声
- **倒计时音效**：最后 5 秒的紧迫提示音

### 7.2 瞄准精度数据可视化

每回合结束后显示详细数据面板：

| 指标 | 说明 |
|------|------|
| 总得分 | 本局累计得分 |
| 命中率 | 命中数 / 生成总数 |
| 平均反应时间 | 毫秒（越低越好） |
| Flick 得分 | 闪瞄评估 |
| Tracking 得分 | 追踪评估 |
| Clicking 得分 | 点击评估 |
| 总鼠标移动距离 | 玩家在本局的鼠标总移动量 |

### 7.3 多种训练模式

| 模式 | 描述 |
|------|------|
| **Spheretrack** | 单个目标持续移动，要求持续追踪命中 |
| **Gridshot** | 固定网格靶位，快速依次点击 |
| **Sixshot** | 快速生成 6 个目标，按顺序依次命中 |
| **Reflex** | 目标随机出现，要求快速反应命中 |
| **Headshot** | 目标有多个部位，要求命中特定部位 |

### 7.4 UI 系统

使用 Qt Widgets 或 QML 构建界面：

```cpp
// 主菜单
class MainMenu : public QWidget {
    Q_OBJECT
public:
    MainMenu(QWidget *parent = nullptr);

private slots:
    void startGame();
    void selectMode(int mode);
    void openSettings();
};
```

- **主菜单**：游戏标题、开始游戏、选择模式、设置、退出
- **游戏 HUD**：实时显示分数、计时器、当前模式
- **暂停菜单**：ESC 呼出
- **结算界面**：回合结束统计

### 7.5 存档系统

```cpp
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void saveHighScore(const QString &mode, int score) {
    json data;
    data["highscores"][mode.toStdString()] = score;

    std::ofstream file("highscores.json");
    file << data.dump();
}
```

### 7.6 性能优化

- **对象池**：目标复用，避免频繁内存分配
- **固定时间步**：逻辑帧与渲染帧分离
- **Qt3D 优化**：减少 Draw Call，使用实例化渲染

### 7.7 美化与特效

- **目标爆炸效果**：命中时播放粒子效果
- **命中特效**：准星变色
- **动态准星**：根据游戏状态改变准星颜色

### 里程碑五（最终验收）

> 一个完整的 AimLab 风格瞄准训练游戏，包含多模式、可视化数据反馈、音效、难度分级，可独立运行。

---

## 八、总体路线图

| 周数 | 阶段 | 目标 |
|------|------|------|
| 第 1 周 | 环境搭建 | 配置 Qt Creator，创建 Qt3D 项目，显示 3D 场景 |
| 第 2 周 | 图形渲染 | 3D 场景 + 摄像机 + 准星 + 键盘控制 |
| 第 3 周 | 输入交互 | 鼠标捕获 + Delta Mouse + 射线检测 + 命中反馈 |
| 第 4 周 | 目标系统 | 目标生成 + 移动轨迹 + 计分系统 + 难度分级 |
| 第 5~6 周 | 完善优化 | 音效 + UI + 多模式 + 数据可视化 |

---

## 九、关键提醒

### 开发原则

1. **每个阶段都要验证** — 每完成一步就测试编译运行
2. **先跑通再优化** — 初版不必完美
3. **里程碑驱动** — 每个阶段设定明确的目标

### 技术重点

1. **Qt3D 是关键** — Qt 自带的 3D 模块比原生 OpenGL 好上手
2. **Delta Mouse 是核心** — AimLab 的精髓在于精准追踪玩家鼠标路径
3. **帧率稳定性** — 使用 `QElapsedTimer` 确保 delta time 准确

### 学习资源

- **Qt 官方文档**：[doc.qt.io](https://doc.qt.io)
- **Qt3D 教程**：[doc.qt.io/qt-6/qt3d-overview.html](https://doc.qt.io/qt-6/qt3d-overview.html)
- **Qt Forum**：[forum.qt.io](https://forum.qt.io)
- **B站 Qt 教程**：搜索 "Qt6 教程" 或 "Qt3D 入门"

### 扩展方向

- 添加多人联机对战模式
- 引入更复杂的物理模拟
- 使用 QML 开发更现代的 UI
- 接入 Steam SDK，发布到 Steam 平台

---

*祝开发顺利！*
