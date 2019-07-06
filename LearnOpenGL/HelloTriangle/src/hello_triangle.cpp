#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma comment(lib, "glfw3.lib")

// ���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ�������
// ���ǿ��ԶԴ���ע��һ���ص�����, ������ÿ�δ��ڴ�С��������ʱ�򱻵���
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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ������ɫ��(Vertex Shader)
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n\0";

// Ƭ����ɫ��(Fragment Shader)
const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello triangle", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window!\n");
        glfwTerminate();
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
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // ע������ص�����, ����GLFW����ϣ��ÿ�����ڵ�����С��ʱ������������
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // ����һ��������ɫ������
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // ����ɫ��Դ�븽�ӵ���ɫ��������
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // ������ɫ��
    glCompileShader(vertexShader);
    // ������ʱ����
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n", infoLog);
    }

    // Ƭ����ɫ��
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", infoLog);
    }

    // ��������ɫ���������ӵ�һ��������Ⱦ����ɫ������(Shader Program)��
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINK_FAILED: %s\n", infoLog);
    }
    
    // �ڰ���ɫ���������ӵ���������Ժ�, �Ͳ�����Ҫ������, ��Ҫɾ����ɫ������
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data and configure vertex attributes
    float vertices[] = {
        0.5f, 0.5f, 0.0f,   // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3,  // ��һ��������
        1, 2, 3   // �ڶ���������
    };

    // ���㻺�����VBO, �����������VAO, �����������EBO
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, 
    // then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // ���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // ��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ����OpenGL����ν�����������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // �Զ�������λ��ֵ��Ϊ���������ö�������
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ��Ⱦѭ��(Render Loop)
    while (!glfwWindowShouldClose(window)) {
        // input
        processInput(window);

        // �����Ļ����ɫ����
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw triangle
        // �ô�������ɫ�����������Ϊ���Ĳ���, �Լ����������
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        // �����û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬,�����ö�Ӧ�Ļص�����
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // ����Ⱦѭ��������������Ҫ��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ
    glfwTerminate();

    return 0;
}