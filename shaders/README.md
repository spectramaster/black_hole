# 着色器说明文档

## 概述

本目录包含黑洞模拟器的所有 GLSL 着色器。

---

## 着色器列表

### 1. `geodesic.comp` - 基础光线追踪
**版本**: OpenGL 4.1 Core
**类型**: 计算着色器
**用途**: 基础的 Schwarzschild 黑洞光线追踪

**特性**:
- Schwarzschild 度规测地线积分
- RK4 数值方法
- 简单吸积盘渲染
- 事件视界检测

**性能**:
- 中等复杂度
- ~60,000 步/光线

**使用场景**:
- 快速预览
- 调试和测试
- 低端硬件

---

### 2. `accretion_disk_advanced.comp` ⭐ 新！惊艳版
**版本**: OpenGL 4.1 Core
**类型**: 计算着色器
**用途**: 物理精确、视觉惊艳的吸积盘渲染

**物理效果**:

#### ✨ Shakura-Sunyaev 温度模型
```glsl
T(r) ∝ (M/r³)^(1/4)
```
- 内边界（ISCO）: ~10⁷ K（蓝白色）
- 外边界: ~10⁴ K（红色）
- 平滑的温度梯度

#### 🔵 多普勒红移/蓝移
```glsl
δ = 1 / [γ(1 - β·cos(θ))]
```
- **蓝移**: 盘的前侧（朝向观察者）
- **红移**: 盘的后侧（远离观察者）
- 相对论性多普勒因子

效果：
- 前侧更蓝、更亮
- 后侧更红、更暗
- 清晰的颜色不对称性

#### 🔴 引力红移
```glsl
z_grav = 1/√(1 - r_s/r) - 1
```
- 深引力势阱中的光子能量损失
- 内边界红移更强
- 叠加在多普勒效应之上

#### 💫 相对论性增亮（Beaming）
```glsl
I' = δ³ * I
```
- 靠近观察者的部分强烈增亮
- 远离观察者的部分变暗
- 立体感增强

#### 🌈 黑体辐射颜色
- 基于 Planck 曲线
- 温度 → RGB 映射
  - 3000K: 深红色
  - 6000K: 黄白色（类太阳）
  - 10000K+: 蓝白色

**视觉效果**:
- 🎨 丰富的颜色渐变
- ✨ 惊艳的光影效果
- 🌀 动态的视觉不对称
- 💎 接近电影级质量

**性能**:
- 高复杂度
- 额外计算: 温度、多普勒、引力红移
- 推荐用于高端 GPU

**对比原版**:
```
基础版 (geodesic.comp):
- 单一颜色吸积盘
- 无物理效果
- FPS: 高

高级版 (accretion_disk_advanced.comp):
- 多物理效果叠加
- 视觉震撼
- FPS: 中-高（取决于硬件）
```

**推荐硬件**:
- **最佳**: Mac M1+, RTX 3060+
- **良好**: GTX 1060, Mac Intel (独显)
- **可用**: 集成显卡（降低分辨率）

---

### 3. `grid.vert` / `grid.frag` - 时空网格
**版本**: OpenGL 3.3 Core
**类型**: 顶点 + 片段着色器
**用途**: 时空曲率可视化

**特性**:
- 弯曲的二维网格
- Schwarzschild 几何变形
- 半透明渲染

---

## 使用指南

### 切换着色器

#### 方法 1: 修改代码（当前）
```cpp
// 在 black_hole.cpp 中
computeProgram = CreateComputeProgram("accretion_disk_advanced.comp");
```

#### 方法 2: ImGui 界面（即将推出）
```
场景设置 > 渲染器
○ 基础渲染（快速）
● 高级渲染（惊艳）⭐
```

### 性能调优

如果帧率过低：
1. 降低分辨率
2. 减少光线步数
3. 切换到基础着色器

### 自定义参数

修改 `DiskParams` uniform:
```cpp
float disk_r1 = r_s * 3.0;  // ISCO
float disk_r2 = r_s * 10.0;  // 外半径
float disk_spin = 0.0;       // 黑洞自旋（暂未使用）
```

---

## 开发指南

### 添加新着色器

1. 创建 `.comp`, `.vert`, 或 `.frag` 文件
2. 添加到 CMakeLists.txt 的 SHADERS glob
3. 在代码中加载：
```cpp
GLuint prog = CreateComputeProgram("my_shader.comp");
```

### 调试着色器

#### 检查编译错误
```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    // 读取错误日志
}
```

#### 输出调试信息
```glsl
// 在着色器中输出调试颜色
imageStore(outImage, pix, vec4(debug_value, 0, 0, 1));
```

### 优化技巧

1. **减少分支**
   ```glsl
   // 避免
   if (condition) { ... } else { ... }

   // 更好
   float factor = float(condition);
   result = mix(valueA, valueB, factor);
   ```

2. **向量化操作**
   ```glsl
   // 使用 vec3/vec4 而非多个 float
   vec3 result = a * b + c;  // 单次操作
   ```

3. **早期退出**
   ```glsl
   if (ray.r > ESCAPE_R) return;  // 尽早退出
   ```

---

## 公式参考

### Schwarzschild 度规
```
ds² = -(1 - r_s/r)c²dt² + dr²/(1-r_s/r) + r²(dθ² + sin²θ dφ²)
```

### 测地线方程（简化）
```
d²r/dλ²  = -Γʳₜₜ(dt/dλ)² + Γʳᵣᵣ(dr/dλ)² + ...
d²θ/dλ² = ...
d²φ/dλ² = ...
```

### 多普勒因子
```
δ = ν_obs/ν_emit = 1/[γ(1 - β·n)]
```
其中:
- γ = 1/√(1-β²) 洛伦兹因子
- β = v/c
- n = 观察方向单位向量

---

## 预设场景（即将推出）

### 场景 1: 静态 Sagittarius A*
- 着色器: `accretion_disk_advanced.comp`
- 参数: 默认
- 相机: 赤道视角

### 场景 2: 极高温吸积盘
- 着色器: `accretion_disk_advanced.comp`
- 温度: 2x 正常
- 效果: 更蓝、更亮

### 场景 3: 快速预览
- 着色器: `geodesic.comp`
- 步数: 30,000
- 用途: 调试、低端硬件

---

## 故障排除

### 问题: 着色器编译失败
**解决方案**:
- 检查 OpenGL 版本（需要 4.1+）
- 查看编译日志
- 确认着色器语法（`#version 410 core`）

### 问题: 渲染结果全黑
**可能原因**:
- Uniform 未正确绑定
- 相机参数错误
- 光线全部掉入黑洞（检查相机距离）

### 问题: 性能极低
**优化措施**:
1. 降低 `steps` 变量
2. 减小 `local_size_x/y`
3. 切换到简单着色器

---

## 版本历史

### v0.2.0 - 2025-11-05
- ✨ 新增 `accretion_disk_advanced.comp`
- 🔧 `geodesic.comp` 版本降级到 4.1
- 📝 创建本文档

### v0.1.0 - 原始版本
- `geodesic.comp` (基础版)
- `grid.vert/frag`

---

## 参考资料

### 学术论文
1. Shakura & Sunyaev (1973) - 吸积盘温度模型
2. Luminet (1979) - 黑洞吸积盘可视化
3. James et al. (2015) - 《星际穿越》渲染技术

### 实现参考
- Interstellar DNGR (Double Negative Gravitational Renderer)
- Schwarzschild Raytracer by Riccardo Antonelli
- General Relativity Geodesics by SciPy

---

**维护者**: Black Hole Simulator Team
**最后更新**: 2025-11-05
**OpenGL 版本**: 4.1 Core Profile
**着色器语言**: GLSL 4.10
