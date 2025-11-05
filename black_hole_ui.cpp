/**
 * Black Hole Simulator - UI Enhanced Version
 *
 * Features:
 * - Complete ImGui interface
 * - Scene presets
 * - Real-time parameter adjustment
 * - Performance monitoring
 * - Advanced accretion disk rendering
 * - Kerr metric support (coming soon)
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <chrono>
#include <fstream>

// ImGui
#include "src/ui/imgui_interface.h"

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;
using namespace std;
using namespace BlackHoleSim;
using Clock = std::chrono::high_resolution_clock;

// VARS
double lastPrintTime = 0.0;
int    framesCount   = 0;
double c = 299792458.0;
double G = 6.67430e-11;
struct Ray;
bool Gravity = false;

// Global UI interface
ImGuiInterface* uiInterface = nullptr;

struct Camera {
    vec3 target = vec3(0.0f, 0.0f, 0.0f);
    float radius = 6.34194e10f;
    float minRadius = 1e10f, maxRadius = 1e12f;

    float azimuth = 0.0f;
    float elevation = M_PI / 2.0f;

    float orbitSpeed = 0.01f;
    float panSpeed = 0.01f;
    double zoomSpeed = 25e9f;

    bool dragging = false;
    bool panning = false;
    bool moving = false;
    double lastX = 0.0, lastY = 0.0;

    vec3 position() const {
        float clampedElevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
        return vec3(
            radius * sin(clampedElevation) * cos(azimuth),
            radius * cos(clampedElevation),
            radius * sin(clampedElevation) * sin(azimuth)
        );
    }

    void update() {
        target = vec3(0.0f, 0.0f, 0.0f);
        if(dragging | panning) {
            moving = true;
        } else {
            moving = false;
        }
    }

    void processMouseMove(double x, double y) {
        // Don't process if mouse is over ImGui window
        if (uiInterface && ImGui::GetIO().WantCaptureMouse) {
            lastX = x;
            lastY = y;
            return;
        }

        float dx = float(x - lastX);
        float dy = float(y - lastY);

        if (dragging && panning) {
            // Pan disabled
        }
        else if (dragging && !panning) {
            azimuth   += dx * orbitSpeed;
            elevation -= dy * orbitSpeed;
            elevation = glm::clamp(elevation, 0.01f, float(M_PI) - 0.01f);
        }

        lastX = x;
        lastY = y;
        update();
    }

    void processMouseButton(int button, int action, int mods, GLFWwindow* win) {
        // Don't process if mouse is over ImGui window
        if (uiInterface && ImGui::GetIO().WantCaptureMouse) {
            return;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (action == GLFW_PRESS) {
                dragging = true;
                panning = false;
                glfwGetCursorPos(win, &lastX, &lastY);
            } else if (action == GLFW_RELEASE) {
                dragging = false;
                panning = false;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                Gravity = true;
            } else if (action == GLFW_RELEASE) {
                Gravity = false;
            }
        }
    }

    void processScroll(double xoffset, double yoffset) {
        // Don't process if mouse is over ImGui window
        if (uiInterface && ImGui::GetIO().WantCaptureMouse) {
            return;
        }

        radius -= yoffset * zoomSpeed;
        radius = glm::clamp(radius, minRadius, maxRadius);
        update();
    }

    void processKey(int key, int scancode, int action, int mods) {
        // Don't process if keyboard is captured by ImGui
        if (uiInterface && ImGui::GetIO().WantCaptureKeyboard) {
            return;
        }

        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_G) {
                Gravity = !Gravity;
                cout << "[INFO] Gravity turned " << (Gravity ? "ON" : "OFF") << endl;
            } else if (key == GLFW_KEY_F1 && uiInterface) {
                uiInterface->getState().showHelp = !uiInterface->getState().showHelp;
            }
        }
    }
};
Camera camera;

struct BlackHole {
    vec3 position;
    double mass;
    double radius;
    double r_s;

    BlackHole(vec3 pos, float m) : position(pos), mass(m) {
        r_s = 2.0 * G * mass / (c*c);
    }

    bool Intercept(float px, float py, float pz) const {
        double dx = double(px) - double(position.x);
        double dy = double(py) - double(position.y);
        double dz = double(pz) - double(position.z);
        double dist2 = dx * dx + dy * dy + dz * dz;
        return dist2 < r_s * r_s;
    }
};

BlackHole SagA(vec3(0.0f, 0.0f, 0.0f), 8.54e36);

struct ObjectData {
    vec4 posRadius;
    vec4 color;
    float mass;
    vec3 velocity = vec3(0.0f, 0.0f, 0.0f);
};

vector<ObjectData> objects = {
    { vec4(4e11f, 0.0f, 0.0f, 4e10f), vec4(1,1,0,1), 1.98892e30 },
    { vec4(0.0f, 0.0f, 4e11f, 4e10f), vec4(1,0,0,1), 1.98892e30 },
    { vec4(0.0f, 0.0f, 0.0f, SagA.r_s), vec4(0,0,0,1), static_cast<float>(SagA.mass) },
};

struct Engine {
    GLuint gridShaderProgram;
    GLFWwindow* window;
    GLuint quadVAO;
    GLuint texture;
    GLuint shaderProgram;
    GLuint computeProgram = 0;
    GLuint cameraUBO = 0;
    GLuint diskUBO = 0;
    GLuint objectsUBO = 0;
    GLuint gridVAO = 0;
    GLuint gridVBO = 0;
    GLuint gridEBO = 0;
    int gridIndexCount = 0;

    int WIDTH = 1920;
    int HEIGHT = 1080;
    int COMPUTE_WIDTH  = 200;
    int COMPUTE_HEIGHT = 150;
    float width = 100000000000.0f;
    float height = 75000000000.0f;

    // Current shader being used
    string currentShader = "geodesic.comp";

    Engine() {
        if (!glfwInit()) {
            cerr << "GLFW init failed\n";
            exit(EXIT_FAILURE);
        }

        #ifdef __APPLE__
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            cout << "[INFO] macOS detected, using OpenGL 4.1" << endl;
        #else
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            cout << "[INFO] Using OpenGL 4.3" << endl;
        #endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Black Hole Simulator - UI Enhanced", nullptr, nullptr);
        if (!window) {
            cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        GLenum glewErr = glewInit();
        if (glewErr != GLEW_OK) {
            cerr << "Failed to initialize GLEW: "
                << (const char*)glewGetErrorString(glewErr) << "\n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

        #ifdef __APPLE__
            if (glewIsSupported("GL_ARB_compute_shader")) {
                cout << "[INFO] Compute shaders supported via GL_ARB_compute_shader" << endl;
            } else {
                cerr << "[WARNING] Compute shaders may not be fully supported on this Mac" << endl;
                cerr << "[INFO] Attempting to use compute shaders anyway..." << endl;
            }
        #endif

        this->shaderProgram = CreateShaderProgram();
        gridShaderProgram = CreateShaderProgram("grid.vert", "grid.frag");
        computeProgram = CreateComputeProgram(currentShader.c_str());

        glGenBuffers(1, &cameraUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferData(GL_UNIFORM_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, cameraUBO);

        glGenBuffers(1, &diskUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, diskUBO);

        glGenBuffers(1, &objectsUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);
        GLsizeiptr objUBOSize = sizeof(int) + 3 * sizeof(float)
            + 16 * (sizeof(vec4) + sizeof(vec4))
            + 16 * sizeof(float);
        glBufferData(GL_UNIFORM_BUFFER, objUBOSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, objectsUBO);

        auto result = QuadVAO();
        this->quadVAO = result[0];
        this->texture = result[1];

        // Initialize ImGui
        uiInterface = new ImGuiInterface();
        if (!uiInterface->initialize(window)) {
            cerr << "Failed to initialize ImGui" << endl;
            delete uiInterface;
            uiInterface = nullptr;
        } else {
            cout << "[INFO] ImGui initialized successfully" << endl;

            // Set up callbacks
            uiInterface->setSceneChangeCallback([this](ScenePreset preset) {
                onSceneChange(preset);
            });

            uiInterface->setParameterChangeCallback([this]() {
                onParameterChange();
            });
        }
    }

    ~Engine() {
        if (uiInterface) {
            delete uiInterface;
            uiInterface = nullptr;
        }
    }

    void onSceneChange(ScenePreset preset) {
        cout << "[INFO] Scene changed to: " << static_cast<int>(preset) << endl;

        // Update black hole mass from UI
        const UIState& state = uiInterface->getState();
        SagA.mass = state.blackHoleMass;
        SagA.r_s = 2.0 * G * SagA.mass / (c*c);

        // Update camera distance
        camera.radius = state.cameraDistance;
        camera.update();

        // Update objects
        objects[2].posRadius.w = SagA.r_s;
        objects[2].mass = static_cast<float>(SagA.mass);
    }

    void onParameterChange() {
        const UIState& state = uiInterface->getState();

        // Update black hole
        SagA.mass = state.blackHoleMass;
        SagA.r_s = 2.0 * G * SagA.mass / (c*c);

        // Update camera
        camera.radius = state.cameraDistance;

        // Switch shader if needed
        if (state.useAdvancedShader && currentShader != "shaders/accretion_disk_advanced.comp") {
            currentShader = "shaders/accretion_disk_advanced.comp";
            reloadComputeShader();
        } else if (!state.useAdvancedShader && currentShader != "geodesic.comp") {
            currentShader = "geodesic.comp";
            reloadComputeShader();
        }
    }

    void reloadComputeShader() {
        if (computeProgram) {
            glDeleteProgram(computeProgram);
        }
        cout << "[INFO] Loading shader: " << currentShader << endl;
        computeProgram = CreateComputeProgram(currentShader.c_str());
    }

    void generateGrid(const vector<ObjectData>& objects) {
        const int gridSize = 25;
        const float spacing = 1e10f;

        vector<vec3> vertices;
        vector<GLuint> indices;

        for (int z = 0; z <= gridSize; ++z) {
            for (int x = 0; x <= gridSize; ++x) {
                float worldX = (x - gridSize / 2) * spacing;
                float worldZ = (z - gridSize / 2) * spacing;
                float y = 0.0f;

                for (const auto& obj : objects) {
                    vec3 objPos = vec3(obj.posRadius);
                    double mass = obj.mass;
                    double r_s = 2.0 * G * mass / (c * c);
                    double dx = worldX - objPos.x;
                    double dz = worldZ - objPos.z;
                    double dist = sqrt(dx * dx + dz * dz);

                    if (dist > r_s) {
                        double deltaY = 2.0 * sqrt(r_s * (dist - r_s));
                        y += static_cast<float>(deltaY) - 3e10f;
                    } else {
                        y += 2.0f * static_cast<float>(sqrt(r_s * r_s)) - 3e10f;
                    }
                }
                vertices.emplace_back(worldX, y, worldZ);
            }
        }

        for (int z = 0; z < gridSize; ++z) {
            for (int x = 0; x < gridSize; ++x) {
                int i = z * (gridSize + 1) + x;
                indices.push_back(i);
                indices.push_back(i + 1);
                indices.push_back(i);
                indices.push_back(i + gridSize + 1);
            }
        }

        if (gridVAO == 0) glGenVertexArrays(1, &gridVAO);
        if (gridVBO == 0) glGenBuffers(1, &gridVBO);
        if (gridEBO == 0) glGenBuffers(1, &gridEBO);

        glBindVertexArray(gridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

        gridIndexCount = indices.size();
        glBindVertexArray(0);
    }

    void drawGrid(const mat4& viewProj) {
        glUseProgram(gridShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(gridShaderProgram, "viewProj"),
                        1, GL_FALSE, glm::value_ptr(viewProj));
        glBindVertexArray(gridVAO);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_LINES, gridIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    void drawFullScreenQuad() {
        glUseProgram(shaderProgram);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glEnable(GL_DEPTH_TEST);
    }

    GLuint CreateShaderProgram() {
        const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        })";

        const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        })";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    GLuint CreateShaderProgram(const char* vertPath, const char* fragPath) {
        auto loadShader = [](const char* path, GLenum type) -> GLuint {
            std::ifstream in(path);
            if (!in.is_open()) {
                std::cerr << "Failed to open shader: " << path << "\n";
                exit(EXIT_FAILURE);
            }
            std::stringstream ss;
            ss << in.rdbuf();
            std::string srcStr = ss.str();
            const char* src = srcStr.c_str();

            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);

            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                GLint logLen;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
                std::vector<char> log(logLen);
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
                std::cerr << "Shader compile error (" << path << "):\n" << log.data() << "\n";
                exit(EXIT_FAILURE);
            }
            return shader;
        };

        GLuint vertShader = loadShader(vertPath, GL_VERTEX_SHADER);
        GLuint fragShader = loadShader(fragPath, GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        GLint linkSuccess;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
        if (!linkSuccess) {
            GLint logLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(program, logLen, nullptr, log.data());
            std::cerr << "Shader link error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return program;
    }

    GLuint CreateComputeProgram(const char* path) {
        std::ifstream in(path);
        if(!in.is_open()) {
            std::cerr << "Failed to open compute shader: " << path << "\n";
            exit(EXIT_FAILURE);
        }
        std::stringstream ss;
        ss << in.rdbuf();
        std::string srcStr = ss.str();
        const char* src = srcStr.c_str();

        GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cs, 1, &src, nullptr);
        glCompileShader(cs);
        GLint ok;
        glGetShaderiv(cs, GL_COMPILE_STATUS, &ok);
        if(!ok) {
            GLint logLen;
            glGetShaderiv(cs, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetShaderInfoLog(cs, logLen, nullptr, log.data());
            std::cerr << "Compute shader compile error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, cs);
        glLinkProgram(prog);
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if(!ok) {
            GLint logLen;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(prog, logLen, nullptr, log.data());
            std::cerr << "Compute shader link error:\n" << log.data() << "\n";
            exit(EXIT_FAILURE);
        }

        glDeleteShader(cs);
        return prog;
    }

    void dispatchCompute(const Camera& cam) {
        const UIState& state = uiInterface->getState();

        int cw = cam.moving ? COMPUTE_WIDTH : state.renderWidth;
        int ch = cam.moving ? COMPUTE_HEIGHT : state.renderHeight;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cw, ch, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glUseProgram(computeProgram);
        uploadCameraUBO(cam);
        uploadDiskUBO();
        uploadObjectsUBO(objects);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        GLuint groupsX = (GLuint)std::ceil(cw / 16.0f);
        GLuint groupsY = (GLuint)std::ceil(ch / 16.0f);
        glDispatchCompute(groupsX, groupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void uploadCameraUBO(const Camera& cam) {
        struct UBOData {
            vec3 pos; float _pad0;
            vec3 right; float _pad1;
            vec3 up; float _pad2;
            vec3 forward; float _pad3;
            float tanHalfFov;
            float aspect;
            bool moving;
            int _pad4;
        } data;

        vec3 fwd = normalize(cam.target - cam.position());
        vec3 up = vec3(0, 1, 0);
        vec3 right = normalize(cross(fwd, up));
        up = cross(right, fwd);

        data.pos = cam.position();
        data.right = right;
        data.up = up;
        data.forward = fwd;

        const UIState& state = uiInterface->getState();
        data.tanHalfFov = tan(radians(state.cameraFOV * 0.5f));
        data.aspect = float(WIDTH) / float(HEIGHT);
        data.moving = cam.dragging || cam.panning;

        glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &data);
    }

    void uploadObjectsUBO(const vector<ObjectData>& objs) {
        struct UBOData {
            int   numObjects;
            float _pad0, _pad1, _pad2;
            vec4  posRadius[16];
            vec4  color[16];
            float mass[16];
        } data;

        size_t count = std::min(objs.size(), size_t(16));
        data.numObjects = static_cast<int>(count);

        for (size_t i = 0; i < count; ++i) {
            data.posRadius[i] = objs[i].posRadius;
            data.color[i] = objs[i].color;
            data.mass[i] = objs[i].mass;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, objectsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &data);
    }

    void uploadDiskUBO() {
        const UIState& state = uiInterface->getState();

        float r1 = SagA.r_s * state.diskInnerRadius;
        float r2 = SagA.r_s * state.diskOuterRadius;
        float num = 2.0;
        float thickness = 1e9f;
        float diskData[4] = { r1, r2, num, thickness };

        glBindBuffer(GL_UNIFORM_BUFFER, diskUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(diskData), diskData);
    }

    vector<GLuint> QuadVAO() {
        float quadVertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, COMPUTE_WIDTH, COMPUTE_HEIGHT,
                    0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        vector<GLuint> VAOtexture = {VAO, texture};
        return VAOtexture;
    }
};

Engine engine;

void setupCameraCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, &camera);

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processMouseButton(button, action, mods, win);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processMouseMove(x, y);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoffset, double yoffset) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processScroll(xoffset, yoffset);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        Camera* cam = (Camera*)glfwGetWindowUserPointer(win);
        cam->processKey(key, scancode, action, mods);
    });
}

int main() {
    setupCameraCallbacks(engine.window);

    auto t0 = Clock::now();
    lastPrintTime = chrono::duration<double>(t0.time_since_epoch()).count();

    double lastTime = glfwGetTime();
    double lastFpsTime = glfwGetTime();
    int frameCount = 0;

    cout << "[INFO] ========================================" << endl;
    cout << "[INFO] Black Hole Simulator - UI Enhanced" << endl;
    cout << "[INFO] ========================================" << endl;
    cout << "[INFO] Press F1 for help" << endl;
    cout << "[INFO] Controls:" << endl;
    cout << "[INFO]   - Left mouse: Rotate camera" << endl;
    cout << "[INFO]   - Scroll: Zoom in/out" << endl;
    cout << "[INFO]   - G: Toggle gravity simulation" << endl;
    cout << "[INFO] ========================================" << endl;

    while (!glfwWindowShouldClose(engine.window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double now = glfwGetTime();
        double dt = now - lastTime;
        lastTime = now;

        // Calculate and update FPS
        frameCount++;
        double fpsDelta = now - lastFpsTime;
        if (fpsDelta >= 0.5) {
            float fps = frameCount / fpsDelta;
            float frameTime = (fpsDelta / frameCount) * 1000.0f;
            if (uiInterface) {
                uiInterface->updateFPS(fps, frameTime);
            }
            frameCount = 0;
            lastFpsTime = now;
        }

        // Gravity simulation
        for (auto& obj : objects) {
            for (auto& obj2 : objects) {
                if (&obj == &obj2) continue;
                float dx = obj2.posRadius.x - obj.posRadius.x;
                float dy = obj2.posRadius.y - obj.posRadius.y;
                float dz = obj2.posRadius.z - obj.posRadius.z;
                float distance = sqrt(dx * dx + dy * dy + dz * dz);
                if (distance > 0 && Gravity) {
                    vector<double> direction = {dx / distance, dy / distance, dz / distance};
                    double Gforce = (G * obj.mass * obj2.mass) / (distance * distance);
                    double acc1 = Gforce / obj.mass;
                    std::vector<double> acc = {direction[0] * acc1, direction[1] * acc1, direction[2] * acc1};

                    obj.velocity.x += acc[0];
                    obj.velocity.y += acc[1];
                    obj.velocity.z += acc[2];
                    obj.posRadius.x += obj.velocity.x;
                    obj.posRadius.y += obj.velocity.y;
                    obj.posRadius.z += obj.velocity.z;
                }
            }
        }

        // Get UI state
        const UIState& state = uiInterface->getState();

        // Draw grid if enabled
        if (state.showGrid) {
            engine.generateGrid(objects);
            mat4 view = lookAt(camera.position(), camera.target, vec3(0,1,0));
            mat4 proj = perspective(radians(state.cameraFOV),
                                   float(engine.WIDTH)/engine.HEIGHT,
                                   1e9f, 1e14f);
            mat4 viewProj = proj * view;
            engine.drawGrid(viewProj);
        }

        // Raytracing
        glViewport(0, 0, engine.WIDTH, engine.HEIGHT);
        engine.dispatchCompute(camera);
        engine.drawFullScreenQuad();

        // Render ImGui on top
        if (uiInterface) {
            uiInterface->beginFrame();
            uiInterface->render();
            uiInterface->endFrame();
        }

        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }

    cout << "[INFO] Shutting down..." << endl;
    glfwDestroyWindow(engine.window);
    glfwTerminate();
    return 0;
}