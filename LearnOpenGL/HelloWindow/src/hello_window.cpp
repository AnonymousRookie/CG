#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma comment(lib, "glfw3.lib")

// 当用户改变窗口的大小的时候，视口也应该被调整。我们可以对窗口注册一个回调函数(Callback Function), 它会在每次窗口大小被调整的时候被调用
void framebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) 
{
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    // 初始化GLFW
    glfwInit();
    // 配置GLFW
    // 告诉GLFW我们使用的OpenGL版本是3.3, 将主版本号(Major)和次版本号(Minor)都设为3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 告诉GLFW我们使用的是核心模式(Core-profile)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建一个窗口对象, 这个窗口对象存放了所有和窗口相关的数据, 而且会被GLFW的其他函数频繁地用到
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello window", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window!\n");
        return -1;
    }
    // 创建完窗口通知GLFW将我们窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);

    // GLAD是用来管理OpenGL的函数指针的, 所以在调用任何OpenGL的函数之前我们需要初始化GLAD
    // 给GLAD传入用来加载系统相关的OpenGL函数指针地址的函数glfwGetProcAddress
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD!\n");
        return -1;
    }

    // 告诉OpenGL渲染窗口的尺寸大小, 即视口(Viewport)
    glViewport(0, 0, 800, 600);

    // 注册这个回调函数, 告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // 渲染循环(Render Loop)
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // 清空屏幕的颜色缓冲
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        // glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态,并调用对应的回调函数
        glfwPollEvents();
    }

    // 当渲染循环结束后我们需要正确释放/删除之前的分配的所有资源
    glfwTerminate();

    return 0;
}