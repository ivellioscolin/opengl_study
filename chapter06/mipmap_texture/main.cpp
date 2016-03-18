#include <iostream>
#include "vutils.h"
#include "vmath.h"
#include "vermilion.h"

using namespace std;
using namespace vmath;

GLuint VAOs[1]; // Vertex Array Object
GLuint IBOs[1]; // Index Buffer Object
GLuint VBOs[1]; // Vertex Buffer Object

GLuint gProgram = 0;

static const GLfloat square_vertices[] =
{
    -20.0f, 0.0f, 50.0f, 1.0f,
    20.0f, 0.0f, 50.0f, 1.0f,
    20.0f, 0.0f, -500.0f, 1.0f,
    -20.0f, 0.0f, -500.0f, 1.0f,   
};

static const GLfloat texture_coordinates[] =
{
    0.0f, 0.0f, 
    1.0f, 0.0f, 
    1.0f, 1.0f, 
    0.0f, 1.0f, 
};

static const GLushort square_indices[] =
{
    1, 2, 0, 3
};

float aspect;
GLuint tex[1];
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
        "layout (location = 0) in vec4 in_position;\n"
        "layout (location = 1) in vec2 in_tex_coord;\n"
        "\n"
        "out vec2 tex_coord;\n"
        "\n"
        "uniform mat4 tc_rotate;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position =tc_rotate * in_position;\n"
        "    tex_coord = in_tex_coord;\n"
        "}\n"
        ;
        
    static const char render_fs[] = 
        "#version 430 core\n"
        "\n"
        "in vec2 tex_coord;\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "uniform sampler2D tex;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = texture(tex, tex_coord);\n"
        "}\n"
        ;
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram, GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram, GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram);
    glUseProgram(gProgram);

    // Create VAO, bind VAO
    glGenVertexArrays(1, VAOs);
    glBindVertexArray(VAOs[0]);
    
    // Create VBO, bind VBO, upload vertex position and color to VBO
    glGenBuffers(1, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(texture_coordinates), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square_vertices), square_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square_vertices), sizeof(texture_coordinates), texture_coordinates);
    
    // Create IBO, bind IBO, upload vertex index to IBO
    glGenBuffers(1, IBOs);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "in_position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram, "in_tex_coord"), 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(square_vertices)));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "in_position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "in_tex_coord"));
    
    glGenTextures(1, &tex[0]);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexStorage2D(GL_TEXTURE_2D, 7, GL_RGBA8, 64, 64);
    
    unsigned int * data = new unsigned int [64 * 64];
    unsigned int colors[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF };
    for (unsigned int level = 0; level < 7; level++)
    {
        for (unsigned int dim = 0; dim < 64 * 64; dim++)
        {
            data[dim] = colors[level];
        }
        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, 64 >> level, 64 >> level, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 4.5f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glClearColor(0.0f, 0.25f, 0.3f, 1.0f);
    glClearDepth(1.0f);
    
    // Unbind VAO, IBO,  VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    vmath::mat4 tc_matrix(vmath::mat4::identity());
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO
    glBindVertexArray(VAOs[0]);

    // Use shader program
    glUseProgram(gProgram);
    
    //GL_LINEAR_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    tc_matrix = vmath::translate(vmath::vec3(-25.0f, 20.0f, -60.0f)) * vmath::rotate(80.0f * 3.0f * 0.03f, X);
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 700.0f) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "tc_rotate"), 1, GL_FALSE, tc_matrix);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    
    //GL_NEAREST_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    tc_matrix = vmath::translate(vmath::vec3(25.0f, 20.0f, -60.0f)) * vmath::rotate(80.0f * 3.0f * 0.03f, X);
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 700.0f) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "tc_rotate"), 1, GL_FALSE, tc_matrix);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    
    //GL_NEAREST_MIPMAP_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    tc_matrix = vmath::translate(vmath::vec3(-25.0f, -20.0f, -60.0f)) * vmath::rotate(80.0f * 3.0f * 0.03f, X);
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 700.0f) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "tc_rotate"), 1, GL_FALSE, tc_matrix);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    
    //GL_LINEAR_MIPMAP_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    tc_matrix = vmath::translate(vmath::vec3(25.0f, -20.0f, -60.0f)) * vmath::rotate(80.0f * 3.0f * 0.03f, X);
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 700.0f) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "tc_rotate"), 1, GL_FALSE, tc_matrix);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);

    glutSwapBuffers();
    if (auto_redraw)
    {
        glutPostRedisplay();
    }
    
    // Unbind VAO and shader program after usage
    glBindVertexArray(0);
    glUseProgram(0);
}
static int continue_in_main_loop = 1;
static void keyPress(unsigned char key, int x, int y)
{
  int need_redisplay = 1;
  
  switch (key) {
  case 'q' :
    continue_in_main_loop = 0 ;
    break ;

  default:
    need_redisplay = 0;
    break;
  }
  if (need_redisplay)
    glutPostRedisplay();
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
// finalize
//
void finalize(void)
{
    unsigned int i = 0;
    glUseProgram(0);
    glDeleteProgram(gProgram);
    for(i=0; i<sizeof(tex) / sizeof(GLuint); i++)
    {
        glDeleteTextures(1, &tex[i]);
    }
    for(i=0; i<sizeof(VBOs) / sizeof(GLuint); i++)
    {
        glDeleteBuffers(1, &VBOs[i]);
    }
    for(i=0; i<sizeof(IBOs) / sizeof(GLuint); i++)
    {
        glDeleteBuffers(1, &IBOs[i]);
    }
    for(i=0; i<sizeof(VAOs) / sizeof(GLuint); i++)
    {
        glDeleteVertexArrays(1, &VAOs[i]);
    }
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
    glutKeyboardFunc(keyPress);

    while(continue_in_main_loop)
        glutMainLoopEvent();
    
    finalize();
}