#include "window.h"
#include "renderer.h"

class Engine {
public:

    void run() {

        Window window(
            800,
            600,
            "Franklin"
        );

        Renderer renderer(window);

        renderer.init();

        while (!window.shouldClose()) {

            window.pollEvents();

            renderer.drawFrame();
        }
    }
};

int main() {

    Engine engine;

    engine.run();

    return 0;
}