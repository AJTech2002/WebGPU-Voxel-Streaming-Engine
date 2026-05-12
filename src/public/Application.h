#include "Engine.h"
#include "GLFW/glfw3.h"

class Application
{
public:
    Engine engine;
    GLFWwindow *window;
    void onInit();
    bool isRunning();
    void onFrame();
    void onFinish();
};
