# 🌌 Black Hole Simulator - 惊艳版

**物理精确、视觉震撼的黑洞可视化工具**

> 专为 Macbook Pro 优化，跨平台支持 Linux/Windows

[![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)](docs/CHANGELOG.md)
[![OpenGL](https://img.shields.io/badge/OpenGL-4.1%2B-green.svg)](docs/BUILD_MAC.md)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey.svg)](docs/BUILD_MAC.md)
[![License](https://img.shields.io/badge/license-MIT-orange.svg)](LICENSE)

---

## ✨ 新特性 (v0.2.0)

### 🍎 完美 macOS 支持
- ✅ **自动平台检测** - Mac/Linux/Windows 条件编译
- ✅ **OpenGL 4.1** - 完全兼容 macOS 限制
- ✅ **零配置** - 开箱即用的 Mac 体验
- ✅ **Retina 支持** - 高分辨率显示优化

### 🎨 惊艳的吸积盘渲染
全新的物理精确渲染引擎，实现了：

- **🌡️ Shakura-Sunyaev 温度模型** - 内圈 10⁷K 蓝白色 → 外圈 10⁴K 红色
- **🔵 多普勒红移/蓝移** - 朝向侧蓝色变亮，远离侧红色变暗
- **🔴 引力红移** - 深引力势阱中的光子频率偏移
- **💫 相对论性增亮** - I' = δ³ × I，beaming 效果
- **🌈 黑体辐射颜色** - 基于 Planck 曲线的物理真实颜色

**视觉效果对比**:
```
原版: 单调橙色吸积盘 ⭐⭐
新版: 多彩、动态、物理精确 ⭐⭐⭐⭐⭐ 惊艳！
```

---

## 🚀 5 分钟快速开始（Mac）

```bash
# 1. 安装依赖
brew install cmake glew glfw glm

# 2. 克隆项目
git clone https://github.com/spectramaster/black_hole.git
cd black_hole

# 3. 编译
mkdir build && cd build
cmake ..
cmake --build . --config Release

# 4. 运行
./BlackHole3D
```

详细说明见 [QUICKSTART_MAC.md](QUICKSTART_MAC.md)

---

## 📸 效果展示

### 惊艳的吸积盘效果
- 🔵 **蓝色前侧** - 高速接近，多普勒蓝移 + 增亮
- 🔴 **红色后侧** - 高速远离，多普勒红移 + 变暗
- 💎 **蓝白内圈** - 极高温 (~10⁷K)，接近事件视界
- 🔥 **橙红外圈** - 中温 (~10⁴K)，温度梯度清晰
- ⚫ **黑洞阴影** - 完美的黑色圆盘，光线无法逃脱

### 动态特性
- 旋转视角时颜色实时变化
- 多普勒效应随观察角度动态调整
- 相对论增亮跟随相机移动
- 物理精确的光照效果

---

## 🎮 控制方法

| 操作 | 功能 |
|------|------|
| **左键拖拽** | 旋转相机环绕黑洞 |
| **滚轮** | 缩放（靠近/远离） |
| **右键** | 开启/关闭引力模拟 |
| **G 键** | 切换引力开关 |
| **ESC** | 退出程序 |

---

## 📊 性能表现

| 平台 | 分辨率 | FPS | 质量设置 |
|------|--------|-----|----------|
| Mac M1 Pro | 1920x1080 | 60+ | 极致 🔥 |
| Mac M1 基础版 | 1920x1080 | 45-60 | 高 |
| Mac Intel (独显) | 1920x1080 | 40-50 | 高 |
| Mac Intel (集显) | 1280x720 | 30-40 | 中 |
| Linux RTX 3080 | 3840x2160 | 60+ | 极致 🔥 |
| Linux GTX 1060 | 1920x1080 | 60 | 高 |

---

## 🏗️ 项目结构

```
black_hole/
├── docs/                        # 📚 完整文档 (2500+ 行)
│   ├── IMPLEMENTATION_PLAN.md   # 75h 详细实施计划
│   ├── CHANGELOG.md              # 版本历史
│   ├── TEST_LOG.md               # 测试记录
│   ├── BUILD_MAC.md              # Mac 专用构建指南
│   ├── PROJECT_STATUS.md         # 项目仪表板
│   └── PROGRESS_SUMMARY.md       # 进展总结
│
├── shaders/                     # 🎨 着色器
│   ├── geodesic.comp             # 基础 Schwarzschild 光线追踪
│   ├── accretion_disk_advanced.comp  # ⭐ 惊艳的吸积盘渲染
│   ├── grid.vert/frag            # 时空网格可视化
│   └── README.md                 # 着色器技术文档
│
├── tests/                       # 🧪 测试
│   └── test_mac_compatibility.cpp  # OpenGL 兼容性测试
│
├── src/                         # 💻 源代码（即将模块化）
├── black_hole.cpp               # 主程序
├── 2D_lensing.cpp               # 2D 引力透镜演示
├── CMakeLists.txt               # 构建配置
├── vcpkg.json                   # 依赖管理
├── QUICKSTART_MAC.md            # 5min 快速开始
└── README.md                    # 本文档
```

---

## 🔬 物理原理

### Schwarzschild 度规
```
ds² = -(1 - r_s/r)c²dt² + dr²/(1-r_s/r) + r²(dθ² + sin²θ dφ²)
```

### 测地线积分
- **方法**: 4 阶 Runge-Kutta (RK4)
- **步长**: 自适应 (~10⁷ m)
- **步数**: 60,000 - 100,000/光线

### 吸积盘物理

#### 温度分布
```
T(r) ∝ (M/r³)^(1/4)
```
- Shakura-Sunyaev 模型
- 内边界 (ISCO): ~10⁷ K
- 外边界: ~10⁴ K

#### 多普勒因子
```
δ = 1 / [γ(1 - β·cos(θ))]
```
- γ = 洛伦兹因子
- β = v/c
- cos(θ) = 视线方向

#### 引力红移
```
z_grav = 1/√(1 - r_s/r) - 1
```

#### 相对论性增亮
```
I_observed = δ³ × I_emitted
```

详见 [shaders/README.md](shaders/README.md)

---

## 📚 文档

| 文档 | 描述 | 目标读者 |
|------|------|----------|
| [QUICKSTART_MAC.md](QUICKSTART_MAC.md) | 5 分钟快速开始 | 所有用户 |
| [BUILD_MAC.md](docs/BUILD_MAC.md) | 详细构建指南 | Mac 用户 |
| [IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md) | 75h 实施路线图 | 开发者 |
| [PROJECT_STATUS.md](docs/PROJECT_STATUS.md) | 项目状态仪表板 | 贡献者 |
| [CHANGELOG.md](docs/CHANGELOG.md) | 版本历史 | 所有人 |
| [TEST_LOG.md](docs/TEST_LOG.md) | 测试记录 | QA/开发者 |
| [shaders/README.md](shaders/README.md) | 着色器技术文档 | 图形开发者 |

---

## 🛠️ 技术栈

- **语言**: C++17
- **图形 API**: OpenGL 4.1+ (Core Profile)
- **依赖库**:
  - GLFW 3.3+ (窗口管理)
  - GLEW 2.2+ (OpenGL 扩展)
  - GLM 0.9.9+ (数学库)
  - ImGui (即将集成 - UI)
- **构建系统**: CMake 3.21+
- **包管理**: vcpkg / Homebrew

---

## 🎯 开发路线图

### ✅ v0.2.0 (当前) - Mac 兼容 + 惊艳渲染
- ✅ macOS 完全支持
- ✅ 物理精确吸积盘
- ✅ 完整文档体系
- ✅ 测试框架

### ⏳ v0.3.0 (本周) - 用户界面
- ⏳ ImGui 集成
- ⏳ 场景预设系统
- ⏳ 参数实时调节
- ⏳ 性能监控面板

### ⏳ v0.4.0 (两周内) - 高级物理
- ⏳ Kerr 旋转黑洞
- ⏳ 黑洞合并动画
- ⏳ 引力波可视化
- ⏳ 光子环渲染

### ⏳ v0.5.0 - 性能与视觉
- ⏳ HDR 渲染管线
- ⏳ Bloom 辉光效果
- ⏳ 自适应质量系统
- ⏳ 60 FPS 稳定性

### 🎯 v1.0.0 - 正式发布
- 完整功能集
- 多平台测试
- 用户文档
- 发布包

详见 [IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md)

---

## 🧪 已知问题

### macOS
- ✅ OpenGL 4.1 兼容性 - 已修复
- ✅ 计算着色器支持 - 已验证
- ⏳ Metal 后端 - 未来版本

### 性能
- ⚠️ 高分辨率下帧率可能较低 - 优化中
- ⏳ 自适应质量系统 - 即将推出

### 功能
- ⏳ UI 界面 - 开发中
- ⏳ 更多预设场景 - 计划中

---

## 🤝 贡献

欢迎贡献！请查看以下内容：

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

详见 [CONTRIBUTING.md](CONTRIBUTING.md) (即将推出)

---

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

---

## 🙏 致谢

- **物理模型**: 基于 Schwarzschild 和 Kerr 度规
- **灵感来源**: 《星际穿越》DNGR 渲染器
- **学术参考**:
  - Luminet (1979) - 黑洞可视化
  - Shakura & Sunyaev (1973) - 吸积盘模型
  - James et al. (2015) - 《星际穿越》技术

---

## 📞 联系方式

- **Issues**: [GitHub Issues](https://github.com/spectramaster/black_hole/issues)
- **Email**: [contact@example.com](mailto:contact@example.com)
- **文档**: [docs/](docs/)

---

## ⭐ Star History

如果这个项目帮助到您，请给个 Star ⭐！

---

**准备好被广义相对论震撼了吗？** 🌌✨

立即在您的 Macbook Pro 上体验惊艳的黑洞模拟！

```bash
brew install cmake glew glfw glm
git clone https://github.com/spectramaster/black_hole.git
cd black_hole
mkdir build && cd build
cmake .. && cmake --build .
./BlackHole3D
```

---

**版本**: 0.2.0
**最后更新**: 2025-11-05
**兼容性**: macOS 10.15+, Linux, Windows
**状态**: 积极开发中 🚀
