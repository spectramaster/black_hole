/**
 * ImGui Interface Module
 *
 * Provides a professional user interface for the black hole simulator
 * with scene selection, parameter controls, and performance monitoring.
 */

#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <string>
#include <vector>
#include <functional>

namespace BlackHoleSim {

// Scene presets
enum class ScenePreset {
    SAGITTARIUS_A_STATIC,
    ACCRETION_DISK_CLOSEUP,
    ROTATING_BLACKHOLE,
    BINARY_MERGER,
    PHOTON_RING,
    CUSTOM
};

// Rendering quality levels
enum class RenderQuality {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA
};

// UI State structure
struct UIState {
    // Scene settings
    ScenePreset currentScene = ScenePreset::SAGITTARIUS_A_STATIC;
    bool useAdvancedShader = true;

    // Black hole parameters
    float blackHoleMass = 8.54e36f;  // Sagittarius A* mass in kg
    float blackHoleSpin = 0.0f;      // Spin parameter (0-0.998)
    float schwarzschildRadius = 1.269e10f;

    // Accretion disk parameters
    float diskInnerRadius = 3.0f;    // In units of r_s
    float diskOuterRadius = 10.0f;   // In units of r_s
    float diskTemperature = 1.0f;    // Multiplier
    bool showDisk = true;

    // Rendering settings
    RenderQuality quality = RenderQuality::HIGH;
    int raySteps = 80000;
    int renderWidth = 1920;
    int renderHeight = 1080;
    bool enableHDR = false;
    bool enableBloom = false;

    // Camera settings
    float cameraDistance = 6.34194e10f;
    float cameraFOV = 60.0f;
    float cameraSpeed = 1.0f;

    // Visualization options
    bool showGrid = true;
    bool showEventHorizon = true;
    bool showPhotonSphere = false;
    bool showInformation = true;
    bool showHelp = false;

    // Performance monitoring
    float fps = 0.0f;
    float frameTime = 0.0f;
    bool showPerformance = true;

    // Animation
    bool animate = false;
    float animationSpeed = 1.0f;
    float currentTime = 0.0f;
};

class ImGuiInterface {
public:
    ImGuiInterface();
    ~ImGuiInterface();

    // Initialize ImGui with GLFW and OpenGL3
    bool initialize(GLFWwindow* window);

    // Cleanup ImGui
    void shutdown();

    // Begin new frame
    void beginFrame();

    // Render all UI elements
    void render();

    // End frame and render
    void endFrame();

    // Get current UI state
    const UIState& getState() const { return state; }
    UIState& getState() { return state; }

    // Update FPS counter
    void updateFPS(float fps, float frameTime);

    // Scene preset callbacks
    void setSceneChangeCallback(std::function<void(ScenePreset)> callback) {
        sceneChangeCallback = callback;
    }

    // Parameter change callbacks
    void setParameterChangeCallback(std::function<void()> callback) {
        parameterChangeCallback = callback;
    }

private:
    UIState state;
    bool initialized = false;

    // Callback functions
    std::function<void(ScenePreset)> sceneChangeCallback;
    std::function<void()> parameterChangeCallback;

    // UI Panel rendering functions
    void renderMainMenuBar();
    void renderSceneSelector();
    void renderParameterControls();
    void renderRenderingSettings();
    void renderCameraControls();
    void renderVisualizationOptions();
    void renderPerformanceMonitor();
    void renderHelpPanel();
    void renderInformationPanel();

    // Helper functions
    const char* scenePresetName(ScenePreset preset) const;
    const char* qualityName(RenderQuality quality) const;
    void applyQualityPreset(RenderQuality quality);
    void applyScenePreset(ScenePreset preset);

    // Styling
    void setupStyle();
};

} // namespace BlackHoleSim
