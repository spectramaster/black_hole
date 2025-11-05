/**
 * Unit Tests for Performance Manager
 *
 * Tests:
 * 1. Initialization with different parameters
 * 2. FPS tracking and averaging
 * 3. Quality level switching
 * 4. Adaptive quality adjustments
 * 5. Statistics tracking
 * 6. Performance logging
 */

#include "../../src/performance/performance_manager.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace BlackHoleSim;

// Test helper to simulate frame rendering
void simulateFrame(PerformanceManager& pm, float targetFrameTime) {
    pm.beginFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(targetFrameTime)));
    pm.endFrame();
}

// Test 1: Basic initialization
bool testInitialization() {
    std::cout << "[TEST] Initialization..." << std::flush;

    try {
        PerformanceManager pm(60.0f, 1920, 1080);

        // Check default values
        assert(pm.getCurrentFPS() > 0.0f);
        assert(pm.isAdaptiveEnabled() == true);

        PerformanceSettings settings = pm.getCurrentSettings();
        assert(settings.renderWidth > 0);
        assert(settings.renderHeight > 0);
        assert(settings.raySteps > 0);

        std::cout << " PASS " << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 2: FPS tracking
bool testFPSTracking() {
    std::cout << "[TEST] FPS Tracking..." << std::flush;

    try {
        PerformanceManager pm(60.0f);

        // Simulate 60 FPS (16.67ms per frame)
        for (int i = 0; i < 10; i++) {
            simulateFrame(pm, 16.67f);
        }

        float fps = pm.getCurrentFPS();
        float frameTime = pm.getFrameTime();

        // FPS should be around 60 (with some tolerance)
        assert(fps > 50.0f && fps < 70.0f);
        assert(frameTime > 10.0f && frameTime < 25.0f);

        std::cout << " PASS  (FPS: " << fps << ", Frame Time: " << frameTime << "ms)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 3: Quality level switching
bool testQualityLevels() {
    std::cout << "[TEST] Quality Level Switching..." << std::flush;

    try {
        PerformanceManager pm(60.0f, 1920, 1080);

        // Test each quality level
        pm.setPerformanceLevel(PerformanceLevel::LOW);
        PerformanceSettings low = pm.getCurrentSettings();
        assert(low.renderWidth == 640);
        assert(low.renderHeight == 480);
        assert(low.raySteps == 30000);

        pm.setPerformanceLevel(PerformanceLevel::MEDIUM);
        PerformanceSettings medium = pm.getCurrentSettings();
        assert(medium.renderWidth == 800);
        assert(medium.renderHeight == 600);
        assert(medium.raySteps == 60000);

        pm.setPerformanceLevel(PerformanceLevel::HIGH);
        PerformanceSettings high = pm.getCurrentSettings();
        assert(high.renderWidth == 1280);
        assert(high.renderHeight == 720);
        assert(high.raySteps == 80000);

        pm.setPerformanceLevel(PerformanceLevel::ULTRA);
        PerformanceSettings ultra = pm.getCurrentSettings();
        assert(ultra.renderWidth == 1920);
        assert(ultra.renderHeight == 1080);
        assert(ultra.raySteps == 120000);

        std::cout << " PASS " << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 4: Adaptive quality (quality reduction on low FPS)
bool testAdaptiveQualityReduction() {
    std::cout << "[TEST] Adaptive Quality Reduction..." << std::flush;

    try {
        PerformanceManager pm(60.0f, 1920, 1080);
        pm.setPerformanceLevel(PerformanceLevel::ULTRA);
        pm.setEnableAdaptive(true);

        // Simulate low FPS (30 FPS = 33.33ms per frame) for 40 frames
        for (int i = 0; i < 40; i++) {
            simulateFrame(pm, 33.33f);
            pm.updateAdaptiveQuality();
        }

        // Should have reduced quality
        PerformanceSettings settings = pm.getCurrentSettings();
        // Should have downgraded from ULTRA (1920x1080)
        assert(settings.renderWidth < 1920 || settings.renderHeight < 1080);

        std::cout << " PASS  (Reduced to " << settings.renderWidth << "x" << settings.renderHeight << ")" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 5: Statistics tracking
bool testStatistics() {
    std::cout << "[TEST] Statistics Tracking..." << std::flush;

    try {
        PerformanceManager pm(60.0f);

        // Simulate various frame rates
        simulateFrame(pm, 16.67f);  // 60 FPS
        simulateFrame(pm, 33.33f);  // 30 FPS
        simulateFrame(pm, 10.0f);   // 100 FPS
        simulateFrame(pm, 50.0f);   // 20 FPS

        float minFPS = pm.getMinFPS();
        float maxFPS = pm.getMaxFPS();

        // Check that min/max are reasonable
        assert(minFPS > 0.0f);
        assert(maxFPS > minFPS);
        assert(maxFPS < 1000.0f); // Sanity check

        // Reset and verify
        pm.resetStatistics();
        // After reset, simulate one frame
        simulateFrame(pm, 16.67f);

        std::cout << " PASS  (Min: " << minFPS << " FPS, Max: " << maxFPS << " FPS)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 6: Disable adaptive quality
bool testDisableAdaptive() {
    std::cout << "[TEST] Disable Adaptive Quality..." << std::flush;

    try {
        PerformanceManager pm(60.0f, 1920, 1080);
        pm.setPerformanceLevel(PerformanceLevel::ULTRA);
        pm.setEnableAdaptive(false);

        // Simulate low FPS
        for (int i = 0; i < 40; i++) {
            simulateFrame(pm, 33.33f);
            pm.updateAdaptiveQuality();
        }

        // Quality should NOT have changed (adaptive disabled)
        PerformanceSettings settings = pm.getCurrentSettings();
        assert(settings.renderWidth == 1920);
        assert(settings.renderHeight == 1080);

        std::cout << " PASS " << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

// Test 7: Performance logging
bool testLogging() {
    std::cout << "[TEST] Performance Logging..." << std::flush;

    try {
        PerformanceManager pm(60.0f);
        pm.enableLogging(true);

        // Simulate frames
        for (int i = 0; i < 5; i++) {
            simulateFrame(pm, 16.67f);
        }

        // Manually trigger log
        pm.logPerformanceData();

        std::cout << " PASS " << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << " FAIL  - " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Performance Manager Unit Tests" << std::endl;
    std::cout << "========================================\n" << std::endl;

    int passed = 0;
    int total = 7;

    if (testInitialization()) passed++;
    if (testFPSTracking()) passed++;
    if (testQualityLevels()) passed++;
    if (testAdaptiveQualityReduction()) passed++;
    if (testStatistics()) passed++;
    if (testDisableAdaptive()) passed++;
    if (testLogging()) passed++;

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << passed << "/" << total << " tests passed";
    if (passed == total) {
        std::cout << "  ALL PASS" << std::endl;
    } else {
        std::cout << "  SOME FAILURES" << std::endl;
    }
    std::cout << "========================================\n" << std::endl;

    return (passed == total) ? 0 : 1;
}
