#include <iostream>
#undef __WIN32
#include "vgl.h"
#include "vmath.h"
#include "LoadShaders.h"
#include "GL/freeglut.h"

using namespace std;
using namespace vmath;

GLuint VAOs[1]; // Vertex Array Object
GLuint IBOs[1]; // Index Buffer Object
GLuint VBOs[1]; // Vertex Buffer Object

GLuint gProgram = 0;

ShaderInfo gShaders[] = {
        { GL_VERTEX_SHADER, "vertex_shader.glsl" },
        { GL_FRAGMENT_SHADER, "fragment_shader.glsl" },
        { GL_NONE, NULL }
    };

// Four vertices
static const GLfloat vertex_positions[] =
    {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  0.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
    };
// Color for each vertex
static const GLfloat vertex_colors[] =
{
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f
};

// Three indices (we’re going to draw one triangle at a time
static const GLushort vertex_indices[] =
{
    0, 1, 2
};

float aspect;
GLint render_model_matrix_loc;
GLint render_projection_matrix_loc;
//---------------------------------------------------------------------
//
// init
//
void
init(void)
{
    // Load shaders
    gProgram = LoadShaders(gShaders);
    
    // Use shader program
    glUseProgram(gProgram);
    
    // "model_matrix" is actually an array of 4 matrices
    render_model_matrix_loc = glGetUniformLocation(gProgram, "model_matrix");
    render_projection_matrix_loc = glGetUniformLocation(gProgram, "projection_matrix");

    // Create VAO, bind VAO
    glGenVertexArrays(1, VAOs);
    glBindVertexArray(VAOs[0]);
    
    // Create IBO, bind IBO, upload vertex index to IBO
    glGenBuffers(1, IBOs);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);
    
    // Create VBO, bind VBO, upload vertex position and color to VBO
    glGenBuffers(1, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions) + sizeof(vertex_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_positions), vertex_positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_positions), sizeof(vertex_colors), vertex_colors);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram, "color"), 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(vertex_positions));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "color"));
    
    glClearColor(0, 0, 0, 1);
    
    // Unbind VAO, IBO,  VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//---------------------------------------------------------------------
//
// display
//
void
display(void)
{
    bool auto_redraw = true;
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);
    mat4 model_matrix;
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader program
    glUseProgram(gProgram);
    
    // Set up the model and projection matrix
    vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f));
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    
    // Bind VAO
    glBindVertexArray(VAOs[0]);

    // Draw Arrays...
    model_matrix = vmath::translate(-3.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // DrawElements
    model_matrix = vmath::translate(-1.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL);

    // DrawElementsBaseVertex
    model_matrix = vmath::translate(1.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL, 1);

    // DrawArraysInstanced
    model_matrix = vmath::translate(3.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
    
    glutSwapBuffers();
    if (auto_redraw)
    {
        glutPostRedisplay();
    }
    
    // Unbind VAO and shader program after usage
    glBindVertexArray(0);
    glUseProgram(0);
}

//---------------------------------------------------------------------
//
// reshape
//
void reshape(int width, int height)
{
    glViewport(0, 0 , width, height);

    aspect = float(height) / float(width);
}
//---------------------------------------------------------------------
//
// main
//
int
main(int argc, char** argv)
{
    glewExperimental = GL_TRUE;

#ifdef _DEBUG
        glutInitContextFlags(GLUT_DEBUG);
#endif

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition (140, 140);
    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow(argv[0]);
    if (glewInit()) {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit(EXIT_FAILURE);
    }
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
}