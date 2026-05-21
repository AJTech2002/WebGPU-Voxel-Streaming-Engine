#include "app/scenes/VoxelScene.h"
#include "engine/Application.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

int main(int, char **)
{
    Application app;
    VoxelScene voxelScene(&app.engine);
    app.onInit();
    app.engine.loadScene(&voxelScene);

#ifdef __EMSCRIPTEN__

    emscripten_set_main_loop_arg(
        [](void *userData)
        {
            Application &app = *reinterpret_cast<Application *>(userData);
            app.onFrame();
        },
        (void *)&app, 0, true);

#else // __EMSCRIPTEN__

    while (app.isRunning())
    {
        app.onFrame();
    }
    app.onFinish();

#endif // __EMSCRIPTEN__

    return 0;
}