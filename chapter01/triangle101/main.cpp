///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
///////////////////////////////////////////////////////////////////////
#include <iostream>
#undef __WIN32
using namespace std;
#include "vgl.h"
#include "LoadShaders.h"
#include "GL/freeglut.h"
enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;
GLuint gProgram = 0;

ShaderInfo gShaders[] = {
        { GL_VERTEX_SHADER, "triangles.vert" },
        { GL_FRAGMENT_SHADER, "triangles.frag" },
        { GL_NONE, NULL }
    };

//---------------------------------------------------------------------
//
// init
//
void
init(void)
{
    // Load shaders
    gProgram = LoadShaders(gShaders);
    
    // Create and bind new VAO
    glGenVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Triangles]);
    
    // Create and bind new VBO
    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    
    // Define triange vertices
    GLfloat vertices[NumVertices][3] = {
        // X Y Z
        { -0.90f, -0.90f, 0.0f }, // Triangle 1
        { 0.85f, -0.90f, 0.0f },
        { -0.90f, 0.85f, 0.0f },
        { 0.90f, -0.85f, 0.0f }, // Triangle 2
        { 0.90f, 0.90f, 0.0f },
        { -0.85f, 0.90f, 0.0f }
    };
    
    // Upload vertices into VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "vPosition"));
    
    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//---------------------------------------------------------------------
//
// display
//
void
display(void)
{
    // Clear screen
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use shader program
    glUseProgram(gProgram);
    
    // Bind VAO
    glBindVertexArray(VAOs[Triangles]);
    
    // Draw and flush
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glFlush();
    
    // Unbind VAO and shader program after usage
    glBindVertexArray(0);
    glUseProgram(0);
}
//---------------------------------------------------------------------
//
// main
//
int
main(int argc, char** argv)
{
    glewExperimental = GL_TRUE;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(512, 512);
    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow(argv[0]);
    if (glewInit()) {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit(EXIT_FAILURE);
    }
    init();
    glutDisplayFunc(display);
    glutMainLoop();
}