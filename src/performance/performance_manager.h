/**
 * Performance Manager for Black Hole Simulator
 *
 * Provides adaptive quality adjustment to maintain target FPS
 * Features:
 * - Dynamic resolution scaling
 * - Ray step adjustment
 * - Quality preset management
 * - Frame time profiling
 */

#pragma once

#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace BlackHoleSim {

enum class PerformanceLevel {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA,
    CUSTOM
};

struct PerformanceSettings {
    int renderWidth;
    int renderHeight;
    int raySteps;
    bool enableHDR;
    bool enableBloom;
    bool enableAdvancedShading;
};

class PerformanceManager {
public:
    PerformanceManager(float targetFPS = 60.0f, int windowWidth = 1920, int windowHeight = 1080);

    // Frame timing
    void beginFrame();
    void endFrame();

    // FPS tracking
    float getCurrentFPS() const { return currentFPS; }
    float getAverageFPS() const { return averageFPS; }
    float getFrameTime() const { return frameTime; } // in milliseconds

    // Adaptive quality
    void updateAdaptiveQuality();
    bool shouldReduceQuality() const;
    bool shouldIncreaseQuality() const;

    // Quality settings
    void setPerformanceLevel(PerformanceLevel level);
    PerformanceSettings getCurrentSettings() const { return currentSettings; }

    // Manual overrides
    void setTargetFPS(float fps) { targetFPS = fps; }
    void setEnableAdaptive(bool enable) { adaptiveEnabled = enable; }
    bool isAdaptiveEnabled() const { return adaptiveEnabled; }

    // Statistics
    float getMinFPS() const { return minFPS; }
    float getMaxFPS() const { return maxFPS; }
    void resetStatistics();

    // Logging
    void enableLogging(bool enable) { loggingEnabled = enable; }
    void logPerformanceData();

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    // Timing
    TimePoint frameStartTime;
    TimePoint lastUpdateTime;
    std::vector<float> frameTimes; // Ring buffer
    int frameTimeIndex;
    static constexpr int FRAME_TIME_BUFFER_SIZE = 60;

    // FPS tracking
    float currentFPS;
    float averageFPS;
    float frameTime;
    float targetFPS;
    int frameCount;

    // Statistics
    float minFPS;
    float maxFPS;
    float totalFrameTime;

    // Adaptive quality
    bool adaptiveEnabled;
    PerformanceLevel currentLevel;
    PerformanceSettings currentSettings;
    int stableFrameCount; // Frames at stable FPS
    int poorFrameCount;   // Frames below target

    // Window dimensions
    int windowWidth;
    int windowHeight;

    // Logging
    bool loggingEnabled;
    int logCounter;

    // Internal helpers
    void calculateFPS();
    void applyPerformanceLevel(PerformanceLevel level);
    PerformanceSettings getSettingsForLevel(PerformanceLevel level) const;
};

} // namespace BlackHoleSim
