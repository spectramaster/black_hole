/**
 * ImGui Interface Implementation
 */

#include "imgui_interface.h"
#include <GLFW/glfw3.h>
#include <cmath>

namespace BlackHoleSim {

ImGuiInterface::ImGuiInterface() {
}

ImGuiInterface::~ImGuiInterface() {
    if (initialized) {
        shutdown();
    }
}

bool ImGuiInterface::initialize(GLFWwindow* window) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer backends
    #ifdef __APPLE__
        const char* glsl_version = "#version 410";
    #else
        const char* glsl_version = "#version 430";
    #endif

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        ImGui_ImplGlfw_Shutdown();
        return false;
    }

    // Setup style
    setupStyle();

    initialized = true;
    return true;
}

void ImGuiInterface::shutdown() {
    if (initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized = false;
    }
}

void ImGuiInterface::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiInterface::render() {
    // Main menu bar
    renderMainMenuBar();

    // Show panels based on state
    renderSceneSelector();

    if (state.showInformation) {
        renderInformationPanel();
    }

    renderParameterControls();
    renderRenderingSettings();
    renderCameraControls();
    renderVisualizationOptions();

    if (state.showPerformance) {
        renderPerformanceMonitor();
    }

    if (state.showHelp) {
        renderHelpPanel();
    }
}

void ImGuiInterface::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiInterface::updateFPS(float fps, float frameTime) {
    state.fps = fps;
    state.frameTime = frameTime;
}

// ========== UI Panel Implementations ==========

void ImGuiInterface::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "ESC")) {
                // Signal exit
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Information", nullptr, &state.showInformation);
            ImGui::MenuItem("Performance", nullptr, &state.showPerformance);
            ImGui::MenuItem("Help", "F1", &state.showHelp);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Presets")) {
            if (ImGui::MenuItem("Sagittarius A*")) {
                applyScenePreset(ScenePreset::SAGITTARIUS_A_STATIC);
            }
            if (ImGui::MenuItem("Accretion Disk Closeup")) {
                applyScenePreset(ScenePreset::ACCRETION_DISK_CLOSEUP);
            }
            if (ImGui::MenuItem("Rotating Black Hole")) {
                applyScenePreset(ScenePreset::ROTATING_BLACKHOLE);
            }
            if (ImGui::MenuItem("Binary Merger")) {
                applyScenePreset(ScenePreset::BINARY_MERGER);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ImGuiInterface::renderSceneSelector() {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 150), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Scene Selection", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Choose a preset scene:");
        ImGui::Separator();

        const char* scenes[] = {
            "Sagittarius A* (Static)",
            "Accretion Disk Closeup",
            "Rotating Black Hole (Kerr)",
            "Binary Black Hole Merger",
            "Photon Ring Demo",
            "Custom"
        };

        int currentItem = static_cast<int>(state.currentScene);
        if (ImGui::Combo("##Scene", &currentItem, scenes, IM_ARRAYSIZE(scenes))) {
            ScenePreset newScene = static_cast<ScenePreset>(currentItem);
            if (newScene != state.currentScene) {
                applyScenePreset(newScene);
                if (sceneChangeCallback) {
                    sceneChangeCallback(newScene);
                }
            }
        }

        ImGui::Spacing();
        if (ImGui::Checkbox("Use Advanced Shader", &state.useAdvancedShader)) {
            if (parameterChangeCallback) {
                parameterChangeCallback();
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Advanced shader includes Doppler shift,\ngravitational redshift, and relativistic beaming");
        }
    }
    ImGui::End();
}

void ImGuiInterface::renderParameterControls() {
    ImGui::SetNextWindowPos(ImVec2(10, 190), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 280), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Black Hole Parameters", nullptr, ImGuiWindowFlags_NoCollapse)) {
        bool changed = false;

        ImGui::Text("Physical Properties:");
        ImGui::Separator();

        // Mass slider (log scale)
        float massExponent = std::log10(state.blackHoleMass / 1.989e30f);  // Solar masses
        if (ImGui::SliderFloat("Mass (M☉)", &massExponent, 0.0f, 10.0f, "10^%.1f")) {
            state.blackHoleMass = std::pow(10.0f, massExponent) * 1.989e30f;
            state.schwarzschildRadius = 2.0f * 6.67430e-11f * state.blackHoleMass / (299792458.0f * 299792458.0f);
            changed = true;
        }

        // Spin parameter
        if (ImGui::SliderFloat("Spin (a)", &state.blackHoleSpin, 0.0f, 0.998f, "%.3f")) {
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Kerr spin parameter:\n0 = Schwarzschild (non-rotating)\n0.998 = Near-extremal rotation");
        }

        ImGui::Spacing();
        ImGui::Text("Accretion Disk:");
        ImGui::Separator();

        if (ImGui::Checkbox("Show Disk##disk", &state.showDisk)) {
            changed = true;
        }

        if (state.showDisk) {
            if (ImGui::SliderFloat("Inner Radius##inner", &state.diskInnerRadius, 1.5f, 6.0f, "%.1f r_s")) {
                changed = true;
            }
            if (ImGui::SliderFloat("Outer Radius##outer", &state.diskOuterRadius, 5.0f, 20.0f, "%.1f r_s")) {
                changed = true;
            }
            if (ImGui::SliderFloat("Temperature##temp", &state.diskTemperature, 0.5f, 2.0f, "%.2fx")) {
                changed = true;
            }
        }

        ImGui::Spacing();
        ImGui::Text("Schwarzschild Radius: %.2e m", state.schwarzschildRadius);

        if (changed && parameterChangeCallback) {
            parameterChangeCallback();
        }
    }
    ImGui::End();
}

void ImGuiInterface::renderRenderingSettings() {
    ImGui::SetNextWindowPos(ImVec2(340, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Rendering Settings", nullptr, ImGuiWindowFlags_NoCollapse)) {
        bool changed = false;

        ImGui::Text("Quality Preset:");
        const char* qualities[] = { "Low", "Medium", "High", "Ultra" };
        int currentQuality = static_cast<int>(state.quality);
        if (ImGui::Combo("##Quality", &currentQuality, qualities, IM_ARRAYSIZE(qualities))) {
            state.quality = static_cast<RenderQuality>(currentQuality);
            applyQualityPreset(state.quality);
            changed = true;
        }

        ImGui::Separator();
        ImGui::Text("Advanced Settings:");

        if (ImGui::SliderInt("Ray Steps", &state.raySteps, 10000, 150000, "%d")) {
            changed = true;
        }

        ImGui::Text("Resolution:");
        const char* resolutions[] = {
            "1280x720 (HD)",
            "1920x1080 (Full HD)",
            "2560x1440 (2K)",
            "3840x2160 (4K)"
        };
        static int currentRes = 1;
        if (ImGui::Combo("##Resolution", &currentRes, resolutions, IM_ARRAYSIZE(resolutions))) {
            switch (currentRes) {
                case 0: state.renderWidth = 1280; state.renderHeight = 720; break;
                case 1: state.renderWidth = 1920; state.renderHeight = 1080; break;
                case 2: state.renderWidth = 2560; state.renderHeight = 1440; break;
                case 3: state.renderWidth = 3840; state.renderHeight = 2160; break;
            }
            changed = true;
        }

        ImGui::Spacing();
        ImGui::Checkbox("Enable HDR", &state.enableHDR);
        ImGui::Checkbox("Enable Bloom", &state.enableBloom);

        if (changed && parameterChangeCallback) {
            parameterChangeCallback();
        }
    }
    ImGui::End();
}

void ImGuiInterface::renderCameraControls() {
    ImGui::SetNextWindowPos(ImVec2(340, 290), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Camera Settings:");
        ImGui::Separator();

        float distanceLog = std::log10(state.cameraDistance);
        if (ImGui::SliderFloat("Distance", &distanceLog, 9.0f, 12.0f, "10^%.1f m")) {
            state.cameraDistance = std::pow(10.0f, distanceLog);
        }

        ImGui::SliderFloat("Field of View", &state.cameraFOV, 30.0f, 120.0f, "%.0f°");
        ImGui::SliderFloat("Movement Speed", &state.cameraSpeed, 0.1f, 5.0f, "%.1fx");

        ImGui::Spacing();
        ImGui::Text("Animation:");
        ImGui::Checkbox("Auto-rotate", &state.animate);
        if (state.animate) {
            ImGui::SliderFloat("Speed##anim", &state.animationSpeed, 0.1f, 5.0f, "%.1fx");
        }
    }
    ImGui::End();
}

void ImGuiInterface::renderVisualizationOptions() {
    ImGui::SetNextWindowPos(ImVec2(650, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 200), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Visualization", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Show/Hide Elements:");
        ImGui::Separator();

        ImGui::Checkbox("Spacetime Grid", &state.showGrid);
        ImGui::Checkbox("Event Horizon", &state.showEventHorizon);
        ImGui::Checkbox("Photon Sphere", &state.showPhotonSphere);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("The photon sphere is at r = 1.5 r_s\nwhere photons can orbit the black hole");
        }

        ImGui::Spacing();
        ImGui::Text("Information Display:");
        ImGui::Checkbox("Show Info Panel", &state.showInformation);
        ImGui::Checkbox("Show Performance", &state.showPerformance);
    }
    ImGui::End();
}

void ImGuiInterface::renderPerformanceMonitor() {
    ImGui::SetNextWindowPos(ImVec2(10, 480), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 120), ImGuiCond_FirstUseEver);

    ImGui::SetNextWindowBgAlpha(0.8f);
    if (ImGui::Begin("Performance", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::Text("Performance Monitor");
        ImGui::Separator();

        // Color code FPS
        if (state.fps >= 55.0f) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "FPS: %.1f", state.fps);
        } else if (state.fps >= 30.0f) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FPS: %.1f", state.fps);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FPS: %.1f", state.fps);
        }

        ImGui::Text("Frame Time: %.2f ms", state.frameTime * 1000.0f);
        ImGui::Text("Ray Steps: %d", state.raySteps);
        ImGui::Text("Resolution: %dx%d", state.renderWidth, state.renderHeight);
    }
    ImGui::End();
}

void ImGuiInterface::renderInformationPanel() {
    ImGui::SetNextWindowPos(ImVec2(650, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 280), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Physical Information", &state.showInformation, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Black Hole Properties:");
        ImGui::Separator();

        float solarMasses = state.blackHoleMass / 1.989e30f;
        ImGui::Text("Mass: %.2e kg (%.1f M☉)", state.blackHoleMass, solarMasses);
        ImGui::Text("Spin: %.3f", state.blackHoleSpin);
        ImGui::Text("Schwarzschild Radius: %.2e m", state.schwarzschildRadius);

        // Calculate derived quantities
        float photonSphereRadius = 1.5f * state.schwarzschildRadius;
        float iscoRadius = (state.blackHoleSpin < 0.001f) ?
            3.0f * state.schwarzschildRadius :
            3.0f * state.schwarzschildRadius; // Simplified

        ImGui::Text("Photon Sphere: %.2e m (1.5 r_s)", photonSphereRadius);
        ImGui::Text("ISCO: %.2e m (%.1f r_s)",
                   iscoRadius, iscoRadius / state.schwarzschildRadius);

        ImGui::Spacing();
        ImGui::Text("Accretion Disk:");
        ImGui::Separator();
        if (state.showDisk) {
            float innerM = state.diskInnerRadius * state.schwarzschildRadius;
            float outerM = state.diskOuterRadius * state.schwarzschildRadius;
            ImGui::Text("Inner: %.2e m", innerM);
            ImGui::Text("Outer: %.2e m", outerM);
            ImGui::Text("Temperature Factor: %.2fx", state.diskTemperature);

            // Estimate temperatures
            float innerTemp = 1.0e7f * state.diskTemperature;
            float outerTemp = 1.0e4f * state.diskTemperature;
            ImGui::Text("Inner Temp: ~%.1e K", innerTemp);
            ImGui::Text("Outer Temp: ~%.1e K", outerTemp);
        } else {
            ImGui::TextDisabled("(Disk hidden)");
        }

        ImGui::Spacing();
        ImGui::Text("Controls:");
        ImGui::Separator();
        ImGui::BulletText("Left Mouse: Rotate camera");
        ImGui::BulletText("Scroll: Zoom in/out");
        ImGui::BulletText("G: Toggle gravity");
    }
    ImGui::End();
}

void ImGuiInterface::renderHelpPanel() {
    ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Help", &state.showHelp, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Black Hole Simulator - Help");
        ImGui::Separator();

        ImGui::Text("Controls:");
        ImGui::BulletText("Left Mouse Drag: Rotate camera around black hole");
        ImGui::BulletText("Mouse Wheel: Zoom in/out");
        ImGui::BulletText("Right Mouse: Toggle gravity simulation");
        ImGui::BulletText("G: Toggle gravity on/off");
        ImGui::BulletText("ESC: Exit program");
        ImGui::BulletText("F1: Toggle this help panel");

        ImGui::Spacing();
        ImGui::Text("Scene Presets:");
        ImGui::BulletText("Sagittarius A*: Realistic view of our galaxy's black hole");
        ImGui::BulletText("Accretion Disk: Close-up of the disk with Doppler effects");
        ImGui::BulletText("Rotating BH: Kerr black hole with frame dragging");
        ImGui::BulletText("Binary Merger: Two black holes spiraling together");

        ImGui::Spacing();
        ImGui::Text("Physics Features:");
        ImGui::BulletText("Geodesic ray tracing in curved spacetime");
        ImGui::BulletText("Doppler shift: Blue=approaching, Red=receding");
        ImGui::BulletText("Gravitational redshift near event horizon");
        ImGui::BulletText("Relativistic beaming (intensity boost)");
        ImGui::BulletText("Physically accurate temperature gradients");

        ImGui::Spacing();
        ImGui::Text("Performance Tips:");
        ImGui::BulletText("Lower ray steps if FPS is too low");
        ImGui::BulletText("Use Quality presets for quick adjustment");
        ImGui::BulletText("Reduce resolution on slower GPUs");
        ImGui::BulletText("Disable HDR/Bloom for better performance");

        ImGui::Spacing();
        if (ImGui::Button("Close Help", ImVec2(120, 0))) {
            state.showHelp = false;
        }
    }
    ImGui::End();
}

// ========== Helper Functions ==========

void ImGuiInterface::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Dark theme
    ImGui::StyleColorsDark();

    // Customize colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);

    // Rounded corners
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
}

const char* ImGuiInterface::scenePresetName(ScenePreset preset) const {
    switch (preset) {
        case ScenePreset::SAGITTARIUS_A_STATIC: return "Sagittarius A*";
        case ScenePreset::ACCRETION_DISK_CLOSEUP: return "Accretion Disk";
        case ScenePreset::ROTATING_BLACKHOLE: return "Rotating Black Hole";
        case ScenePreset::BINARY_MERGER: return "Binary Merger";
        case ScenePreset::PHOTON_RING: return "Photon Ring";
        case ScenePreset::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

const char* ImGuiInterface::qualityName(RenderQuality quality) const {
    switch (quality) {
        case RenderQuality::LOW: return "Low";
        case RenderQuality::MEDIUM: return "Medium";
        case RenderQuality::HIGH: return "High";
        case RenderQuality::ULTRA: return "Ultra";
        default: return "Unknown";
    }
}

void ImGuiInterface::applyQualityPreset(RenderQuality quality) {
    switch (quality) {
        case RenderQuality::LOW:
            state.raySteps = 30000;
            state.renderWidth = 1280;
            state.renderHeight = 720;
            state.enableHDR = false;
            state.enableBloom = false;
            break;

        case RenderQuality::MEDIUM:
            state.raySteps = 60000;
            state.renderWidth = 1920;
            state.renderHeight = 1080;
            state.enableHDR = false;
            state.enableBloom = false;
            break;

        case RenderQuality::HIGH:
            state.raySteps = 80000;
            state.renderWidth = 1920;
            state.renderHeight = 1080;
            state.enableHDR = true;
            state.enableBloom = false;
            break;

        case RenderQuality::ULTRA:
            state.raySteps = 120000;
            state.renderWidth = 2560;
            state.renderHeight = 1440;
            state.enableHDR = true;
            state.enableBloom = true;
            break;
    }
}

void ImGuiInterface::applyScenePreset(ScenePreset preset) {
    state.currentScene = preset;

    switch (preset) {
        case ScenePreset::SAGITTARIUS_A_STATIC:
            state.blackHoleMass = 8.54e36f;  // Sgr A* mass
            state.blackHoleSpin = 0.0f;
            state.cameraDistance = 6.34e10f;
            state.diskInnerRadius = 3.0f;
            state.diskOuterRadius = 10.0f;
            state.showDisk = true;
            state.showGrid = true;
            break;

        case ScenePreset::ACCRETION_DISK_CLOSEUP:
            state.blackHoleMass = 8.54e36f;
            state.blackHoleSpin = 0.0f;
            state.cameraDistance = 3.0e10f;  // Closer
            state.diskInnerRadius = 2.5f;
            state.diskOuterRadius = 8.0f;
            state.showDisk = true;
            state.showGrid = false;
            break;

        case ScenePreset::ROTATING_BLACKHOLE:
            state.blackHoleMass = 8.54e36f;
            state.blackHoleSpin = 0.9f;  // High spin
            state.cameraDistance = 5.0e10f;
            state.diskInnerRadius = 1.8f;  // Closer ISCO for spinning BH
            state.diskOuterRadius = 12.0f;
            state.showDisk = true;
            state.showGrid = true;
            break;

        case ScenePreset::BINARY_MERGER:
            state.blackHoleMass = 3.0e36f;  // Smaller masses
            state.blackHoleSpin = 0.5f;
            state.cameraDistance = 8.0e10f;  // Further out
            state.showDisk = false;
            state.showGrid = true;
            break;

        case ScenePreset::PHOTON_RING:
            state.blackHoleMass = 8.54e36f;
            state.blackHoleSpin = 0.0f;
            state.cameraDistance = 2.5e10f;  // Very close
            state.diskInnerRadius = 1.5f;
            state.diskOuterRadius = 3.0f;
            state.showDisk = true;
            state.showGrid = false;
            state.showPhotonSphere = true;
            break;

        case ScenePreset::CUSTOM:
            // Don't change anything
            break;
    }
}

} // namespace BlackHoleSim
