#include <iostream>
#include "vutils.h"
#include "vmath.h"
#include "vermilion.h"
#include "vbm.h"

using namespace std;
using namespace vmath;

GLuint VAOs[1]; // Vertex Array Object
GLuint IBOs[1]; // Index Buffer Object
GLuint VBOs[1]; // Vertex Buffer Object

GLuint gProgram[2];

static const GLfloat cube_vertices[] =
{
    -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};

static const GLushort cube_indices[] =
{
    0, 1, 2, 3, 6, 7, 4, 5, // First strip
    0xFFFF, // <<- - This is the restart index
    2, 6, 0, 4, 1, 5, 3, 7 // Second strip
};

float aspect;
GLuint tex[2];

VBObject object;
//---------------------------------------------------------------------
//
// init
//
void
init(void)
{
    // Create shader for environment
    gProgram[0] = glCreateProgram();
    
    static const char environment_vs[] = 
        "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "\n"
        "out vec3 tex_coord;\n"
        "\n"
        "uniform mat4 tc_rotate;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    tex_coord = position.xyz;\n"
        "    gl_Position = tc_rotate * position;\n"
        "}\n";
        
    static const char environment_fs[] = 
        "#version 430 core\n"
        "\n"
        "uniform samplerCube tex;"
        "in vec3 tex_coord;\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = texture(tex, tex_coord);\n"
        "}\n";
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram[0], GL_VERTEX_SHADER, environment_vs);
    vglAttachShaderSource(gProgram[0], GL_FRAGMENT_SHADER, environment_fs);
    glLinkProgram(gProgram[0]);
    
    char buf[1024];
    glGetProgramInfoLog(gProgram[0], 1024, NULL, buf);
    
    // Create shader for object
    gProgram[1] = glCreateProgram();
    
    static const char object_vs[] = 
        "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec3 normal;\n"
        "\n"
        "out vec3 vs_fs_normal;\n"
        "out vec3 vs_fs_position;\n"
        "\n"
        "uniform mat4 mat_mvp;\n"
        "uniform mat4 mat_mv;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = mat_mvp * position;\n"
        "    vs_fs_normal = mat3(mat_mv) * normal;\n"
        "    vs_fs_position = (mat_mv * position).xyz;\n"
        "}\n";
        
    static const char object_fs[] = 
        "#version 430 core\n"
        "\n"
        "uniform samplerCube tex;"
        "\n"
        "in vec3 vs_fs_normal;\n"
        "in vec3 vs_fs_position;\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vec3 tc =  reflect(vs_fs_position, normalize(vs_fs_normal));\n"
        "    color = vec4(0.3, 0.2, 0.1, 1.0) + vec4(0.97, 0.83, 0.79, 0.0) * texture(tex, tc);\n"
        "}\n";
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram[1], GL_VERTEX_SHADER, object_vs);
    vglAttachShaderSource(gProgram[1], GL_FRAGMENT_SHADER, object_fs);
    glLinkProgram(gProgram[1]);
    
    glGetProgramInfoLog(gProgram[1], 1024, NULL, buf);
    
    //glUseProgram(gProgram[1]);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    
    // Setup VAO
    glVertexAttribPointer(glGetAttribLocation(gProgram[0], "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram[0], "position"));

    glClearColor(0, 0, 0, 1);
    
    vglImageData image;
    
    tex[0] = vglLoadTexture("TantolundenCube.dds", 0, &image);
    glTexParameteri(image.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vglUnloadImage(&image);
    
    object.LoadFromVBM("torus.vbm", 0, 1, 2);
    object.BindVertexArray();
    
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
    static const unsigned int start_time = GetTickCount();
    float t = float((GetTickCount() - start_time)) / float(0x3FFF);
    
    vmath::mat4 tc_matrix(vmath::mat4::identity());
    
    glEnable(GL_PRIMITIVE_RESTART);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);// To demo seam on edge, with GL_LINEAR_MIPMAP_LINEAR enabled
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO
    glBindVertexArray(VAOs[0]);

    // Use shader program
    glUseProgram(gProgram[0]);
    
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 100.0f) * rotate(t * 180.0f, Y) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "tc_rotate"), 1, GL_FALSE, tc_matrix);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOs[0]);
    glPrimitiveRestartIndex(0xFFFF);
    glDrawElements(GL_TRIANGLE_STRIP, 17, GL_UNSIGNED_SHORT, NULL);
    
    glUseProgram(gProgram[1]);

    tc_matrix = vmath::translate(vmath::vec3(0.0f, 0.0f, -80.0f)) *
                vmath::rotate(80.0f * 3.0f * t, Y) * vmath::rotate(70.0f * 3.0f * t, Z);

    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "mat_mv"), 1, GL_FALSE, tc_matrix);
    tc_matrix = vmath::perspective(35.0f, 1.0f / aspect, 0.1f, 100.0f) * tc_matrix;
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "mat_mvp"), 1, GL_FALSE, tc_matrix);

    glClear(GL_DEPTH_BUFFER_BIT);

    object.Render();
    
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
    for(i=0; i<sizeof(gProgram) / sizeof(GLuint); i++)
    {
        glDeleteProgram(gProgram[i]);
    }
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