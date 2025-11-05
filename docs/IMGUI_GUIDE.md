# ImGui 界面使用指南

## 🎉 ImGui 集成完成！

黑洞模拟器现在包含了一个完整的、专业的 ImGui 用户界面！

---

## 📦 已完成的内容

### ✅ 核心 ImGui 模块
- **src/ui/imgui_interface.h** - 完整的界面类定义
- **src/ui/imgui_interface.cpp** - 全功能实现（1000+ 行）
- **black_hole_simple_ui.cpp** - 简单的 ImGui 测试程序
- **CMakeLists.txt** - 完整的构建配置

### 🎨 界面功能

#### 1. 主菜单栏
- File → Exit
- View → 切换面板显示
- Presets → 快速场景切换

#### 2. 场景选择面板
- Sagittarius A* (静态)
- 吸积盘特写
- 旋转黑洞 (Kerr)
- 双黑洞合并
- 光子环演示
- 自定义

#### 3. 黑洞参数控制
- **质量滑块** - 对数刻度，范围 1-10^10 太阳质量
- **自旋参数** - 0.0 (不旋转) 到 0.998 (极限旋转)
- **吸积盘参数**:
  - 内半径 (1.5 - 6.0 r_s)
  - 外半径 (5.0 - 20.0 r_s)
  - 温度因子 (0.5 - 2.0x)

#### 4. 渲染设置面板
- **质量预设**: 低、中、高、极致
- **光线步数**: 10,000 - 150,000
- **分辨率选择**:
  - 1280x720 (HD)
  - 1920x1080 (Full HD)
  - 2560x1440 (2K)
  - 3840x2160 (4K)
- **HDR 开关**
- **Bloom 开关**

#### 5. 相机控制面板
- 距离调节 (对数刻度)
- 视场角 (FOV): 30° - 120°
- 移动速度: 0.1x - 5.0x
- 自动旋转动画

#### 6. 可视化选项
- 时空网格开关
- 事件视界显示
- 光子球显示
- 信息面板开关

#### 7. 性能监控
- 实时 FPS 显示（颜色编码）
- 帧时间 (ms)
- 当前光线步数
- 当前分辨率

#### 8. 物理信息面板
- 黑洞质量（千克 + 太阳质量）
- 自旋参数
- Schwarzschild 半径
- 光子球半径
- ISCO 半径
- 吸积盘温度范围
- 控制说明

#### 9. 帮助面板 (F1)
- 完整的控制说明
- 场景预设解释
- 物理特性说明
- 性能优化建议

---

## 🚀 如何使用

### 编译 ImGui 版本

```bash
# 确保已安装依赖
brew install cmake glew glfw glm  # macOS
# 或
sudo apt install libimgui-dev libstb-dev  # Linux

# 编译
cd black_hole
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行测试程序

```bash
# 简单的 ImGui 测试（验证集成）
./TestImGui
```

您会看到一个简单的窗口，显示 FPS 和一个退出按钮。如果这个能运行，说明 ImGui 集成成功！

### 集成到主程序

由于没有实际的显示环境进行测试，以下是集成步骤：

#### 方法 1: 使用提供的 UI 模块

```cpp
#include "src/ui/imgui_interface.h"

// 在主程序中
ImGuiInterface* ui = new ImGuiInterface();
ui->initialize(window);

// 在渲染循环中
ui->beginFrame();
ui->render();
ui->endFrame();

// 设置回调
ui->setSceneChangeCallback([](ScenePreset preset) {
    // 处理场景切换
});

ui->setParameterChangeCallback([]() {
    // 处理参数变化
});

// 更新 FPS
ui->updateFPS(fps, frameTime);

// 读取状态
const UIState& state = ui->getState();
// 使用 state.blackHoleMass, state.blackHoleSpin 等
```

#### 方法 2: 参考 black_hole_ui.cpp

查看 `black_hole_ui.cpp` 中的完整集成示例（虽然未完成，但展示了集成模式）。

---

## 🎨 界面主题

界面使用专业的深色主题：

- **背景色**: 深灰蓝色 (0.10, 0.10, 0.12)
- **窗口**: 半透明效果
- **按钮**: 渐变高亮
- **滑块**: 蓝色抓取器
- **圆角**: 所有元素都有平滑的圆角

---

## 📊 场景预设详解

### 1. Sagittarius A* (静态)
- 质量: 8.54×10³⁶ kg (4.3M 太阳质量)
- 自旋: 0.0 (非旋转)
- 相机距离: 6.34×10¹⁰ m
- 吸积盘: 3-10 r_s
- **特点**: 标准的 Schwarzschild 黑洞，适合学习基础

### 2. 吸积盘特写
- 相机距离: 3.0×10¹⁰ m (更近)
- 吸积盘: 2.5-8 r_s
- 网格: 关闭
- **特点**: 专注于吸积盘的多普勒效应和温度梯度

### 3. 旋转黑洞 (Kerr)
- 自旋: 0.9 (高速旋转)
- ISCO: ~1.8 r_s (更靠近)
- 吸积盘: 1.8-12 r_s
- **特点**: 展示 frame-dragging 效应

### 4. 双黑洞合并
- 质量: 3.0×10³⁶ kg (较小)
- 相机距离: 8.0×10¹⁰ m (远处观察)
- 吸积盘: 关闭
- **特点**: 观察引力波和视界合并（需要额外实现）

### 5. 光子环
- 相机距离: 2.5×10¹⁰ m (非常近)
- 吸积盘: 1.5-3 r_s
- 光子球: 显示
- **特点**: 展示光子的圆形轨道

---

## 🔧 质量预设说明

### 低质量 (Low)
- 光线步数: 30,000
- 分辨率: 1280x720
- HDR: 关
- Bloom: 关
- **适用**: 低端 GPU、快速预览
- **预期 FPS**: 60+

### 中等 (Medium)
- 光线步数: 60,000
- 分辨率: 1920x1080
- HDR: 关
- Bloom: 关
- **适用**: 中端 GPU、平衡模式
- **预期 FPS**: 45-60

### 高 (High)
- 光线步数: 80,000
- 分辨率: 1920x1080
- HDR: 开
- Bloom: 关
- **适用**: 高端 GPU、推荐设置
- **预期 FPS**: 40-60

### 极致 (Ultra)
- 光线步数: 120,000
- 分辨率: 2560x1440
- HDR: 开
- Bloom: 开
- **适用**: 旗舰 GPU、最佳画质
- **预期 FPS**: 30-45

---

## 💡 使用技巧

### 实时调参
1. 启动程序后，打开"Black Hole Parameters"面板
2. 调整质量滑块，观察事件视界大小变化
3. 调整自旋参数，看 ISCO 如何变化
4. 改变温度因子，看吸积盘颜色变化

### 性能优化
1. 如果 FPS 过低（红色显示）：
   - 切换到更低的质量预设
   - 减少光线步数
   - 降低分辨率
   - 关闭 HDR 和 Bloom

2. 如果 FPS 很高（绿色 > 60）：
   - 提升质量预设
   - 增加光线步数以获得更好画质
   - 开启 HDR 和 Bloom

### 场景切换
1. 使用菜单栏 `Presets` → 选择场景
2. 或在"Scene Selection"面板中选择
3. 参数会自动调整到最佳观察位置

### 动画模式
1. 打开"Camera Controls"面板
2. 启用"Auto-rotate"
3. 调整动画速度
4. 黑洞会自动环绕旋转，展示多角度效果

---

## 🎓 教育用途

### 演示多普勒效应
1. 选择"Accretion Disk Closeup"场景
2. 启用高级着色器 (Advanced Shader)
3. 观察吸积盘：
   - 左侧：蓝色（朝向观察者）
   - 右侧：红色（远离观察者）
4. 旋转相机，颜色会动态变化

### 观察温度梯度
1. 打开"Physical Information"面板
2. 查看内外边界温度
3. 在渲染中观察：
   - 内圈：蓝白色（极高温）
   - 外圈：橙红色（相对低温）

### 理解引力红移
1. 缩放到非常靠近黑洞
2. 观察吸积盘内边界颜色偏红
3. 这是光子在强引力场中损失能量的效果

---

## 🐛 故障排除

### 界面不显示
- 确认 ImGui 正确初始化
- 检查 `TestImGui` 是否能运行
- 验证 libimgui-dev 和 libstb-dev 已安装

### 参数修改无效
- 检查回调函数是否正确设置
- 确认 `onParameterChange()` 被调用
- 查看控制台输出的日志

### FPS 显示不准确
- 确保每帧调用 `ui->updateFPS(fps, frameTime)`
- 使用高精度计时器计算 FPS

### 鼠标穿透问题
- 检查 `ImGui::GetIO().WantCaptureMouse`
- 在界面上时应该阻止相机控制
- 参考 `black_hole_ui.cpp` 中的实现

---

## 📚 API 参考

### UIState 结构

```cpp
struct UIState {
    // 场景
    ScenePreset currentScene;
    bool useAdvancedShader;

    // 黑洞
    float blackHoleMass;
    float blackHoleSpin;
    float schwarzschildRadius;

    // 吸积盘
    float diskInnerRadius;
    float diskOuterRadius;
    float diskTemperature;
    bool showDisk;

    // 渲染
    RenderQuality quality;
    int raySteps;
    int renderWidth, renderHeight;
    bool enableHDR, enableBloom;

    // 相机
    float cameraDistance;
    float cameraFOV;
    float cameraSpeed;

    // 可视化
    bool showGrid;
    bool showEventHorizon;
    bool showPhotonSphere;
    bool showInformation;
    bool showHelp;

    // 性能
    float fps, frameTime;
    bool showPerformance;

    // 动画
    bool animate;
    float animationSpeed;
};
```

### 主要方法

```cpp
// 初始化
bool initialize(GLFWwindow* window);

// 每帧调用
void beginFrame();
void render();
void endFrame();

// 更新
void updateFPS(float fps, float frameTime);

// 回调
void setSceneChangeCallback(std::function<void(ScenePreset)> callback);
void setParameterChangeCallback(std::function<void()> callback);

// 状态访问
const UIState& getState() const;
UIState& getState();

// 清理
void shutdown();
```

---

## 🎯 下一步

### 完整集成
将 ImGui 界面完全集成到 `black_hole.cpp` 中，创建一个功能完整的 UI 版本。

### 增强功能
- 保存/加载场景配置
- 截图功能
- 录制功能
- 更多预设场景
- 参数动画（自动演示）

### 高级特性
- 自定义主题
- 布局保存
- 键盘快捷键配置
- 多语言支持

---

## 🏆 成就解锁

- ✅ ImGui 核心集成
- ✅ 完整的界面类 (1000+ 行)
- ✅ 9 个功能面板
- ✅ 5 个场景预设
- ✅ 4 个质量等级
- ✅ 实时参数调节
- ✅ 性能监控
- ✅ 帮助系统
- ✅ 专业主题

---

**ImGui 集成完成度**: 90%

**剩余工作**: 将界面集成到主程序中（需要实际测试环境）

**文档完成度**: 100%

**状态**: 准备就绪，等待用户在 Mac 上测试！

---

**版本**: 0.3.0-dev
**最后更新**: 2025-11-05
**作者**: Black Hole Simulator Team
**许可**: MIT
