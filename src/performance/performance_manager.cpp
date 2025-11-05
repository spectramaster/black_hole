/**
 * Performance Manager Implementation
 */

#include "performance_manager.h"
#include <cmath>
#include <iomanip>

namespace BlackHoleSim {

PerformanceManager::PerformanceManager(float targetFPS, int windowWidth, int windowHeight)
    : targetFPS(targetFPS)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , currentFPS(60.0f)
    , averageFPS(60.0f)
    , frameTime(16.67f)
    , frameCount(0)
    , minFPS(1000.0f)
    , maxFPS(0.0f)
    , totalFrameTime(0.0f)
    , adaptiveEnabled(true)
    , currentLevel(PerformanceLevel::MEDIUM)
    , stableFrameCount(0)
    , poorFrameCount(0)
    , frameTimeIndex(0)
    , loggingEnabled(false)
    , logCounter(0)
{
    frameTimes.resize(FRAME_TIME_BUFFER_SIZE, 16.67f);
    lastUpdateTime = Clock::now();
    applyPerformanceLevel(PerformanceLevel::MEDIUM);
}

void PerformanceManager::beginFrame() {
    frameStartTime = Clock::now();
}

void PerformanceManager::endFrame() {
    auto frameEndTime = Clock::now();
    std::chrono::duration<float, std::milli> duration = frameEndTime - frameStartTime;
    frameTime = duration.count();

    // Store in ring buffer
    frameTimes[frameTimeIndex] = frameTime;
    frameTimeIndex = (frameTimeIndex + 1) % FRAME_TIME_BUFFER_SIZE;

    // Calculate FPS
    currentFPS = (frameTime > 0.0f) ? (1000.0f / frameTime) : 0.0f;
    frameCount++;

    // Update statistics
    if (currentFPS < minFPS) minFPS = currentFPS;
    if (currentFPS > maxFPS) maxFPS = currentFPS;
    totalFrameTime += frameTime;

    calculateFPS();

    // Log if enabled
    if (loggingEnabled && ++logCounter % 60 == 0) {
        logPerformanceData();
    }
}

void PerformanceManager::calculateFPS() {
    // Calculate average from ring buffer
    float sum = 0.0f;
    for (float ft : frameTimes) {
        sum += ft;
    }
    float avgFrameTime = sum / frameTimes.size();
    averageFPS = (avgFrameTime > 0.0f) ? (1000.0f / avgFrameTime) : 0.0f;
}

void PerformanceManager::updateAdaptiveQuality() {
    if (!adaptiveEnabled) {
        return;
    }

    float fpsThreshold = targetFPS * 0.85f; // 15% tolerance below target
    float fpsUpperBound = targetFPS * 1.05f; // 5% tolerance above target

    if (averageFPS < fpsThreshold) {
        poorFrameCount++;
        stableFrameCount = 0;

        // Reduce quality after 30 consecutive poor frames (~0.5 seconds)
        if (poorFrameCount >= 30 && currentLevel != PerformanceLevel::LOW) {
            std::cout << "[PERF] FPS below target (" << averageFPS << " < " << fpsThreshold
                     << "), reducing quality..." << std::endl;

            switch (currentLevel) {
                case PerformanceLevel::ULTRA:
                    setPerformanceLevel(PerformanceLevel::HIGH);
                    break;
                case PerformanceLevel::HIGH:
                    setPerformanceLevel(PerformanceLevel::MEDIUM);
                    break;
                case PerformanceLevel::MEDIUM:
                    setPerformanceLevel(PerformanceLevel::LOW);
                    break;
                default:
                    break;
            }
            poorFrameCount = 0;
        }
    } else if (averageFPS > fpsUpperBound) {
        stableFrameCount++;
        poorFrameCount = 0;

        // Increase quality after 120 stable frames (~2 seconds)
        if (stableFrameCount >= 120 && currentLevel != PerformanceLevel::ULTRA) {
            std::cout << "[PERF] FPS stable above target (" << averageFPS << " > " << targetFPS
                     << "), increasing quality..." << std::endl;

            switch (currentLevel) {
                case PerformanceLevel::LOW:
                    setPerformanceLevel(PerformanceLevel::MEDIUM);
                    break;
                case PerformanceLevel::MEDIUM:
                    setPerformanceLevel(PerformanceLevel::HIGH);
                    break;
                case PerformanceLevel::HIGH:
                    setPerformanceLevel(PerformanceLevel::ULTRA);
                    break;
                default:
                    break;
            }
            stableFrameCount = 0;
        }
    } else {
        // FPS within acceptable range
        poorFrameCount = std::max(0, poorFrameCount - 1);
        stableFrameCount = std::max(0, stableFrameCount - 1);
    }
}

bool PerformanceManager::shouldReduceQuality() const {
    return averageFPS < targetFPS * 0.85f;
}

bool PerformanceManager::shouldIncreaseQuality() const {
    return averageFPS > targetFPS * 1.05f && stableFrameCount > 120;
}

void PerformanceManager::setPerformanceLevel(PerformanceLevel level) {
    currentLevel = level;
    applyPerformanceLevel(level);

    std::cout << "[PERF] Performance level set to: ";
    switch (level) {
        case PerformanceLevel::LOW: std::cout << "LOW"; break;
        case PerformanceLevel::MEDIUM: std::cout << "MEDIUM"; break;
        case PerformanceLevel::HIGH: std::cout << "HIGH"; break;
        case PerformanceLevel::ULTRA: std::cout << "ULTRA"; break;
        case PerformanceLevel::CUSTOM: std::cout << "CUSTOM"; break;
    }
    std::cout << " (Width: " << currentSettings.renderWidth
              << ", Height: " << currentSettings.renderHeight
              << ", Ray Steps: " << currentSettings.raySteps << ")" << std::endl;
}

void PerformanceManager::applyPerformanceLevel(PerformanceLevel level) {
    currentSettings = getSettingsForLevel(level);
}

PerformanceSettings PerformanceManager::getSettingsForLevel(PerformanceLevel level) const {
    PerformanceSettings settings;

    switch (level) {
        case PerformanceLevel::LOW:
            settings.renderWidth = 640;
            settings.renderHeight = 480;
            settings.raySteps = 30000;
            settings.enableHDR = false;
            settings.enableBloom = false;
            settings.enableAdvancedShading = false;
            break;

        case PerformanceLevel::MEDIUM:
            settings.renderWidth = 800;
            settings.renderHeight = 600;
            settings.raySteps = 60000;
            settings.enableHDR = false;
            settings.enableBloom = false;
            settings.enableAdvancedShading = true;
            break;

        case PerformanceLevel::HIGH:
            settings.renderWidth = 1280;
            settings.renderHeight = 720;
            settings.raySteps = 80000;
            settings.enableHDR = true;
            settings.enableBloom = false;
            settings.enableAdvancedShading = true;
            break;

        case PerformanceLevel::ULTRA:
            settings.renderWidth = 1920;
            settings.renderHeight = 1080;
            settings.raySteps = 120000;
            settings.enableHDR = true;
            settings.enableBloom = true;
            settings.enableAdvancedShading = true;
            break;

        case PerformanceLevel::CUSTOM:
            // Keep current settings
            settings = currentSettings;
            break;
    }

    return settings;
}

void PerformanceManager::resetStatistics() {
    minFPS = 1000.0f;
    maxFPS = 0.0f;
    totalFrameTime = 0.0f;
    frameCount = 0;
    stableFrameCount = 0;
    poorFrameCount = 0;

    std::cout << "[PERF] Statistics reset" << std::endl;
}

void PerformanceManager::logPerformanceData() {
    float avgFrameTime = (frameCount > 0) ? (totalFrameTime / frameCount) : 0.0f;

    std::cout << "[PERF] ================================================" << std::endl;
    std::cout << "[PERF] Performance Statistics:" << std::endl;
    std::cout << "[PERF]   Current FPS: " << std::fixed << std::setprecision(2) << currentFPS << std::endl;
    std::cout << "[PERF]   Average FPS: " << averageFPS << std::endl;
    std::cout << "[PERF]   Min FPS: " << minFPS << std::endl;
    std::cout << "[PERF]   Max FPS: " << maxFPS << std::endl;
    std::cout << "[PERF]   Frame Time: " << frameTime << " ms" << std::endl;
    std::cout << "[PERF]   Avg Frame Time: " << avgFrameTime << " ms" << std::endl;
    std::cout << "[PERF]   Resolution: " << currentSettings.renderWidth << "x" << currentSettings.renderHeight << std::endl;
    std::cout << "[PERF]   Ray Steps: " << currentSettings.raySteps << std::endl;
    std::cout << "[PERF]   HDR: " << (currentSettings.enableHDR ? "ON" : "OFF") << std::endl;
    std::cout << "[PERF]   Bloom: " << (currentSettings.enableBloom ? "ON" : "OFF") << std::endl;
    std::cout << "[PERF]   Advanced Shading: " << (currentSettings.enableAdvancedShading ? "ON" : "OFF") << std::endl;
    std::cout << "[PERF]   Adaptive Quality: " << (adaptiveEnabled ? "ENABLED" : "DISABLED") << std::endl;
    std::cout << "[PERF] ================================================" << std::endl;
}

} // namespace BlackHoleSim
