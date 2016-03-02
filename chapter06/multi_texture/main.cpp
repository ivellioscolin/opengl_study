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
    -1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,   
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
GLuint tex[2];
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
        "layout (location = 1) in vec2 in_tex_coord;\n"
        "\n"
        "out vec2 tex1_coord;\n"
        "out vec2 tex2_coord;\n"
        "\n"
        "uniform float time;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    const mat2 m = mat2( vec2(cos(time), sin(time)),\n"
        "                                            vec2(-sin(time), cos(time)) );\n"
        "    tex1_coord = in_tex_coord * m;\n"
        "    tex2_coord = in_tex_coord * transpose(m);\n"
        "    gl_Position = projection_matrix * (model_matrix * position);\n"
        "}\n";
        
    static const char render_fs[] = 
        "#version 430 core\n"
        "\n"
        "uniform sampler2D tex1;"
        "uniform sampler2D tex2;"
        "in vec2 tex1_coord;\n"
        "in vec2 tex2_coord;\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = texture(tex1, tex1_coord) + texture(tex2, tex2_coord);\n"
        "}\n";
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram, GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram, GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram);
    
    char buf[1024];
    glGetProgramInfoLog(gProgram, 1024, NULL, buf);
    
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW);
    
    // Create VBO, bind VBO, upload vertex position and color to VBO
    glGenBuffers(1, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(texture_coordinates), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square_vertices), square_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square_vertices), sizeof(texture_coordinates), texture_coordinates);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram, "in_tex_coord"), 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(square_vertices)));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "in_tex_coord"));
    
    glClearColor(0, 0, 0, 1);
    
    glUniform1i(glGetUniformLocation(gProgram, "tex1"), 0);
    glUniform1i(glGetUniformLocation(gProgram, "tex2"), 2);
    
    vglImageData image;
    
    tex[0] = vglLoadTexture("test.dds", 0, &image);
    glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vglUnloadImage(&image);

    tex[1] = vglLoadTexture("test3.dds", 1, &image);
    vglUnloadImage(&image);
    
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
    float t = float(GetTickCount() & 0x3FFF) / float(0x3FFF);
    
    glDisable(GL_DEPTH_TEST);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO
    glBindVertexArray(VAOs[0]);

    // Use shader program
    glUseProgram(gProgram);
    
    glUniform1f(glGetUniformLocation(gProgram, "time"), t);
    
    // Now draw the square.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex[1]);
    projection_matrix = vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    model_matrix = vmath::translate(0.0f, 0.0f, -2.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
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