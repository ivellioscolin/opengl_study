#include <iostream>
#undef __WIN32
#include "vgl.h"
#include "vutils.h"
#include "vmath.h"
#include "vbm.h"
#include "GL/freeglut.h"

using namespace std;

GLuint VBO_weight[1]; // Vertex Buffer Object for weight of each instance
GLuint VBO_color[1]; // VBO for color of each instance

GLuint gProgram = 0;

float aspect;
GLint render_model_matrix_loc;
GLint render_projection_matrix_loc;

VBObject vbmObj;

#define INSTANCE_COUNT 200

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
        // Uniforms
        "uniform mat4 model_matrix[4];\n"
        "uniform mat4 projection_matrix;\n"
        "\n"
        // Regular vertex attributes
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec3 normal;\n"
        "\n"
        // Instanced vertex attributes
        "layout (location = 3) in vec4 instance_weights;\n"
        "layout (location = 4) in vec4 instance_color;\n"
        "\n"
        // Outputs to the fragment shader
        "out vec3 vs_fs_normal;\n"
        "out vec4 vs_fs_color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    int n;\n"
        "    mat4 m = mat4(0.0);\n"
        "    vec4 pos = position;\n"
        // Normalize the weights so that their sum total is 1.0
        "    vec4 weights = normalize(instance_weights);\n"
        "    for (n = 0; n < 4; n++)\n"
        "    {\n"
        // Calulate a weighted average of the matrices
        "        m += (model_matrix[n] * weights[n]);\n"
        "    }\n"
        // Use that calculated matrix to transform the object.
        "    vs_fs_normal = normalize((m * vec4(normal, 0.0)).xyz);\n"
        "    vs_fs_color = instance_color;\n"
        "    gl_Position = projection_matrix * (m * pos);\n"
        "}\n";

    static const char render_fs[] =
        "#version 430 core\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "in vec3 vs_fs_normal;\n"
        "in vec4 vs_fs_color;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    color = vs_fs_color * (0.1 + abs(vs_fs_normal.z)) + vec4(0.8, 0.9, 0.7, 1.0) * pow(abs(vs_fs_normal.z), 40.0);\n"
        "}\n";
    // Compile and link the shader
    vglAttachShaderSource(gProgram, GL_VERTEX_SHADER, render_vs);
    vglAttachShaderSource(gProgram, GL_FRAGMENT_SHADER, render_fs);
    glLinkProgram(gProgram);
    glUseProgram(gProgram);
    
    // Get location of uniform. "model_matrix" is start of a matrix array
    render_model_matrix_loc = glGetUniformLocation(gProgram, "model_matrix");
    render_projection_matrix_loc = glGetUniformLocation(gProgram, "projection_matrix");
    
    // Load the VBM object
    vbmObj.LoadFromVBM("armadillo_low.vbm", 0, 1, 2);
    
    // Bind VAO
    vbmObj.BindVertexArray();
    
    // Generate the colors of the objects
    vmath::vec4 colors[INSTANCE_COUNT];

    for (int n = 0; n < INSTANCE_COUNT; n++)
    {
        float a = float(n) / 4.0f;
        float b = float(n) / 5.0f;
        float c = float(n) / 6.0f;

        colors[n][0] = 0.5f * (sinf(a + 1.0f) + 1.0f);
        colors[n][1] = 0.5f * (sinf(b + 2.0f) + 1.0f);
        colors[n][2] = 0.5f * (sinf(c + 3.0f) + 1.0f);
        colors[n][3] = 1.0f;
    }
    
    // Create and allocate the VBO to hold the weights
    // Notice that we use the 'colors' array as the initial data, but only because
    // we know it's the same size.
    glGenBuffers(1, VBO_weight);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_weight[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);
    
    // Here is the instanced vertex attribute - set the divisor to update for each instance
    glVertexAttribDivisor(glGetAttribLocation(gProgram, "instance_weights"), 1);
    // It's otherwise the same as any other vertex attribute - set the pointer and enable it
    glVertexAttribPointer(glGetAttribLocation(gProgram, "instance_weights"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "instance_weights"));
    
    // Do the same for VBO of color
    glGenBuffers(1, VBO_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glVertexAttribDivisor(glGetAttribLocation(gProgram, "instance_color"), 1);
    glVertexAttribPointer(glGetAttribLocation(gProgram, "instance_color"), 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(glGetAttribLocation(gProgram, "instance_color"));
    
    glClearColor(0, 0, 0, 1);
    
    // Unbind VAO, VBO
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
    float t = float(GetTickCount() & 0x3FFFF) / float(0x3FFFF);
    
    // Set weights for each instance
    vmath::vec4 weights[INSTANCE_COUNT];

    for (int n = 0; n < INSTANCE_COUNT; n++)
    {
        float a = float(n) / 4.0f;
        float b = float(n) / 5.0f;
        float c = float(n) / 6.0f;

        weights[n][0] = 0.5f * (sinf(t * 6.28318531f * 8.0f + a) + 1.0f);
        weights[n][1] = 0.5f * (sinf(t * 6.28318531f * 26.0f + b) + 1.0f);
        weights[n][2] = 0.5f * (sinf(t * 6.28318531f * 21.0f + c) + 1.0f);
        weights[n][3] = 0.5f * (sinf(t * 6.28318531f * 13.0f + a + b) + 1.0f);
    }
    
    // Bind and update weight VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_weight[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(weights), weights, GL_DYNAMIC_DRAW);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Use shader program
    glUseProgram(gProgram);
    
    // Set four model matrices
    vmath::mat4 model_matrix[4];

    for (int n = 0; n < 4; n++)
    {
        model_matrix[n] = (vmath::scale(5.0f) *
                           vmath::rotate(t * 360.0f * 40.0f + float(n + 1) * 29.0f, 0.0f, 1.0f, 0.0f) *
                           vmath::rotate(t * 360.0f * 20.0f + float(n + 1) * 35.0f, 0.0f, 0.0f, 1.0f) *
                           vmath::rotate(t * 360.0f * 30.0f + float(n + 1) * 67.0f, 0.0f, 1.0f, 0.0f) *
                           vmath::translate((float)n * 10.0f - 15.0f, 0.0f, 0.0f) *
                           vmath::scale(0.01f));
    }
    
    glUniformMatrix4fv(render_model_matrix_loc, 4, GL_FALSE, model_matrix[0]);
    
    // Set up the projection matrix
    vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5000.0f) * vmath::translate(0.0f, 0.0f, -100.0f));
    
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