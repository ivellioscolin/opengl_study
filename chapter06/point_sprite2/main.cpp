#include <iostream>
#include "vutils.h"
#include "vmath.h"
#include "vermilion.h"
#include "time.h"
#include "stdlib.h"

using namespace std;
using namespace vmath;

GLuint VAOs[1]; // Vertex Array Object
GLuint VBOs[1]; // Vertex Buffer Object

GLuint gProgram = 0;

GLfloat *p_point_position = NULL;
#define POINT_NUMBER 40

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
    srand(time(0));
    // Create shader
    gProgram = glCreateProgram();
    
    static const char render_vs[] = 
        "#version 430 core\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = projection_matrix * (model_matrix * position);\n"
        "    gl_PointSize = (1.0 - gl_Position.z / gl_Position.w) * 64.0;\n"
        "}\n";
        
    static const char render_fs[] = 
        "#version 430 core\n"
        "\n"
        "uniform sampler2D tex;"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    const vec4 color1 = vec4(0.6, 0.0, 0.0, 1.0);\n"
        "    const vec4 color2 = vec4(0.9, 0.7, 1.0, 0.0);\n"
        "    vec2 temp = gl_PointCoord - vec2(0.5);"//gl_PointCoord.s ranges from 0.0 to 1.0
        "    float f = dot(temp, temp);\n"// squared distance to center
        "    if (f > 0.25)\n"
        "        discard;"
        "    color = mix(color1, color2, smoothstep(0.1, 0.25, f));\n"
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
    
    p_point_position = (GLfloat *)malloc(sizeof(GLfloat) * 4 * POINT_NUMBER);
    for(unsigned int i=0;i < POINT_NUMBER;i++)
    {
        p_point_position[i * 4 + 0] = rand() * 2.0f / RAND_MAX - 1.0f;// x
        p_point_position[i * 4 + 1] = rand() * 2.0f / RAND_MAX - 1.0f;// y
        p_point_position[i * 4 + 2] = rand() * 1.0f / RAND_MAX;// z
        p_point_position[i * 4 + 3] = 1.0f;// w
    }

    // Create VBO, bind VBO, upload vertex position and color to VBO
    glGenBuffers(1, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * POINT_NUMBER, p_point_position, GL_STATIC_DRAW);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram, "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "position"));
    
    glClearColor(0, 0, 0, 1);
    
    // Unbind VAO, IBO,  VBO
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
    bool auto_redraw = true;
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);
    vmath::mat4 projection_matrix;
    vmath::mat4 model_matrix;
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_DST_ALPHA);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO
    glBindVertexArray(VAOs[0]);

    // Use shader program
    glUseProgram(gProgram);
    
    // Now draw the square.
    projection_matrix = vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 500.0f);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    model_matrix = vmath::translate(0.0f, 0.0f, -2.0f);
    glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glDrawArrays(GL_POINTS, 0, POINT_NUMBER);
    
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
    for(i=0; i<sizeof(VBOs) / sizeof(GLuint); i++)
    {
        glDeleteBuffers(1, &VBOs[i]);
    }
    for(i=0; i<sizeof(VAOs) / sizeof(GLuint); i++)
    {
        glDeleteVertexArrays(1, &VAOs[i]);
    }
    free(p_point_position);
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