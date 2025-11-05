/**
 * Unit Test: Shader Syntax Validation
 *
 * Validates shader files without needing OpenGL context
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

std::vector<TestResult> results;

std::string loadFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void testShaderSyntax(const char* name, const char* path, const char* expectedVersion) {
    std::string content = loadFile(path);

    if (content.empty()) {
        results.push_back({name, false, "File not found: " + std::string(path)});
        return;
    }

    // Check version directive
    bool hasVersion = content.find(expectedVersion) != std::string::npos;
    if (!hasVersion) {
        results.push_back({name, false, "Missing or incorrect version directive"});
        return;
    }

    // Basic syntax checks
    int braceBalance = 0;
    int parenBalance = 0;
    for (char c : content) {
        if (c == '{') braceBalance++;
        if (c == '}') braceBalance--;
        if (c == '(') parenBalance++;
        if (c == ')') parenBalance--;
    }

    if (braceBalance != 0) {
        results.push_back({name, false, "Unbalanced braces"});
        return;
    }

    if (parenBalance != 0) {
        results.push_back({name, false, "Unbalanced parentheses"});
        return;
    }

    results.push_back({name, true, "Syntax OK"});
}

void testComputeShaderBindings(const char* name, const char* path) {
    std::string content = loadFile(path);

    if (content.empty()) {
        results.push_back({std::string(name) + " Bindings", false, "File not found"});
        return;
    }

    // Check for required bindings
    bool hasImageBinding = content.find("binding = 0") != std::string::npos;
    bool hasCameraBinding = content.find("binding = 1") != std::string::npos;
    bool hasDiskBinding = content.find("binding = 2") != std::string::npos;
    bool hasObjectsBinding = content.find("binding = 3") != std::string::npos;

    if (hasImageBinding && hasCameraBinding && hasDiskBinding && hasObjectsBinding) {
        results.push_back({std::string(name) + " Bindings", true, "All bindings present"});
    } else {
        std::string missing = "Missing: ";
        if (!hasImageBinding) missing += "image ";
        if (!hasCameraBinding) missing += "camera ";
        if (!hasDiskBinding) missing += "disk ";
        if (!hasObjectsBinding) missing += "objects ";
        results.push_back({std::string(name) + " Bindings", false, missing});
    }
}

void testShaderConstants(const char* name, const char* path) {
    std::string content = loadFile(path);

    if (content.empty()) {
        results.push_back({std::string(name) + " Constants", false, "File not found"});
        return;
    }

    // Check for either explicit c/G constants OR pre-computed values (like SagA_rs)
    bool hasConstants = (content.find("const float c") != std::string::npos || 
                        content.find("const double c") != std::string::npos ||
                        content.find("const float G") != std::string::npos ||
                        content.find("const double G") != std::string::npos ||
                        content.find("const float SagA_rs") != std::string::npos ||
                        content.find("const float r_s") != std::string::npos);

    if (hasConstants) {
        results.push_back({std::string(name) + " Constants", true, "Physical constants defined"});
    } else {
        results.push_back({std::string(name) + " Constants", false, "No physical constants found"});
    }
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Shader Syntax Validation Tests" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Test shader syntax
    #ifdef __APPLE__
        const char* expectedVersion = "#version 410";
    #else
        const char* expectedVersion = "#version 410";
    #endif

    testShaderSyntax("geodesic.comp", "../geodesic.comp", expectedVersion);
    testShaderSyntax("kerr_black_hole.comp", "../shaders/kerr_black_hole.comp", expectedVersion);
    testShaderSyntax("accretion_disk_advanced.comp", "../shaders/accretion_disk_advanced.comp", expectedVersion);
    testShaderSyntax("grid.vert", "../grid.vert", "#version 330");
    testShaderSyntax("grid.frag", "../grid.frag", "#version 330");

    // Test bindings
    testComputeShaderBindings("geodesic.comp", "../geodesic.comp");
    testComputeShaderBindings("kerr_black_hole.comp", "../shaders/kerr_black_hole.comp");

    // Test constants
    testShaderConstants("geodesic.comp", "../geodesic.comp");
    testShaderConstants("kerr_black_hole.comp", "../shaders/kerr_black_hole.comp");

    // Print results
    int passed = 0;
    for (const auto& r : results) {
        std::cout << "[" << (r.passed ? "PASS" : "FAIL") << "] " << r.name;
        if (!r.passed) {
            std::cout << "\n       " << r.message;
        }
        std::cout << std::endl;

        if (r.passed) passed++;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << passed << "/" << results.size() << " passed";
    std::cout << (passed == results.size() ? " ✓ ALL PASS" : " ✗ SOME FAILURES") << std::endl;
    std::cout << "========================================\n" << std::endl;

    return (passed == results.size()) ? 0 : 1;
}
