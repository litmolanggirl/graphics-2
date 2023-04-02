#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

GLuint VBO;  //переменная для хранения дескриптора буфера вершин
GLuint gWorldLocation;   //указатель для доступа к всемирной матрице

struct Vector3f
{
    float x;
    float y;
    float z;

    Vector3f()
    {
    }

    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
};

struct Matrix4f
{
    float m[4][4];
};

static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWorld * vec4(Position, 1.0);                                     \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = vec4(1.0, 0.0, 1.0, 0.0);                                           \n\
}";

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);   //очистка буфера для предустановленных значений

    static float Scale = 0.0f;

    Scale += 0.001f;

    //подготавливаем и заполняем матрицу 4х4
    Matrix4f World;
    World.m[0][0] = sinf(Scale); World.m[0][1] = 0.0f;        World.m[0][2] = 0.0f;        World.m[0][3] = 0.0f;
    World.m[1][0] = 0.0f;        World.m[1][1] = cosf(Scale); World.m[1][2] = 0.0f;        World.m[1][3] = 0.0f;
    World.m[2][0] = 0.0f;        World.m[2][1] = 0.0f;        World.m[2][2] = sinf(Scale); World.m[2][3] = 0.0f;
    World.m[3][0] = 0.0f;        World.m[3][1] = 0.0f;        World.m[3][2] = 0.0f;        World.m[3][3] = 1.0f;


    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);   //загрузка матрицы в шейдер

    glEnableVertexAttribArray(0);   //разрешение использования атрибутов вершин
    glBindBuffer(GL_ARRAY_BUFFER, VBO);   //привязка буфера
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);   //параметры данных в буфере

    glDrawArrays(GL_TRIANGLES, 0, 3);   //отрисовка треугольника

    glDisableVertexAttribArray(0);  //отключение атрибутов вершин

    glutSwapBuffers();
}


static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);   //обратный вызов, который отрисовывает 1 кадр
    glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[3];   //увеличение массива, чтобы он мог содержать 3 вершины
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);   //левая вершина
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);   //средняя верщина
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);   //правая вершина

    glGenBuffers(1, &VBO);  //создание буфера (первый - количество объектов, которое хотим создать, а второй — адрес массива для хранения дескрипторов)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //привязка объекта именованного буфера, 1 поле - атрибуты вершин
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);  //заполнение объекта данными (1 поле - атрибуты вершин, 4 - GL_STATIC_DRAW, потому что не собираюсь менять содержимое буфера)

}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);   //создание программного объекта

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);   //указываем тип шейдера и размерность массивов
    //компилируем шейдер
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);   //присоединяем спомпилированный объект шейдера к объекту программы
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);   //линкуем шейдеры

    //проверяем программные ошибки
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);   //назначаем программу для конвейера, программа сохранит эффект для всех вызовов отрисовки

    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);    //инициализация GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);    //настройка некоторых параметров GLUT
    glutInitWindowSize(512, 384);    //размер окна
    glutInitWindowPosition(100, 100);   //начальное положение
    glutCreateWindow("Tutorial 08");    //заголовок окна

    InitializeGlutCallbacks();

    //инициализация GLEW и проверка на ошибки
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);   //цвет, который будет использоваться при очистке буфера кадра

    CreateVertexBuffer();   //создание буфера вершины

    CompileShaders();

    glutMainLoop();

    return 0;
}