#include <iostream>
#undef __WIN32
#include "vgl.h"
#include "vutils.h"
#include "vmath.h"
#include "vbm.h"
#include "GL/freeglut.h"

using namespace std;

GLuint VBO_color[1]; // VBO for color
GLuint VBO_model_matrix[1];// VBO for model matrix

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
        "// model_matrix will be used as a per-instance transformation\n"
        "// matrix. Note that a mat4 consumes 4 consecutive locations, so\n"
        "// this will actually sit in locations, 3, 4, 5, and 6.\n"
        "layout (location = 4) in mat4 model_matrix;\n"
        "\n"
        "// The view matrix and the projection matrix are constant across a draw\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
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
    
    // Load the VBM object
    vbmObj.LoadFromVBM("armadillo_low.vbm", 0, 1, 2);
    
    // Bind VAO so that we can append the instanced attributes to VBO
    vbmObj.BindVertexArray();
    
    // Get the locations of the vertex attributes color and model_matrix
    // For color, we will upload data once.
    // For model_matrix, we will update at each display call
    // position and normal attributes are already enabled in vbmObj.
    int color_loc       = glGetAttribLocation(gProgram, "color");
    int model_matrix_loc      = glGetAttribLocation(gProgram, "model_matrix");
    
    // Generate and upload the colors to VBO
    vmath::vec4 colors[INSTANCE_COUNT];

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
    
    glGenBuffers(1, VBO_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    
    // Now we set up the color attribute. We want each instance of our geometry
    // to assume a different color, so we'll just pack colors into a buffer
    // object and make an instanced vertex attribute out of it.
    glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(color_loc);
    
    // This is the important bit... set the divisor for the color array to
    // 1 to get OpenGL to give us a new value of 'color' per-instance
    // rather than per-vertex.
    glVertexAttribDivisor(color_loc, 1);
    
    // Likewise, we can do the same with the model matrix. Note that a
    // matrix input to the vertex shader consumes N consecutive input
    // locations, where N is the number of columns in the matrix. So...
    // we have four vertex attributes to set up.
    glGenBuffers(1, VBO_model_matrix);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_matrix[0]);
    glBufferData(GL_ARRAY_BUFFER, INSTANCE_COUNT * sizeof(vmath::mat4), NULL, GL_DYNAMIC_DRAW);// Don't upload the data since we'll update it in display call
    
    // Loop over each column of the matrix...
    for (int i = 0; i < 4; i++)
    {
        // Set up the vertex attribute
        glVertexAttribPointer(model_matrix_loc + i,              // Location
                              4, GL_FLOAT, GL_FALSE,       // vec4
                              sizeof(vmath::mat4),                // Stride
                              (void *)(sizeof(vmath::vec4) * i)); // Start offset
        // Enable it
        glEnableVertexAttribArray(model_matrix_loc + i);
        // Make it instanced
        glVertexAttribDivisor(model_matrix_loc + i, 1);
    }
    
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
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Bind the model matrix VBO and change its data
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_matrix[0]);
    
    // Set model matrices for each instance
    vmath::mat4 * matrices = (vmath::mat4 *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    for (int n = 0; n < INSTANCE_COUNT; n++)
    {
        float a = 50.0f * float(n) / 4.0f;
        float b = 50.0f * float(n) / 5.0f;
        float c = 50.0f * float(n) / 6.0f;

        matrices[n] = vmath::rotate(a + t * 360.0f, 1.0f, 0.0f, 0.0f) *
                      vmath::rotate(b + t * 360.0f, 0.0f, 1.0f, 0.0f) *
                      vmath::rotate(c + t * 360.0f, 0.0f, 0.0f, 1.0f) *
                      vmath::translate(10.0f + a, 40.0f + b, 50.0f + c);
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    
    // Use shader program
    glUseProgram(gProgram);
    
    // Set up the view and projection matrices
    vmath::mat4 view_matrix(vmath::translate(0.0f, 0.0f, -1500.0f) * vmath::rotate(t * 360.0f * 2.0f, 0.0f, 1.0f, 0.0f));
    vmath::mat4 projection_matrix(vmath::frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 5000.0f));

    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    // Render INSTANCE_COUNT objects
    vbmObj.Render(0, INSTANCE_COUNT);
    
    glutSwapBuffers();
    if (auto_redraw)
    {
        glutPostRedisplay();
    }
    
    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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