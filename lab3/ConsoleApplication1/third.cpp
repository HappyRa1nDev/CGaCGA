#include <iostream>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL.h"

using namespace std;

GLFWwindow* g_window;

GLuint g_shaderProgram;
#define size 100

#define r_x 0.0f //поворот вокруг x
#define r_y 0.0f //поворот вокруг y
#define r_z 1.0f //споворот вокруг z

#define t_x -0.5f //сдвиг по x
#define t_y -0.5f //сдвиг по y
#define t_z 0.0f //сдвиг по z
#define a_plus 0.01f //скорость вращения



GLuint textureID[2];


GLint location1;
GLint location2;

class Model
{
public:
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLsizei indexCount;
};

Model g_model;

chrono::system_clock::time_point f_time = chrono::system_clock::now();
chrono::system_clock::time_point s_time = chrono::system_clock::now();



const char* imgpath1 = "C:\\Study\\ПГНИУ2\\3 курс\\ВГиАКГ\\labTest\\ConsoleApplication1\\sea.png";
const char* imgpath2 = "C:\\Study\\ПГНИУ2\\3 курс\\ВГиАКГ\\labTest\\ConsoleApplication1\\anime_girl.png";

float angle = glm::radians(0.0f);
GLint g_r;
GLint g_sc;
GLint g_t;


GLfloat Mixing;
float MixingValue = 0.5;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        MixingValue= MixingValue < 1 ? MixingValue + 0.1 : 1;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        MixingValue = MixingValue > 0 ? MixingValue - 0.1 : 0;
}

float f(float x, float y) {
    //return sin(x) * cos(y) * exp(-1*(x * x + y * y) / 10);
    return 3*sin(sqrt(x * x + y * y)) * cos(sqrt(x * x + y * y));
    //return sin((x*x+y*y));
}
float dx(float x, float y) {
    return 3 *x* sin(2 * sqrt(x * x + y * y)) / sqrt(x * x + y * y);
}
float dy(float x, float y) {
    return 3*y * sin(2 * sqrt(x * x + y * y)) / sqrt(x * x + y * y);
}
float dz(float x, float y) {
    return dx(x,y)+dy(x,y);
}
GLuint createShader(const GLchar* code, GLenum type)
{
    GLuint result = glCreateShader(type);

    glShaderSource(result, 1, &code, NULL);
    glCompileShader(result);

    GLint compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetShaderInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader compilation error" << endl << infoLog << endl;
        }
        glDeleteShader(result);
        return 0;
    }

    return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
    GLuint result = glCreateProgram();

    glAttachShader(result, vsh);
    glAttachShader(result, fsh);

    glLinkProgram(result);

    GLint linked;
    glGetProgramiv(result, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetProgramInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader program linking error" << endl << infoLog << endl;
        }
        glDeleteProgram(result);
        return 0;
    }

    return result;
}

bool createShaderProgram()
{
    g_shaderProgram = 0;
    const GLchar vsh[] =
        "#version 330\n"
        ""
        "layout(location = 0) in vec3 a_position;"
        "layout(location = 1) in vec3 a_color;"
        ""
        "uniform mat4 r; "
        "uniform mat4 t; "
        "uniform mat4 sc; "
        "out vec3 v_color, v_position;"
        "out vec2 text_coord;"
        ""
        "void main()"
        "{"
        "    mat4 rtsc= r*t*sc;"
        "    gl_Position =rtsc*vec4(a_position, 1.0);"
        "    v_position = vec3(gl_Position);"
        "    mat3 n = mat3(rtsc);"
        "    n = inverse(n);"
        "    n = transpose(n);"
        "    v_color = n*a_color;"
        "    text_coord = vec2(sc*vec4(a_position, 1));"
        "}"
        ;
    //    "   o_color = vec4(v_color, 1.0);"
    const GLchar fsh[] =
        "#version 330\n"
        ""
        "in vec3 v_color,v_position;"
        "in vec2 text_coord;"

        "uniform sampler2D map1;"
        "uniform sampler2D map2;"
        "uniform float Mixing;"
        ""
        "layout(location = 0) out vec4 o_color;"
        ""
        "void main()"
        "{"
        //"   vec3 mycolor = vec3(1.0f, 0.0f, 1.0f);"
        "   vec3 n = normalize(v_color);"

        "   vec3 L = vec3(0, 0.5, 0);" // источник света
        "   vec3 l = normalize(L - v_position);" // поток света
        "   vec3 e = normalize(-1 * v_position);" // вектор в сторону наблюдателя

        "   float d = max(dot(l, n), 0.3);" // дифузное освещение

        "   vec3 h = normalize(l + e);"
        "   float s = dot(h, n);"
        "   s = pow(max(s, 0), 20) * int(dot(l, n)>=0);" // блик

        "   vec4 t1 = texture(map1, text_coord);"
        "   vec4 t2 = texture(map2, text_coord);"
        "   vec4 dc = mix(t1, t2, Mixing);"//мешаем текстуры

        "   o_color = vec4(vec3(dc) * d + vec3(s), 1);"
        "}"
        ;
    GLuint vertexShader, fragmentShader;

    vertexShader = createShader(vsh, GL_VERTEX_SHADER);
    fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

    g_shaderProgram = createProgram(vertexShader, fragmentShader);
   
    //запрос из шейдера
    g_r = glGetUniformLocation(g_shaderProgram, "r");
    g_t = glGetUniformLocation(g_shaderProgram, "t");
    g_sc = glGetUniformLocation(g_shaderProgram, "sc");



    location1 = glGetUniformLocation(g_shaderProgram, "map1");
    location2 = glGetUniformLocation(g_shaderProgram, "map2");
    Mixing = glGetUniformLocation(g_shaderProgram, "Mixing");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return g_shaderProgram != 0;
}

bool createModel()
{
    const int x_size = size;
    const int y_size = size;

    GLfloat* vertices= new GLfloat[x_size*y_size*6];
    for (int i = 0; i < x_size*y_size; i++) {
        int y = i / x_size;
        int x = i % x_size;
        vertices[i * 6 + 0] = x;///(x_size*1.0f)-0.5f;
        vertices[i * 6 + 1] = y;///(y_size*1.0f)-0.5f;
        vertices[i * 6 + 2] = f(x, y);//f(vertices[i * 6 + 0], vertices[i * 6 + 1]) ;
        vertices[i * 6 + 3] = dx(x, y);  //vertices[i * 6 + 1];//abs(rand() % 255)/255.0;
        vertices[i * 6 + 4] = dy(x, y); //vertices[i * 6 + 2];//abs(rand() % 255) / 255.0;
        vertices[i * 6 + 5] = -1;//dz(x, y); //-1;//abs(rand() % 255) / 255.0;
    }
    GLuint* indices= new  GLuint[x_size * y_size*2*3];
    int ind_ = 0;
    for (int y = 0; y < y_size-1; y++) {
        for (int x = 0; x < x_size - 1; x++) {
            indices[ind_++] = x + y * x_size;
            indices[ind_++] = x + y * x_size+x_size;
            indices[ind_++] = x + y * x_size+1;
        }
        for (int x = 1; x < x_size; x++) {
            indices[ind_++] = x + y * x_size;
            indices[ind_++] = x-1 + y * x_size + x_size;
            indices[ind_++] = x + y * x_size + x_size;
        }
    }

    glGenVertexArrays(1, &g_model.vao);
    glBindVertexArray(g_model.vao);

    glGenBuffers(1, &g_model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, x_size*y_size*6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_model.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, x_size * y_size * 2 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);
    g_model.indexCount = x_size * y_size * 2 * 3;

    delete[] vertices;
    delete[] indices;


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));

    return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}
//функци для загрузки текстуры RGBA
void initRGBA(GLuint texID, const char* fname)
{
    GLsizei texW, texH;
    unsigned char* image = SOIL_load_image(fname, &texW, &texH, 0, SOIL_LOAD_RGBA);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}
//функци для загрузки текстуры RGBA
void initRGB(GLuint texID, const char* fname)
{
    GLsizei texW, texH;
    unsigned char* image = SOIL_load_image(fname, &texW, &texH, 0, SOIL_LOAD_RGB);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
bool init()
{
    // Set initial color of color buffer to white.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glGenTextures(2, textureID);

    //подгружаем текстуры
    initRGBA(textureID[0], imgpath1);
    initRGBA(textureID[1], imgpath2);

    glEnable(GL_DEPTH_TEST); // изменение состояний в машине состояний 
    glEnable(GL_FRAMEBUFFER_SRGB);
    return createShaderProgram() && createModel();
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void draw()
{
    // Clear buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_shaderProgram);
    glBindVertexArray(g_model.vao);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



    s_time = chrono::system_clock::now();
    if (chrono::duration_cast<chrono::milliseconds>(s_time - f_time).count() >= 10) {
        f_time = s_time;
        angle = fmod((float)(angle + a_plus), (3.14 * 2));
    }

    


    // Матрица поворота 
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(r_x, r_y, r_z));

    // Матрица переноса 
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(t_x, t_y, t_z));

    // Матрица уменьшения
    glm::mat4 sc = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / size));

    glUniformMatrix4fv(g_r, 1, GL_FALSE, glm::value_ptr(r));
    glUniformMatrix4fv(g_t, 1, GL_FALSE, glm::value_ptr(t));
    glUniformMatrix4fv(g_sc, 1, GL_FALSE, glm::value_ptr(sc));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glUniform1i(location1, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    glUniform1i(location2, 1);

    glUniform1f(Mixing, MixingValue);

    glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cleanup()
{
    if (g_shaderProgram != 0)
        glDeleteProgram(g_shaderProgram);
    if (g_model.vbo != 0)
        glDeleteBuffers(1, &g_model.vbo);
    if (g_model.ibo != 0)
        glDeleteBuffers(1, &g_model.ibo);
    if (g_model.vao != 0)
        glDeleteVertexArrays(1, &g_model.vao);
    glDeleteTextures(2, textureID);
}

bool initOpenGL()
{
    // Initialize GLFW functions.
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return false;
    }

    // Request OpenGL 3.3 without obsoleted functions.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window.
    g_window = glfwCreateWindow(800, 600, "OpenGL Test", NULL, NULL);
    if (g_window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        glfwTerminate();
        return false;
    }

    // Initialize OpenGL context with.
    glfwMakeContextCurrent(g_window);

    // Set internal GLEW variable to activate OpenGL core profile.
    glewExperimental = true;

    // Initialize GLEW functions.
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return false;
    }

    // Set callback for framebuffer resizing event.
    glfwSetKeyCallback(g_window, key_callback); //ивент на клик

    // Ensure we can capture the escape key being pressed.
    glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set callback for framebuffer resizing event.
    glfwSetFramebufferSizeCallback(g_window, reshape);

    return true;
}

void tearDownOpenGL()
{
    // Terminate GLFW.
    glfwTerminate();
}

int main()
{
    // Initialize OpenGL
    if (!initOpenGL())
        return -1;

    // Initialize graphical resources.
    bool isOk = init();

    if (isOk)
    {
        // Main loop until window closed or escape pressed.
        while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
        {
            // Draw scene.
            draw();

            // Swap buffers.
            glfwSwapBuffers(g_window);
            // Poll window events.
            glfwPollEvents();
        }
    }

    // Cleanup graphical resources.
    cleanup();

    // Tear down OpenGL.
    tearDownOpenGL();

    return isOk ? 0 : -1;
}