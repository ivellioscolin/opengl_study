#include <iostream>
#undef __WIN32
#include "vgl.h"
#include "vutils.h"
#include "vmath.h"
#include "vbm.h"
#include "GL/freeglut.h"

using namespace std;
using namespace vmath;

GLuint TO_color[1]; // Texture Object for color
GLuint TO_model_matrix[1];// TO for model matrix
GLuint TBO_color[1]; // Texture Buffer Object for color
GLuint TBO_model_matrix[1];// TBO for model matrix

GLuint gProgram = 0;

float aspect;
GLint view_matrix_loc;
GLint render_projection_matrix_loc;

VBObject vbmObj;

#define INSTANCE_COUNT 100

//---------------------------------------------------------------------
//
// init
//
void
init(void)
{
    // Create shader
    gProgram = glCreateProgram();
    
    // Initialize inline vertex shader and fragment shader
    static const char render_vs[] =
        "#version 430 core\n"
        "\n"
        "// 'position' and 'normal' are regular vertex attributes\n"
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec3 normal;\n"
        "\n"
        "// Color is a per-instance attribute\n"
        "layout (location = 3) in vec4 color;\n"
        "\n"
        "// The view matrix and the projection matrix are constant across a draw\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        "// These are the TBOs that hold per-instance colors and per-instance\n"
        "// model matrices\n"
        "uniform samplerBuffer color_tbo;\n"
        "uniform samplerBuffer model_matrix_tbo;\n"
        "\n"
        "// The output of the vertex shader (matched to the fragment shader)\n"
        "out VERTEX\n"
        "{\n"
        "    vec3    normal;\n"
        "    vec4    color;\n"
        "} vertex;\n"
        "\n"
        "// Ok, go!\n"
        "void main(void)\n"
        "{\n"
        "    // Use gl_InstanceID to obtain the instance color from the color TBO\n"
        "    vec4 color = texelFetch(color_tbo, gl_InstanceID);\n"
        "\n"
        "    // Generating the model matrix is more complex because you can't\n"
        "    // store mat4 data in a TBO. Instead, we need to store each matrix\n"
        "    // as four vec4 variables and assemble the matrix in the shader.\n"
        "    // First, fetch the four columns of the matrix (remember, matrices are\n"
        "    // stored in memory in column-primary order).\n"
        "    vec4 col1 = texelFetch(model_matrix_tbo, gl_InstanceID * 4);\n"
        "    vec4 col2 = texelFetch(model_matrix_tbo, gl_InstanceID * 4 + 1);\n"
        "    vec4 col3 = texelFetch(model_matrix_tbo, gl_InstanceID * 4 + 2);\n"
        "    vec4 col4 = texelFetch(model_matrix_tbo, gl_InstanceID * 4 + 3);\n"
        "\n"
        "    // Now assemble the four columns into a matrix.\n"
        "    mat4 model_matrix = mat4(col1, col2, col3, col4);\n"
        "\n"
        "    // Construct a model-view matrix from the uniform view matrix\n"
        "    // and the per-instance model matrix.\n"
        "    mat4 model_view_matrix = view_matrix * model_matrix;\n"
        "\n"
        "    // Transform position by the model-view matrix, then by the\n"
        "    // projection matrix.\n"
        "    gl_Position = projection_matrix * (model_view_matrix * position);\n"
        "    // Transform the normal by the upper-left-3x3-submatrix of the\n"
        "    // model-view matrix\n"
        "    vertex.normal = mat3(model_view_matrix) * normal;\n"
        "    // Pass the per-instance color through to the fragment shader.\n"
        "    vertex.color = color;\n"
        "}\n";

    static const char render_fs[] =
        "#version 430 core\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "in VERTEX\n"
        "{\n"
        "    vec3    normal;\n"
        "    vec4    color;\n"
        "} vertex;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = vertex.color * (0.1 + abs(vertex.normal.z)) + vec4(0.8, 0.9, 0.7, 1.0) * pow(abs(vertex.normal.z), 40.0);\n"
        "}\n";

    // Compile and link the shader
    vglAttachShaderSource(gProgram, GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram, GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram);
    glUseProgram(gProgram);
    
    // Get location of uniform view_matrix_loc and render_projection_matrix_loc
    view_matrix_loc = glGetUniformLocation(gProgram, "view_matrix");
    render_projection_matrix_loc = glGetUniformLocation(gProgram, "projection_matrix");
      
    // Set up the TBO samplers so that shader knows where to fetch color and model matrix from the data stored as texture
    GLuint color_tbo_loc = glGetUniformLocation(gProgram, "color_tbo");
    GLuint model_matrix_tbo_loc = glGetUniformLocation(gProgram, "model_matrix_tbo");

    // Set them to the right texture unit indices, since color is uploaded as GL_TEXTURE0, model_matrix is as GL_TEXTURE1
    glUniform1i(color_tbo_loc, 0);
    glUniform1i(model_matrix_tbo_loc, 1);
    
    // Load the VBM object
    vbmObj.LoadFromVBM("armadillo_low.vbm", 0, 1, 2);

    // Now we set up the TBOs for the instance colors and the model matrices

    // Create a texture name, active and bind it as current.
    glGenTextures(1, TO_color);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, TO_color[0]);
    
    // Generate the colors of the objects for each instance.
    vec4 colors[INSTANCE_COUNT];

    for (int n = 0; n < INSTANCE_COUNT; n++)
    {
        float a = float(n) / 4.0f;
        float b = float(n) / 5.0f;
        float c = float(n) / 6.0f;

        colors[n][0] = 0.5f + 0.25f * (sinf(a + 1.0f) + 1.0f);
        colors[n][1] = 0.5f + 0.25f * (sinf(b + 2.0f) + 1.0f);
        colors[n][2] = 0.5f + 0.25f * (sinf(c + 3.0f) + 1.0f);
        colors[n][3] = 1.0f;
    }
    
    // Create a texture buffer object and upload data to it.
    glGenBuffers(1, TBO_color);
    glBindBuffer(GL_TEXTURE_BUFFER, TBO_color[0]);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    
    // Attach the texture buffer object data to the texture object
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, TBO_color[0]);
    
    // Now do the same thing with a TBO for the model matrices. The buffer object
    // (TBO_model_matrix) has been created and sized to store one mat4 per-
    // instance.
    glGenTextures(1, TO_model_matrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, TO_model_matrix[0]);
    glGenBuffers(1, TBO_model_matrix);
    glBindBuffer(GL_TEXTURE_BUFFER, TBO_model_matrix[0]);
    glBufferData(GL_TEXTURE_BUFFER, INSTANCE_COUNT * sizeof(mat4), NULL, GL_DYNAMIC_DRAW); // Don't upload the data since we'll update it in display call
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, TBO_model_matrix[0]);
    
    glClearColor(0, 0, 0, 1);
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
    float t = float(GetTickCount() & 0x3FFFF) / float(0x3FFFF);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Set model matrices for each instance
    mat4 matrices[INSTANCE_COUNT];

    for (int n = 0; n < INSTANCE_COUNT; n++)
    {
        float a = 50.0f * float(n) / 4.0f;
        float b = 50.0f * float(n) / 5.0f;
        float c = 50.0f * float(n) / 6.0f;

        matrices[n] = rotate(a + t * 360.0f, 1.0f, 0.0f, 0.0f) *
                      rotate(b + t * 360.0f, 0.0f, 1.0f, 0.0f) *
                      rotate(c + t * 360.0f, 0.0f, 0.0f, 1.0f) *
                      translate(10.0f + a, 40.0f + b, 50.0f + c);
    }
    
    // Bind the TBO for model_matrix and change its data, since we want the model matrix updated at each display call
    glActiveTexture(GL_TEXTURE1);
    glBindBuffer(GL_TEXTURE_BUFFER, TBO_model_matrix[0]);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(matrices), matrices, GL_DYNAMIC_DRAW);
    
    // Use shader program
    glUseProgram(gProgram);
    
    // Set up the view and projection matrices
    mat4 view_matrix(translate(0.0f, 0.0f, -1500.0f) * rotate(t * 360.0f * 2.0f, 0.0f, 1.0f, 0.0f));
    mat4 projection_matrix(frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5000.0f));

    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    // Render INSTANCE_COUNT objects
    vbmObj.Render(0, INSTANCE_COUNT);
    
    glutSwapBuffers();
    if (auto_redraw)
    {
        glutPostRedisplay();
    }
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