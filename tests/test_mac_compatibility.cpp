/*
 * Mac Compatibility Test
 *
 * This program tests basic OpenGL initialization and compute shader support
 * on macOS systems.
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

using namespace std;

bool testOpenGLVersion() {
    cout << "\n=== OpenGL Version Test ===" << endl;

    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    cout << "OpenGL Version: " << version << endl;
    cout << "Vendor: " << vendor << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "GLSL Version: " << glslVersion << endl;

    // Parse version
    string versionStr((const char*)version);
    int major = 0, minor = 0;
    sscanf(versionStr.c_str(), "%d.%d", &major, &minor);

    cout << "Parsed Version: " << major << "." << minor << endl;

    if (major > 4 || (major == 4 && minor >= 1)) {
        cout << "✓ OpenGL 4.1+ supported" << endl;
        return true;
    } else {
        cout << "✗ OpenGL version too old (need 4.1+)" << endl;
        return false;
    }
}

bool testComputeShaderSupport() {
    cout << "\n=== Compute Shader Support Test ===" << endl;

    // Check for compute shader extension
    if (glewIsSupported("GL_ARB_compute_shader")) {
        cout << "✓ GL_ARB_compute_shader extension supported" << endl;
    } else {
        cout << "✗ GL_ARB_compute_shader extension NOT supported" << endl;
    }

    // Check for compute shader capability
    GLint maxComputeWorkGroupCount[3];
    GLint maxComputeWorkGroupSize[3];
    GLint maxComputeWorkGroupInvocations;

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeWorkGroupCount[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxComputeWorkGroupCount[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxComputeWorkGroupCount[2]);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxComputeWorkGroupSize[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxComputeWorkGroupSize[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxComputeWorkGroupSize[2]);

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeWorkGroupInvocations);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        cout << "✗ Compute shader queries failed (Error: " << err << ")" << endl;
        return false;
    }

    cout << "Max Work Group Count: ["
         << maxComputeWorkGroupCount[0] << ", "
         << maxComputeWorkGroupCount[1] << ", "
         << maxComputeWorkGroupCount[2] << "]" << endl;

    cout << "Max Work Group Size: ["
         << maxComputeWorkGroupSize[0] << ", "
         << maxComputeWorkGroupSize[1] << ", "
         << maxComputeWorkGroupSize[2] << "]" << endl;

    cout << "Max Work Group Invocations: " << maxComputeWorkGroupInvocations << endl;

    if (maxComputeWorkGroupSize[0] >= 16 && maxComputeWorkGroupSize[1] >= 16) {
        cout << "✓ Work group size 16x16 is supported" << endl;
        return true;
    } else {
        cout << "✗ Work group size 16x16 may not be supported" << endl;
        return false;
    }
}

bool testImageFormat() {
    cout << "\n=== Image Format Support Test ===" << endl;

    // Test RGBA8 support
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLenum err = glGetError();
    glDeleteTextures(1, &texture);

    if (err == GL_NO_ERROR) {
        cout << "✓ RGBA8 texture format supported" << endl;
        return true;
    } else {
        cout << "✗ RGBA8 texture format failed (Error: " << err << ")" << endl;
        return false;
    }
}

bool testUniformBuffer() {
    cout << "\n=== Uniform Buffer Object Test ===" << endl;

    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 256, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    GLenum err = glGetError();
    glDeleteBuffers(1, &ubo);

    if (err == GL_NO_ERROR) {
        cout << "✓ Uniform Buffer Objects supported" << endl;
        return true;
    } else {
        cout << "✗ UBO test failed (Error: " << err << ")" << endl;
        return false;
    }
}

int main() {
    cout << "Black Hole Simulator - Mac Compatibility Test" << endl;
    cout << "=============================================" << endl;

    // Initialize GLFW
    if (!glfwInit()) {
        cerr << "✗ Failed to initialize GLFW" << endl;
        return 1;
    }
    cout << "✓ GLFW initialized" << endl;

    // Set OpenGL version hints
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        cout << "Platform: macOS (OpenGL 4.1 Core Profile)" << endl;
    #else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        cout << "Platform: Linux/Windows (OpenGL 4.3 Core Profile)" << endl;
    #endif

    // Create invisible window for testing
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    if (!window) {
        cerr << "✗ Failed to create GLFW window" << endl;
        glfwTerminate();
        return 1;
    }
    cout << "✓ GLFW window created" << endl;

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        cerr << "✗ Failed to initialize GLEW: " << glewGetErrorString(glewErr) << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    cout << "✓ GLEW initialized" << endl;

    // Run tests
    int passed = 0;
    int total = 0;

    total++; if (testOpenGLVersion()) passed++;
    total++; if (testComputeShaderSupport()) passed++;
    total++; if (testImageFormat()) passed++;
    total++; if (testUniformBuffer()) passed++;

    // Summary
    cout << "\n=== Test Summary ===" << endl;
    cout << "Passed: " << passed << "/" << total << endl;

    if (passed == total) {
        cout << "\n✓ All tests passed! Mac compatibility looks good." << endl;
    } else {
        cout << "\n⚠ Some tests failed. There may be compatibility issues." << endl;
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return (passed == total) ? 0 : 1;
}
