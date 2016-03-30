#include <iostream>
#include "vutils.h"
#include "vmath.h"
#include "vermilion.h"

using namespace std;
using namespace vmath;

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 256
#define WINDOW_WIDTH_DEFAULT 1024
#define WINDOW_HEIGHT_DEFAULT 512
#define WINDOW_POS_X 140
#define WINDOW_POS_Y 140
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 3

int winWidth = WINDOW_WIDTH_DEFAULT;
int winHeight = WINDOW_HEIGHT_DEFAULT;
float aspect = float(winHeight) / float(winWidth);

#define USE_RENDERBUFFER 0

GLuint VAOs[2]; // Vertex Array Object
GLuint IBOs[2]; // Index Buffer Object
GLuint VBOs[2]; // Vertex Buffer Object
GLuint FBOs[2]; //Frame buffer object
GLuint RBOs[2]; //Frame buffer object
GLuint TEX_FBOs[2]; // Texture for framebuffer texture, attach color and depth
GLuint gProgram[2]; // shader program

static const GLfloat icosahedron_x = 0.525731112119133606f;
static const GLfloat icosahedron_z = 0.850650808352039932f;
static const GLfloat icosahedron_vertices[] =
{
    -icosahedron_x, 0.0f, icosahedron_z, 1.0f, 
    icosahedron_x, 0.0f, icosahedron_z, 1.0f, 
    -icosahedron_x, 0.0f, -icosahedron_z, 1.0f, 
    icosahedron_x, 0.0f, -icosahedron_z, 1.0f, 
    0.0f, icosahedron_z, icosahedron_x, 1.0f, 
    0.0f, icosahedron_z, -icosahedron_x, 1.0f, 
    0.0f, -icosahedron_z, icosahedron_x, 1.0f, 
    0.0f, -icosahedron_z, -icosahedron_x, 1.0f, 
    icosahedron_z, icosahedron_x, 0.0f, 1.0f, 
    -icosahedron_z, icosahedron_x, 0.0f, 1.0f, 
    icosahedron_z, -icosahedron_x, 0.0f, 1.0f, 
    -icosahedron_z, -icosahedron_x, 0.0f, 1.0f, 
};

static const GLushort icosahedron_indices[] =
{
    0, 4, 1, 
    0, 9, 4, 
    9, 5, 4, 
    4, 5, 8, 
    4, 8, 1, 
    8, 10, 1, 
    8, 3, 10, 
    5, 3, 8, 
    5, 2, 3, 
    2, 7, 3, 
    7, 10, 3, 
    7, 6, 10, 
    7, 11, 6, 
    11, 0, 6, 
    0, 1, 6, 
    6, 1, 10, 
    9, 0, 11, 
    9, 11, 2, 
    9, 2, 5, 
    7, 2, 11, 
};

static const GLfloat icosahedron_colors[] =
{
    0.0f, 0.0f, 0.0f, 1.0f, // 0 - Black
    0.0f, 0.0f, 1.0f, 1.0f, // 1 - Blue
    0.0f, 1.0f, 0.0f, 1.0f, // 2 - Green
    0.0f, 1.0f, 1.0f, 1.0f, // 3 - Cyan
    1.0f, 0.0f, 0.0f, 1.0f, // 4 - Red
    1.0f, 0.0f, 1.0f, 1.0f, // 5 - Pink
    1.0f, 1.0f, 0.0f, 1.0f, // 6 - Yellow
    1.0f, 1.0f, 1.0f, 1.0f, // 7 - White
    1.0f, 0.0f, 0.0f, 1.0f, // 8 - Red
    0.0f, 1.0f, 0.0f, 1.0f, // 9 - Green
    0.0f, 0.0f, 1.0f, 1.0f, // 10 - Blue
    1.0f, 0.0f, 1.0f, 1.0f, // 11 - Pink
};

static const GLfloat cube_vertices[] =
{
    -1.0f, -1.0f, 1.0f, 1.0f, 
    1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f, // Front Face
    1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f, // Back
    -1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, // Left
    1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, // Right
    -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, // Top
    -1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 
    1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, // Bottom
    /*
    -1.0f, -1.0f, -1.0f, 1.0f, // 0
    -1.0f, -1.0f, 1.0f, 1.0f, // 1
    -1.0f, 1.0f, -1.0f, 1.0f, // 2
    -1.0f, 1.0f, 1.0f, 1.0f, // 3
    1.0f, -1.0f, -1.0f, 1.0f, // 4
    1.0f, -1.0f, 1.0f, 1.0f, // 5
    1.0f, 1.0f, -1.0f, 1.0f, // 6
    1.0f, 1.0f, 1.0f, 1.0f, // 7
     * */
};

static const GLushort cube_quad_indices[] =
{
    1, 2, 0, 3, // Front
    5, 6, 4, 7, // Back
    9, 10, 8, 11, // Left
    13, 14, 12, 15, // Right
    17, 18, 16, 19, // Top
    21, 22, 20, 23, // Bottom
};

// Texture coordinates should match vertices
static const GLfloat texture_coordinates[] =
{
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Front
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Back
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Left
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Right
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Top
    0.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 0.0f, 0.0f, 1.0f, 
    1.0f, 1.0f, 0.0f, 1.0f, 
    0.0f, 1.0f, 0.0f, 1.0f, // Bottom
};

//---------------------------------------------------------------------
//
// init
//
void
init(void)
{
    // Create shader for fbo_texture
    gProgram[0] = glCreateProgram();
    
    static const char tex_vs[] = 
        "#version 430 core\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec4 color;\n"
        "out vec4 vs_fs_color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vs_fs_color = color;\n"
        "    gl_Position = projection_matrix * (model_matrix * position);\n"
        "}\n";
        
    static const char tex_fs[] = 
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
    vglAttachShaderSource(gProgram[0], GL_VERTEX_SHADER, tex_vs);
    vglAttachShaderSource(gProgram[0], GL_FRAGMENT_SHADER, tex_fs);
    glLinkProgram(gProgram[0]);
    
    // Create shader for widow-system-framebuffer
    gProgram[1] = glCreateProgram();
    
    static const char render_vs[] = 
        "#version 430 core\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        "layout (location = 0) in vec4 in_position;\n"
        "layout (location = 1) in vec4 in_tex_coord;\n"
        "out vec2 tex_coord;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    vec4 tex_coord_tmp = in_tex_coord;\n"
        "    tex_coord = tex_coord_tmp.xy;\n"
        "    gl_Position = projection_matrix * (model_matrix * in_position);\n"
        "}\n";
        
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
        "}\n";
    
    // Compile and link the shader
    vglAttachShaderSource(gProgram[1], GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram[1], GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram[1]);

    // Get name for VAO, IBO, VBO
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, IBOs);
    glGenBuffers(2, VBOs);
    
    // Setup VAO, VBO, IBO for fbo_texture
    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(icosahedron_indices), icosahedron_indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(icosahedron_vertices) + sizeof(icosahedron_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(icosahedron_vertices), icosahedron_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(icosahedron_vertices), sizeof(icosahedron_colors), icosahedron_colors);
    
    glVertexAttribPointer(glGetAttribLocation(gProgram[0], "position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram[0], "color"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(icosahedron_vertices)));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram[0], "position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram[0], "color"));
    
    // Setup VAO, VBO, IBO for drawing objects
    glBindVertexArray(VAOs[1]);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_quad_indices), cube_quad_indices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + sizeof(texture_coordinates), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vertices), cube_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices), sizeof(texture_coordinates), texture_coordinates);
    
    glVertexAttribPointer(glGetAttribLocation(gProgram[1], "in_position"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glVertexAttribPointer(glGetAttribLocation(gProgram[1], "in_tex_coord"), 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(cube_vertices)));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram[1], "in_position"));
    glEnableVertexAttribArray(glGetAttribLocation(gProgram[1], "in_tex_coord"));

    // Create frame buffer
#if (USE_RENDERBUFFER ==1)
    glGenFramebuffers(2, FBOs);
#else
    glGenFramebuffers(1, FBOs);
#endif
    
    // Bind frame buffer for texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOs[0]);
    
    // Create color and depth texture
    glGenTextures(2, TEX_FBOs);
    
    // Bind color texture
    glBindTexture(GL_TEXTURE_2D, TEX_FBOs[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    
    // attach texture to the frame buffer
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TEX_FBOs[0], 0);
    
    // Bind depth texture
    glBindTexture(GL_TEXTURE_2D, TEX_FBOs[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    
    // attach depth bufer to the frame buffer
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TEX_FBOs[1], 0);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
#if (USE_RENDERBUFFER ==1)
    // Bind frame buffer for render buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOs[1]);
    
    glGenRenderbuffers(2, RBOs);
    glBindRenderbuffer(GL_RENDERBUFFER, RBOs[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RBOs[0]);
    glBindRenderbuffer(GL_RENDERBUFFER, RBOs[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOs[1]);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#endif

    // Global configuration
    glClearColor(0, 0, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Unbind all
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
//---------------------------------------------------------------------
//
// display
//
vmath::mat4 projection_matrix;
vmath::mat4 model_matrix;
void
display(void)
{
    bool auto_redraw = true;
    static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
    static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
    static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);
    
    float t = float(GetTickCount() & 0x1FFF) / float(0x1FFF);

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    
    // Switch to frame buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOs[0]);
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1,  attachments);// only for color buffer
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glClearColor(0.4, 0.4, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // draw icosahedron to framebuffer as texture
    glUseProgram(gProgram[0]);
    glBindVertexArray(VAOs[0]);
    
    projection_matrix = vmath::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 500.0f);
    model_matrix = vmath::translate(0.0f, 0.0f, -2.0f) * rotate(t * 360.0f, X) * rotate(t * 1.0f * 360.0f, Y) * rotate(t * 2.0f * 360.0f, Z);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "projection_matrix"), 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "model_matrix"), 1, GL_FALSE, model_matrix);
    glDrawElements(GL_TRIANGLES, sizeof(icosahedron_indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, NULL);
    
    // Copy texture frame buffer to on-screen frame buffer, can used to check the texture
    
    /*
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOs[0]);
    glBindFramebuffer (GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, 0, winWidth,winHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
     * */     
     
    // Switch back to window-system-provided frame buffer
#if (USE_RENDERBUFFER ==1)
    // Draw to render buffer instead of primary surface
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBOs[1]);
#else
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    //glDrawBuffer(GL_BACK);
#endif
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glGenerateMipmap(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    
    glUseProgram(gProgram[1]);
    glBindVertexArray(VAOs[1]);
    
    glBindTexture(GL_TEXTURE_2D, TEX_FBOs[0]); // Using the color texture
    //glBindTexture(GL_TEXTURE_2D, TEX_FBOs[1]); // Visualize depth buffer using texture
    
    // Draw quad at left
    glViewport(0, 0, winWidth/2, winHeight);
    projection_matrix = vmath::frustum(-1.0f, 0.0f, -aspect, aspect, 1.0f, 10.0f);
    model_matrix = vmath::translate(-1.5f, 0.0f, -4.0f);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "projection_matrix"), 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "model_matrix"), 1, GL_FALSE, model_matrix);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    
    // Draw cube at right
    glViewport(winWidth/2, 0, winWidth/2, winHeight);
    projection_matrix = vmath::frustum(0.0f, 1.0f, -aspect, aspect, 1.0f, 10.0f);
    model_matrix = vmath::translate(1.5f, 0.0f, -4.0f) * rotate(22.5f, X) * rotate(315.0f, Y);;
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "projection_matrix"), 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "model_matrix"), 1, GL_FALSE, model_matrix);
    // Comment out some invisible face in current view
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL); //Front
    //glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(4 * 1 * sizeof(GLushort))); // Back
    //glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(4 * 2 * sizeof(GLushort))); // Left
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(4 * 3 * sizeof(GLushort))); // Right
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(4 * 4 * sizeof(GLushort))); // Top
    //glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (const GLvoid *)(4 * 5 * sizeof(GLushort))); // Bottom
    
#if (USE_RENDERBUFFER ==1)
    // Read and copy render buffer to primary surface
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOs[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, winWidth, winHeight, 0, 0, winWidth,winHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
#endif
    
    glutSwapBuffers();
    if (auto_redraw)
    {
        glutPostRedisplay();
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);

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
    winWidth = width;
    winHeight = height;
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
    for(i=0; i<sizeof(gProgram)/sizeof(GLuint); i++)
    {
        glDeleteProgram(gProgram[i]);
    }
    for(i=0; i<sizeof(TEX_FBOs) / sizeof(GLuint); i++)
    {
        glDeleteTextures(1, &TEX_FBOs[i]);
    }
    for(i=0; i<sizeof(RBOs) / sizeof(GLuint); i++)
    {
        glDeleteFramebuffers(1, &RBOs[i]);
    }
    for(i=0; i<sizeof(FBOs) / sizeof(GLuint); i++)
    {
        glDeleteFramebuffers(1, &FBOs[i]);
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
    glutInitWindowSize(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT);
    glutInitWindowPosition (WINDOW_POS_X, WINDOW_POS_Y);
    glutInitContextVersion(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR);
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