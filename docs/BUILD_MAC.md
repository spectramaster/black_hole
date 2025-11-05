# macOS 构建指南

本指南专门针对 Macbook Pro 用户，确保黑洞模拟器可以在 macOS 上丝滑运行。

---

## 系统要求

### 硬件
- **推荐**: Macbook Pro M1/M2/M3 或 Intel i7+
- **内存**: 最少 8GB RAM（推荐 16GB+）
- **显卡**: 集成显卡即可（Metal 支持的 GPU）
- **存储**: 至少 500MB 可用空间

### 软件
- **macOS**: 10.15 Catalina 或更高版本
- **Xcode**: 最新版本（用于编译器）
- **Homebrew**: 包管理器

---

## 安装步骤

### 1. 安装 Homebrew

如果尚未安装 Homebrew，在终端中运行：

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. 安装构建工具

```bash
# 安装 CMake
brew install cmake

# 安装 Git（如果没有）
brew install git
```

### 3. 克隆项目

```bash
git clone https://github.com/spectramaster/black_hole.git
cd black_hole
```

### 4. 安装依赖库

有两种方法：**Homebrew**（推荐）或 **vcpkg**

#### 方法 A: 使用 Homebrew（更简单）

```bash
brew install glew glfw glm
# ImGui 将在后续版本中通过 vcpkg 安装
```

#### 方法 B: 使用 vcpkg

```bash
# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
cd ..

# 安装依赖
./vcpkg/vcpkg install

# 获取 toolchain 文件路径
./vcpkg/vcpkg integrate install
# 记下输出的 CMAKE_TOOLCHAIN_FILE 路径
```

### 5. 构建项目

#### 使用 Homebrew 依赖：

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### 使用 vcpkg 依赖：

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### 6. 运行程序

```bash
# 3D 黑洞模拟（主程序）
./BlackHole3D

# 2D 引力透镜演示
./BlackHole2D

# Mac 兼容性测试
./TestMacCompat
```

---

## macOS 特定说明

### OpenGL 版本

macOS 仅支持 OpenGL 4.1 Core Profile。代码已自动适配：

```cpp
#ifdef __APPLE__
    // 使用 OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
```

### 计算着色器支持

虽然 macOS OpenGL 4.1 理论上支持计算着色器（通过 `GL_ARB_compute_shader` 扩展），但支持可能有限。程序会自动检测并报告：

```
[INFO] macOS detected, using OpenGL 4.1
[INFO] Compute shaders supported via GL_ARB_compute_shader
```

如果遇到问题，程序会尝试使用 CPU 回退路径。

### Retina 显示屏

程序自动支持 Retina 显示屏的高分辨率。

---

## 性能优化

### 推荐设置（在 ImGui 界面中调整）

#### Macbook Pro M1/M2/M3
- **分辨率**: 1920x1080
- **质量**: 高
- **光线步数**: 80,000 - 100,000
- **预期 FPS**: 60+

#### Macbook Pro Intel (集成显卡)
- **分辨率**: 1280x720
- **质量**: 中
- **光线步数**: 40,000 - 60,000
- **预期 FPS**: 30-45

#### Macbook Pro Intel (独立显卡)
- **分辨率**: 1920x1080
- **质量**: 高
- **光线步数**: 80,000
- **预期 FPS**: 45-60

### 性能提示

1. **移动相机时自动降低质量**
   程序会在拖拽时自动降低分辨率以保持流畅

2. **关闭后台应用**
   关闭不必要的应用以释放 GPU 资源

3. **连接电源**
   插上电源以获得最佳性能（不受电池节能限制）

---

## 故障排除

### 问题 1: OpenGL 版本错误

```
错误: OpenGL version too old
```

**解决方案**:
- 更新 macOS 到最新版本
- 更新显卡驱动（通常通过系统更新）
- 某些虚拟机不支持 OpenGL 4.1

### 问题 2: GLFW 初始化失败

```
✗ Failed to initialize GLFW
```

**解决方案**:
- 确保不在 SSH 远程会话中运行（需要本地显示）
- 重新安装 GLFW: `brew reinstall glfw`

### 问题 3: 计算着色器不可用

```
[WARNING] Compute shaders may not be fully supported
```

**解决方案**:
- 这是警告不是错误，程序会继续运行
- 性能可能略有下降
- 考虑使用较新的 Mac（M1+ 有更好的 OpenGL 支持）

### 问题 4: 帧率过低 (< 20 FPS)

**解决方案**:
1. 在 UI 中降低质量设置
2. 减少光线步数
3. 降低窗口分辨率
4. 确保连接电源
5. 关闭其他 GPU 密集型应用（视频渲染等）

### 问题 5: 窗口无法显示

**解决方案**:
- 检查是否启用了"显示器"权限
- 系统偏好设置 → 安全性与隐私 → 隐私 → 屏幕录制

---

## 卸载

```bash
# 删除构建产物
cd black_hole
rm -rf build

# 卸载依赖（如果不再需要）
brew uninstall glew glfw glm

# 删除项目
cd ..
rm -rf black_hole
```

---

## 已知限制

### macOS OpenGL 弃用

Apple 已弃用 OpenGL，推荐使用 Metal。未来版本可能会添加 Metal 后端以获得更好的性能。

### 功能差异

相比 Linux/Windows 版本，macOS 版本的差异：
- ✅ 所有核心功能完全相同
- ✅ 性能相当（M1+ 甚至更快）
- ⚠️ 某些高级 OpenGL 特性可能受限
- 🔮 未来将支持 Metal 加速

---

## 获取帮助

如果遇到问题：

1. 查看 [测试日志](TEST_LOG.md) 了解已知问题
2. 查看 [变更日志](CHANGELOG.md) 了解最新更新
3. 在 GitHub 提交 Issue
4. 包含以下信息：
   - macOS 版本
   - Mac 型号（M1/M2/Intel）
   - 错误信息
   - `./TestMacCompat` 的输出

---

## 下一步

构建成功后，查看：
- **用户指南** (即将推出): 如何使用程序
- **场景介绍** (即将推出): 预设场景说明
- **自定义设置** (即将推出): 调整物理参数

---

**祝您在 Mac 上享受黑洞模拟的惊艳体验！** 🌌✨

**最后更新**: 2025-11-05
**适用版本**: 0.2.0+
