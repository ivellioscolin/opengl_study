#include <iostream>
#undef __WIN32
#include "vutils.h"
#include "vmath.h"

using namespace std;
using namespace vmath;

GLuint VAOs[1]; // Vertex Array Object
GLuint IBOs[1]; // Index Buffer Object
GLuint VBOs[1]; // Vertex Buffer Object

GLuint gProgram = 0;

static const GLfloat cube_vertices[] =
{
    -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,// Above is a cube
    -0.577f, -0.289f, 1.3f, 1.0f,
    0.577f, -0.289f, 1.3f, 1.0f,
    0.0f, 0.577f, 1.3f, 1.0f,
    0.0f, 0.0f, -1.3f, 1.0f,// Above is a prism
};

static const GLfloat cube_vertex_colors[] =
{
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 1.0f,// Above is for cube
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.0f, 0.0f, 1.0f, 
    0.0f, 0.8f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.8f, 1.0f,// Above is for prism
};

static const GLushort cube_indices[] =
{
    0, 1, 2, 3, 6, 7, 4, 5, // First strip
    0xFFFF, // <<- - This is the restart index
    2, 6, 0, 4, 1, 5, 3, 7, // Second strip
    0xFFFF,
    8, 9, 10,
    0xFFFF,
    9, 11, 10,
    0xFFFF,
    8, 11, 9,
    0xFFFF,
    8, 10, 11,
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
    // Create shader
    gProgram = glCreateProgram();
    
    static const char render_vs[] = 
        "#version 430 core\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec4 color;\n"
        "\n"
        "out vec4 vs_fs_color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vs_fs_color = color;\n"
        "    gl_Position = projection_matrix * (model_matrix * position);\n"
        "}\n";
        
    static const char render_fs[] = 
        "#version 430 core\n"
        "\n"
        "in vec4 vs_fs_color;\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = vs_fs_color;\n"
        "}\n";
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram, GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram, GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    
    // Create VBO, bind VBO, upload vertex position and color to VBO
    glGenBuffers(1, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + sizeof(cube_vertex_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vertices), cube_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices), sizeof(cube_vertex_colors), cube_vertex_colors);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram, "color"), 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)sizeof(cube_vertices));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "color"));
    
    glClearColor(0, 0, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
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
    vmath::mat4 projection_matrix;
    vmath::mat4 model_matrix;
    
    glEnable(GL_PRIMITIVE_RESTART);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO
    glBindVertexArray(VAOs[0]);
       
    // Use shader program
    glUseProgram(gProgram);
    
    glPrimitiveRestartIndex(0xFFFF);
    
    // Set up the model and projection matrix
    float t = float(GetTickCount() & 0x1FFF) / float(0x1FFF);
    projection_matrix = vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    model_matrix = vmath::translate(0.0f, 0.0f, -5.0f) * rotate(t* 360.0f, Y);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    
    // Draw prism
    glDepthFunc(GL_LESS);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, NULL);

    // Draw cube
    glDepthFunc(GL_GREATER);
    glDrawElements(GL_TRIANGLES, 15, GL_UNSIGNED_SHORT, (const GLvoid *)(18 * sizeof(GLushort)));
      
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
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_STENCIL);
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