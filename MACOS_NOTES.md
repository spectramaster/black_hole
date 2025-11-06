# macOS Compatibility Notes

## OpenGL Limitations on macOS

⚠️ **Important**: macOS has deprecated OpenGL and only supports up to **OpenGL 4.1 Core Profile**. This project requires OpenGL 4.3 features (specifically Compute Shaders), which are **not officially supported** on macOS.

### What This Means

1. **Compute Shaders**: Introduced in OpenGL 4.3, compute shaders are used for GPU-accelerated ray tracing in this simulation
2. **macOS Support**: macOS OpenGL stops at version 4.1, which predates compute shaders
3. **Apple's Direction**: Apple deprecated OpenGL in macOS 10.14 Mojave (2018) in favor of Metal

### Current Status

The code has been modified to:
- Request OpenGL 4.1 with forward compatibility on macOS
- Add `GLFW_OPENGL_FORWARD_COMPAT` hint (required on macOS)
- Attempt to use compute shaders despite version mismatch

**Results may vary**:
- ✅ May work if GPU driver provides ARB_compute_shader extension
- ❌ Will likely fail with shader compilation errors
- ❌ Window creation may fail if strict version checking is enabled

### Recommended Solutions

1. **Use Linux or Windows**: Full OpenGL 4.3+ support
   ```bash
   # Linux with modern GPU
   ./build/BlackHole3D
   ```

2. **Metal Port** (Future Work): Rewrite rendering pipeline for Metal
   - Native macOS performance
   - Better future-proofing
   - Requires significant development effort

3. **Software Fallback** (Future Work): CPU-based ray tracing
   - Platform-independent
   - Much slower than GPU
   - Good for debugging/testing

### Testing on macOS

If you want to try anyway:

```bash
# 1. Build the project
./build_macos.sh

# 2. Run from project root
./run_macos.sh

# Or manually
./build/BlackHole3D
```

**Expected Errors**:
- `Failed to create GLFW window`: OpenGL 4.1 context can't be created
- `Compute shader compile error`: Version 430 not supported
- `GL_COMPUTE_SHADER not defined`: GLEW doesn't find compute shader support

### Workaround: Use a VM or Boot Camp

- **Parallels/VMware**: Run Linux VM with GPU passthrough
- **Boot Camp**: Install Windows for native performance
- **Remote**: SSH to Linux machine with proper OpenGL support

### Technical Details

| Feature              | Required | macOS Max | Status |
|---------------------|----------|-----------|--------|
| OpenGL Version      | 4.3      | 4.1       | ❌     |
| Compute Shaders     | Yes      | No        | ❌     |
| GLSL Version        | 430      | 410       | ❌     |
| Core Profile        | Yes      | Yes       | ✅     |
| Forward Compat      | No       | Required  | ✅     |

### Alternative: BlackHole2D

The 2D version (`BlackHole2D`) uses older OpenGL features and may work on macOS:

```bash
./build/BlackHole2D
```

This is a simplified gravitational lensing demo without compute shaders.

---

**For best results, use Linux or Windows with a modern GPU (NVIDIA GTX 900+ or AMD RX 400+).**
