// Ejercicio 2: Abstract Factory
#include <iostream>
#include <memory>
using namespace std;

class Window {
public:
    virtual ~Window() = default;
    virtual void draw() = 0;
};

class Button {
public:
    virtual ~Button() = default;
    virtual void draw() = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile() = 0;
};


class RenderFactory {
public:
    virtual ~RenderFactory() = default;
    virtual unique_ptr<Window> createWindow() = 0;
    virtual unique_ptr<Button> createButton() = 0;
    virtual unique_ptr<Shader> createShader() = 0;
};


class OpenGLWindow : public Window {
public:
    void draw() override { cout << "[OpenGL] Ventana creada con contexto GL (glfwCreateWindow)\n"; }
};
class OpenGLButton : public Button {
public:
    void draw() override { cout << "[OpenGL] Boton dibujado con glDrawArrays\n"; }
};
class OpenGLShader : public Shader {
public:
    void compile() override { cout << "[OpenGL] Shader GLSL compilado con glCompileShader\n"; }
};


class VulkanWindow : public Window {
public:
    void draw() override { cout << "[Vulkan] Ventana creada con VkSurfaceKHR y swapchain\n"; }
};
class VulkanButton : public Button {
public:
    void draw() override { cout << "[Vulkan] Boton dibujado con vkCmdDraw en un command buffer\n"; }
};
class VulkanShader : public Shader {
public:
    void compile() override { cout << "[Vulkan] Shader compilado a SPIR-V (vkCreateShaderModule)\n"; }
};


class OpenGLFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() override { return make_unique<OpenGLWindow>(); }
    unique_ptr<Button> createButton() override { return make_unique<OpenGLButton>(); }
    unique_ptr<Shader> createShader() override { return make_unique<OpenGLShader>(); }
};

class VulkanFactory : public RenderFactory {
public:
    unique_ptr<Window> createWindow() override { return make_unique<VulkanWindow>(); }
    unique_ptr<Button> createButton() override { return make_unique<VulkanButton>(); }
    unique_ptr<Shader> createShader() override { return make_unique<VulkanShader>(); }
};


void renderScene(RenderFactory& factory) {
    auto window = factory.createWindow();
    auto button = factory.createButton();
    auto shader = factory.createShader();
    window->draw();
    button->draw();
    shader->compile();
}

int main() {
    unique_ptr<RenderFactory> factory = make_unique<OpenGLFactory>();

    auto window = factory->createWindow();
    auto button = factory->createButton();
    window->draw();
    button->draw();

    // Cambio de fabrica en tiempo de ejecucion
    factory = make_unique<VulkanFactory>();
    auto shader = factory->createShader();
    shader->compile();

    cout << "\n--- Escena completa con la fabrica actual (Vulkan) ---\n";
    renderScene(*factory);
}
