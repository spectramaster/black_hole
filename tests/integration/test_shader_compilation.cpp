/**
 * Integration Test: Shader Compilation
 *
 * Tests that all shaders compile successfully
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

std::vector<TestResult> results;

std::string loadShader(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool compileShader(const char* filename, GLenum type, std::string& error) {
    std::string source = loadShader(filename);
    if (source.empty()) {
        error = "Failed to load file";
        return false;
    }

    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        error = std::string(log.data());
        glDeleteShader(shader);
        return false;
    }

    glDeleteShader(shader);
    return true;
}

void testShader(const char* name, const char* path, GLenum type) {
    std::string error;
    bool success = compileShader(path, type, error);

    if (success) {
        results.push_back({name, true, "OK"});
        std::cout << "[PASS] " << name << std::endl;
    } else {
        results.push_back({name, false, error});
        std::cout << "[FAIL] " << name << "\n       " << error << std::endl;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return 1;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "Shader Compilation Tests" << std::endl;
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "========================================\n" << std::endl;

    testShader("geodesic.comp", "../geodesic.comp", GL_COMPUTE_SHADER);
    testShader("kerr_black_hole.comp", "../shaders/kerr_black_hole.comp", GL_COMPUTE_SHADER);
    testShader("accretion_disk_advanced.comp", "../shaders/accretion_disk_advanced.comp", GL_COMPUTE_SHADER);
    testShader("grid.vert", "../grid.vert", GL_VERTEX_SHADER);
    testShader("grid.frag", "../grid.frag", GL_FRAGMENT_SHADER);

    int passed = 0;
    for (const auto& r : results) {
        if (r.passed) passed++;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << passed << "/" << results.size() << " passed";
    std::cout << (passed == results.size() ? " ✓" : " ✗") << std::endl;
    std::cout << "========================================\n" << std::endl;

    glfwDestroyWindow(window);
    glfwTerminate();

    return (passed == results.size()) ? 0 : 1;
}
