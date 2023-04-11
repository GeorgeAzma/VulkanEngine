#include "silk_engine/gfx/window/glfw.h"
#include "silk_engine/core/input/input.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/scene/resources.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/renderer.h"
#include "silk_engine/scene/scene_manager.h"
#include "silk_engine/core/event.h"
#include "silk_engine/core/input/keys.h"

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp()
{
    Input::init();
    GLFW::init();
    RenderContext::init("MyApp");
    Resources::init();

    window = new Window();
    Renderer::init();

    scene = makeShared<MyScene>();
    SceneManager::add(scene);

    Dispatcher::subscribe(*this, &MyApp::onKeyPress);
    Dispatcher::subscribe(*this, &MyApp::onWindowClose);
    Dispatcher::subscribe(*this, &MyApp::onFramebufferResize);
}

MyApp::~MyApp()
{
    Dispatcher::unsubscribe(*this, &MyApp::onKeyPress);
    Dispatcher::unsubscribe(*this, &MyApp::onWindowClose);
    Dispatcher::unsubscribe(*this, &MyApp::onFramebufferResize);

    SceneManager::destroy();
    Renderer::destroy();
    Resources::destroy();
    delete window;
    RenderContext::destroy();
    GLFW::destroy();
    SK_INFO("Terminated");
}

void MyApp::onUpdate()
{
}

void MyApp::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Keys::ESCAPE:
        Dispatcher::post(WindowCloseEvent(e.window));
        break;
    case Keys::F11:
        e.window.setFullscreen(!e.window.isFullscreen());
        break;
    }
}

void MyApp::onWindowClose(const WindowCloseEvent& e)
{
    running = false;
}

void MyApp::onFramebufferResize(const FramebufferResizeEvent& e)
{
    if (Window::getActive().isMinimized())
        return;
    update();
    update();
}