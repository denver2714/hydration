#include <iostream>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#ifdef __EMSCRIPTEN__
#define GLFW_INCLUDE_NONE
#include <GLES3/gl3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#endif
#include <GLFW/glfw3.h>
#include "src/Simulation.h"
#include "src/Renderer.h"

static GLFWwindow* g_window   = nullptr;
static Simulation* g_sim      = nullptr;
static Renderer*   g_renderer = nullptr;
static bool   g_mouseDown = false;
static double g_mouseX = 0.0, g_mouseY = 0.0;
static int    g_winW = 800, g_winH = 600;
static double g_lastTime = 0.0;

// Same coordinate mapping as main.cpp
static glm::vec2 screenToSim(double sx, double sy) {
    float aspect  = (float)g_winW / (float)g_winH;
    float padding = 0.05f;
    float normX   = (float)sx / (float)g_winW;
    float normY   = 1.0f - (float)sy / (float)g_winH;
    float simX, simY;
    if (aspect >= 1.0f) {
        float halfW = (0.5f + padding) * aspect;
        float halfH =  0.5f + padding;
        simX = (normX - 0.5f) * 2.0f * halfW + 0.5f;
        simY = (normY - 0.5f) * 2.0f * halfH + 0.5f;
    } else {
        float halfW =  0.5f + padding;
        float halfH = (0.5f + padding) / aspect;
        simX = (normX - 0.5f) * 2.0f * halfW + 0.5f;
        simY = (normY - 0.5f) * 2.0f * halfH + 0.5f;
    }
    return glm::vec2(simX, simY);
}

// ----- Exported functions callable from JavaScript buttons -----
#ifdef __EMSCRIPTEN__
extern "C" {
    EMSCRIPTEN_KEEPALIVE void web_reset()         { if (g_sim) g_sim->reset(); }
    EMSCRIPTEN_KEEPALIVE void web_toggle_gravity() { if (g_sim) g_sim->toggleGravity(); }
}
#endif

// ----- Main loop (called by Emscripten or the while-loop on desktop) -----
static void mainLoop() {
    double now = glfwGetTime();
    float dt = std::min((float)(now - g_lastTime), 0.02f);
    g_lastTime = now;

    // Keep window size in sync with canvas (canvas may be resized by JS)
    glfwGetFramebufferSize(g_window, &g_winW, &g_winH);

    glm::vec2 cur = screenToSim(g_mouseX, g_mouseY);
    g_sim->applyCursorForce(cur.x, cur.y, g_mouseDown);
    g_sim->update(dt);

    glClear(GL_COLOR_BUFFER_BIT);
    g_renderer->render(*g_sim, g_winW, g_winH);

    glfwSwapBuffers(g_window);
    glfwPollEvents();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Request WebGL 2 / OpenGL ES 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(g_winW, g_winH, "Hydration Physics", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1);

    // ----- Input callbacks -----
    glfwSetKeyCallback(g_window, [](GLFWwindow* w, int key, int, int action, int) {
        if (action != GLFW_PRESS) return;
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, true);
        if (key == GLFW_KEY_SPACE && g_sim) g_sim->reset();
        if (key == GLFW_KEY_G     && g_sim) g_sim->toggleGravity();
    });
    glfwSetMouseButtonCallback(g_window, [](GLFWwindow*, int btn, int action, int) {
        if (btn == GLFW_MOUSE_BUTTON_LEFT)
            g_mouseDown = (action == GLFW_PRESS);
    });
    glfwSetCursorPosCallback(g_window, [](GLFWwindow*, double x, double y) {
        g_mouseX = x;
        g_mouseY = y;
    });

    glfwGetFramebufferSize(g_window, &g_winW, &g_winH);

    // ----- Simulation & Renderer -----
    static Simulation sim(2000);
    g_sim = &sim;

    static Renderer renderer;
    g_renderer = &renderer;

    if (!renderer.init("shaders")) {
        std::cerr << "Failed to init renderer\n";
        return -1;
    }

    g_lastTime = glfwGetTime();

#ifdef __EMSCRIPTEN__
    // 0 = use browser's requestAnimationFrame rate; 1 = simulate infinite loop
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (!glfwWindowShouldClose(g_window)) mainLoop();
    glfwDestroyWindow(g_window);
    glfwTerminate();
#endif
    return 0;
}
