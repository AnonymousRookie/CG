#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma comment(lib, "glfw3.lib")

// ���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ����������ǿ��ԶԴ���ע��һ���ص�����(Callback Function), ������ÿ�δ��ڴ�С��������ʱ�򱻵���
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
    // ��ʼ��GLFW
    glfwInit();
    // ����GLFW
    // ����GLFW����ʹ�õ�OpenGL�汾��3.3, �����汾��(Major)�ʹΰ汾��(Minor)����Ϊ3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // ����GLFW����ʹ�õ��Ǻ���ģʽ(Core-profile)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ����һ�����ڶ���, ������ڶ����������кʹ�����ص�����, ���һᱻGLFW����������Ƶ�����õ�
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello window", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window!\n");
        return -1;
    }
    // �����괰��֪ͨGLFW�����Ǵ��ڵ�����������Ϊ��ǰ�̵߳���������
    glfwMakeContextCurrent(window);

    // GLAD����������OpenGL�ĺ���ָ���, �����ڵ����κ�OpenGL�ĺ���֮ǰ������Ҫ��ʼ��GLAD
    // ��GLAD������������ϵͳ��ص�OpenGL����ָ���ַ�ĺ���glfwGetProcAddress
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD!\n");
        return -1;
    }

    // ����OpenGL��Ⱦ���ڵĳߴ��С, ���ӿ�(Viewport)
    glViewport(0, 0, 800, 600);

    // ע������ص�����, ����GLFW����ϣ��ÿ�����ڵ�����С��ʱ������������
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // ��Ⱦѭ��(Render Loop)
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // �����Ļ����ɫ����
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        // glfwPollEvents���������û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬,�����ö�Ӧ�Ļص�����
        glfwPollEvents();
    }

    // ����Ⱦѭ��������������Ҫ��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ
    glfwTerminate();

    return 0;
}