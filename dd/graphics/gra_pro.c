/*
 * OpenGL 3.3 Core + GLFW — 2D (triangle, square) + 3D (cube)
 * Transformations: translation, rotation, scaling — keyboard driven.
 *
 * Build: gcc -Wall -Wextra -O2 -o gra_demo main.c -lglfw -lGL -lm
 * (Uses glfwGetProcAddress — no GLEW required.)
 */

#define _POSIX_C_SOURCE 200809L
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
/* Expose OpenGL 1.x prototypes (viewport, clear, …) for clean compilation; symbols come from -lGL. */
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Core-profile functions are not linked from libGL on all setups; load once after context creation.
 * Pointers use a trailing underscore; macros remap glFoo → glFoo_ for the rest of this file.
 */
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_;
static PFNGLGENBUFFERSPROC glGenBuffers_;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_;
static PFNGLBINDBUFFERPROC glBindBuffer_;
static PFNGLBUFFERDATAPROC glBufferData_;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_;
static PFNGLCREATESHADERPROC glCreateShader_;
static PFNGLSHADERSOURCEPROC glShaderSource_;
static PFNGLCOMPILESHADERPROC glCompileShader_;
static PFNGLGETSHADERIVPROC glGetShaderiv_;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_;
static PFNGLCREATEPROGRAMPROC glCreateProgram_;
static PFNGLATTACHSHADERPROC glAttachShader_;
static PFNGLLINKPROGRAMPROC glLinkProgram_;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_;
static PFNGLDELETESHADERPROC glDeleteShader_;
static PFNGLUSEPROGRAMPROC glUseProgram_;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_;
static PFNGLUNIFORM3FPROC glUniform3f_;
static PFNGLDRAWARRAYSPROC glDrawArrays_;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers_;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_;

#define glGenVertexArrays glGenVertexArrays_
#define glGenBuffers glGenBuffers_
#define glBindVertexArray glBindVertexArray_
#define glBindBuffer glBindBuffer_
#define glBufferData glBufferData_
#define glVertexAttribPointer glVertexAttribPointer_
#define glEnableVertexAttribArray glEnableVertexAttribArray_
#define glCreateShader glCreateShader_
#define glShaderSource glShaderSource_
#define glCompileShader glCompileShader_
#define glGetShaderiv glGetShaderiv_
#define glGetShaderInfoLog glGetShaderInfoLog_
#define glCreateProgram glCreateProgram_
#define glAttachShader glAttachShader_
#define glLinkProgram glLinkProgram_
#define glGetProgramiv glGetProgramiv_
#define glGetProgramInfoLog glGetProgramInfoLog_
#define glDeleteShader glDeleteShader_
#define glUseProgram glUseProgram_
#define glGetUniformLocation glGetUniformLocation_
#define glUniformMatrix4fv glUniformMatrix4fv_
#define glUniform3f glUniform3f_
#define glDrawArrays glDrawArrays_
#define glDeleteVertexArrays glDeleteVertexArrays_
#define glDeleteBuffers glDeleteBuffers_
#define glDeleteProgram glDeleteProgram_

static int load_gl3_functions(void) {
#define L(sym, Type)                                                                 \
    do {                                                                             \
        gl##sym##_ = (Type)glfwGetProcAddress("gl" #sym);                           \
        if (!gl##sym##_) {                                                           \
            fprintf(stderr, "OpenGL: missing gl" #sym "\n");                         \
            return -1;                                                               \
        }                                                                            \
    } while (0)

    L(GenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
    L(GenBuffers, PFNGLGENBUFFERSPROC);
    L(BindVertexArray, PFNGLBINDVERTEXARRAYPROC);
    L(BindBuffer, PFNGLBINDBUFFERPROC);
    L(BufferData, PFNGLBUFFERDATAPROC);
    L(VertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    L(EnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    L(CreateShader, PFNGLCREATESHADERPROC);
    L(ShaderSource, PFNGLSHADERSOURCEPROC);
    L(CompileShader, PFNGLCOMPILESHADERPROC);
    L(GetShaderiv, PFNGLGETSHADERIVPROC);
    L(GetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
    L(CreateProgram, PFNGLCREATEPROGRAMPROC);
    L(AttachShader, PFNGLATTACHSHADERPROC);
    L(LinkProgram, PFNGLLINKPROGRAMPROC);
    L(GetProgramiv, PFNGLGETPROGRAMIVPROC);
    L(GetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
    L(DeleteShader, PFNGLDELETESHADERPROC);
    L(UseProgram, PFNGLUSEPROGRAMPROC);
    L(GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    L(UniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
    L(Uniform3f, PFNGLUNIFORM3FPROC);
    L(DrawArrays, PFNGLDRAWARRAYSPROC);
    L(DeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
    L(DeleteBuffers, PFNGLDELETEBUFFERSPROC);
    L(DeleteProgram, PFNGLDELETEPROGRAMPROC);
#undef L
    return 0;
}

/* ---------- Minimal math (column-major 4x4, OpenGL convention) ---------- */

static void mat4_identity(float out[16]) {
    memset(out, 0, 16 * sizeof(float));
    out[0] = out[5] = out[10] = out[15] = 1.0f;
}

/* Multiply: out = A * B (column-major). Transforms apply right-to-left on vectors. */
static void mat4_mul(float out[16], const float a[16], const float b[16]) {
    float r[16];
    for (int c = 0; c < 4; ++c) {
        for (int rrow = 0; rrow < 4; ++rrow) {
            r[c * 4 + rrow] =
                a[0 * 4 + rrow] * b[c * 4 + 0] +
                a[1 * 4 + rrow] * b[c * 4 + 1] +
                a[2 * 4 + rrow] * b[c * 4 + 2] +
                a[3 * 4 + rrow] * b[c * 4 + 3];
        }
    }
    memcpy(out, r, sizeof(r));
}

static void mat4_translate(float out[16], float tx, float ty, float tz) {
    mat4_identity(out);
    out[12] = tx;
    out[13] = ty;
    out[14] = tz;
}

static void mat4_scale(float out[16], float sx, float sy, float sz) {
    mat4_identity(out);
    out[0] = sx;
    out[5] = sy;
    out[10] = sz;
}

/* Rotation about Y axis (degrees) — common for spinning objects */
static void mat4_rotate_y(float out[16], float degrees) {
    float rad = degrees * (float)M_PI / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);
    mat4_identity(out);
    out[0] = c;
    out[2] = s;
    out[8] = -s;
    out[10] = c;
}

/* Rotation about X axis (degrees) */
static void mat4_rotate_x(float out[16], float degrees) {
    float rad = degrees * (float)M_PI / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);
    mat4_identity(out);
    out[5] = c;
    out[6] = s;
    out[9] = -s;
    out[10] = c;
}

/*
 * Perspective projection: vertical FOV in degrees, aspect = width/height
 * Maps frustum to clip space [-w,w] for x,y and [0,w] for z (GL depth range handled by glDepthRange)
 */
static void mat4_perspective(float out[16], float fovy_deg, float aspect, float near_z, float far_z) {
    float fovy = fovy_deg * (float)M_PI / 180.0f;
    float f = 1.0f / tanf(fovy * 0.5f);
    memset(out, 0, 16 * sizeof(float));
    out[0] = f / aspect;
    out[5] = f;
    out[10] = (far_z + near_z) / (near_z - far_z);
    out[11] = -1.0f;
    out[14] = (2.0f * far_z * near_z) / (near_z - far_z);
}

/* Orthographic (good for “pure” 2D feel): maps [left,right] x [bottom,top] x [near,far] to clip volume */
static void mat4_ortho(float out[16], float left, float right, float bottom, float top,
                      float near_z, float far_z) {
    memset(out, 0, 16 * sizeof(float));
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (far_z - near_z);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(far_z + near_z) / (far_z - near_z);
    out[15] = 1.0f;
}

/* ---------- Shader sources (GLSL runs on GPU) ---------- */

static const char *vs_src =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n"
    "}\n";

static const char *fs_src =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    FragColor = vec4(uColor, 1.0);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(sh, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        exit(EXIT_FAILURE);
    }
    return sh;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

/* ---------- Global state for keyboard & drawing ---------- */

typedef enum { SHAPE_TRIANGLE = 0, SHAPE_SQUARE, SHAPE_CUBE } ShapeMode;

static struct {
    ShapeMode shape;
    float tx, ty, tz;       /* translation (world) */
    float rot_x, rot_y;     /* degrees */
    float sx, sy, sz;       /* scale */
    int width, height;
    int use_perspective;    /* 0 = ortho (2D modes), 1 = perspective (cube) */
} g;

static void reset_transforms(void) {
    g.tx = g.ty = g.tz = 0.0f;
    g.rot_x = g.rot_y = 0.0f;
    g.sx = g.sy = g.sz = 1.0f;
}

static void framebuffer_size_cb(GLFWwindow *win, int w, int h) {
    (void)win;
    g.width = w > 0 ? w : 1;
    g.height = h > 0 ? h : 1;
    glViewport(0, 0, g.width, g.height);
}

static void key_cb(GLFWwindow *win, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    const float step_t = 0.08f;
    const float step_r = 3.0f;
    const float step_s = 0.05f;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(win, GLFW_TRUE);
        break;
    case GLFW_KEY_1:
        g.shape = SHAPE_TRIANGLE;
        g.use_perspective = 0;
        reset_transforms();
        break;
    case GLFW_KEY_2:
        g.shape = SHAPE_SQUARE;
        g.use_perspective = 0;
        reset_transforms();
        break;
    case GLFW_KEY_3:
        g.shape = SHAPE_CUBE;
        g.use_perspective = 1;
        reset_transforms();
        break;
    /* Move (translation) */
    case GLFW_KEY_W:
        g.ty += step_t;
        break;
    case GLFW_KEY_S:
        g.ty -= step_t;
        break;
    case GLFW_KEY_A:
        g.tx -= step_t;
        break;
    case GLFW_KEY_D:
        g.tx += step_t;
        break;
    case GLFW_KEY_Q:
        g.tz += step_t;
        break;
    case GLFW_KEY_E:
        g.tz -= step_t;
        break;
    /* Rotate */
    case GLFW_KEY_J:
        g.rot_y -= step_r;
        break;
    case GLFW_KEY_L:
        g.rot_y += step_r;
        break;
    case GLFW_KEY_I:
        g.rot_x -= step_r;
        break;
    case GLFW_KEY_K:
        g.rot_x += step_r;
        break;
    /* Scale (uniform for simplicity) */
    case GLFW_KEY_EQUAL: /* + key often needs shift; also support keypad */
    case GLFW_KEY_KP_ADD:
        g.sx += step_s;
        g.sy += step_s;
        g.sz += step_s;
        break;
    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT:
        g.sx = fmaxf(0.1f, g.sx - step_s);
        g.sy = fmaxf(0.1f, g.sy - step_s);
        g.sz = fmaxf(0.1f, g.sz - step_s);
        break;
    case GLFW_KEY_R:
        reset_transforms();
        break;
    default:
        break;
    }
}

static void print_help(void) {
    printf(
        "\n=== OpenGL demo controls ===\n"
        "  1 / 2 / 3     : Triangle / Square / Cube\n"
        "  W A S D       : Translate in X/Y (2D plane)\n"
        "  Q / E         : Translate in Z (depth; useful in 3D)\n"
        "  I K / J L     : Rotate X / Y (degrees)\n"
        "  +/- (or keypad): Scale up / down\n"
        "  R             : Reset transforms\n"
        "  ESC           : Quit\n\n");
}

int main(void) {
    print_help();

    /* Initialize GLFW: window + context creation */
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return EXIT_FAILURE;
    }
    /* Request OpenGL 3.3 Core: programmable pipeline, no fixed-function */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(900, 600, "gra_pro — 2D/3D OpenGL", NULL, NULL);
    if (!window) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); /* VSync: ties frame rate to display (double buffer swap) */

    /* Resolve OpenGL 3.3 entry points (core profile) after context is current */
    if (load_gl3_functions() != 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glGetError(); /* clear sticky error from driver if any */

    glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);
    glfwSetKeyCallback(window, key_cb);
    glfwGetFramebufferSize(window, &g.width, &g.height);

    /* Depth test: fragments must pass depth comparison to be visible (essential for 3D) */
    glEnable(GL_DEPTH_TEST);

    GLuint program = link_program(
        compile_shader(GL_VERTEX_SHADER, vs_src),
        compile_shader(GL_FRAGMENT_SHADER, fs_src));

    GLint loc_mvp = glGetUniformLocation(program, "uMVP");
    GLint loc_color = glGetUniformLocation(program, "uColor");

    /* --- Triangle: 3 vertices in XY plane (Z=0) --- */
    float triVerts[] = {
        -0.5f, -0.4f, 0.0f,
        0.5f, -0.4f, 0.0f,
        0.0f, 0.5f, 0.0f,
    };
    GLuint vao_tri, vbo_tri;
    glGenVertexArrays(1, &vao_tri);
    glGenBuffers(1, &vbo_tri);
    glBindVertexArray(vao_tri);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVerts), triVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    /* --- Square: two triangles, 6 vertices (same Z=0 plane) --- */
    float sqVerts[] = {
        -0.45f, -0.45f, 0.0f,
        0.45f, -0.45f, 0.0f,
        0.45f, 0.45f, 0.0f,
        -0.45f, -0.45f, 0.0f,
        0.45f, 0.45f, 0.0f,
        -0.45f, 0.45f, 0.0f,
    };
    GLuint vao_sq, vbo_sq;
    glGenVertexArrays(1, &vao_sq);
    glGenBuffers(1, &vbo_sq);
    glBindVertexArray(vao_sq);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sq);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sqVerts), sqVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    /*
     * --- Cube: 36 vertices (6 faces × 2 triangles × 3 verts), centered at origin
     * Each face has outward-facing winding for back-face culling if enabled.
     */
    float cubeVerts[] = {
        /* +Z */
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        /* -Z */
        0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        /* +X */
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        /* -X */
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        /* +Y */
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        /* -Y */
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
    };
    GLuint vao_cube, vbo_cube;
    glGenVertexArrays(1, &vao_cube);
    glGenBuffers(1, &vbo_cube);
    glBindVertexArray(vao_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    /* Default mode */
    g.shape = SHAPE_TRIANGLE;
    g.use_perspective = 0;
    reset_transforms();

    while (!glfwWindowShouldClose(window)) {
        /* Clear color and depth buffers each frame */
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        /* Model matrix: M = T * Rx * Ry * S (scale, then rotate Y/X, then translate) */
        float S[16], Rx[16], Ry[16], T[16], tmp[16], model[16];
        mat4_scale(S, g.sx, g.sy, g.sz);
        mat4_rotate_x(Rx, g.rot_x);
        mat4_rotate_y(Ry, g.rot_y);
        mat4_translate(T, g.tx, g.ty, g.tz);
        mat4_mul(tmp, Ry, S);
        mat4_mul(model, Rx, tmp);
        mat4_mul(tmp, T, model);
        memcpy(model, tmp, sizeof(model));

        float view[16], proj[16], vp[16], mvp[16];
        float aspect = (float)g.width / (float)g.height;

        if (g.use_perspective) {
            /* 3D: perspective + move camera back (translate world -Z in eye space) */
            mat4_perspective(proj, 55.0f, aspect, 0.1f, 50.0f);
            mat4_translate(view, 0.0f, 0.0f, -4.0f);
        } else {
            /* 2D-style ortho: centered, flipped Y optional — here top is +Y like math */
            float half_h = 1.2f;
            float half_w = half_h * aspect;
            mat4_ortho(proj, -half_w, half_w, -half_h, half_h, -5.0f, 5.0f);
            mat4_identity(view);
        }

        mat4_mul(vp, proj, view);
        mat4_mul(mvp, vp, model);

        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, mvp);

        /* Draw current shape */
        switch (g.shape) {
        case SHAPE_TRIANGLE:
            glUniform3f(loc_color, 0.95f, 0.45f, 0.35f);
            glBindVertexArray(vao_tri);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        case SHAPE_SQUARE:
            glUniform3f(loc_color, 0.45f, 0.75f, 0.95f);
            glBindVertexArray(vao_sq);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            break;
        case SHAPE_CUBE:
            glUniform3f(loc_color, 0.65f, 0.85f, 0.45f);
            glBindVertexArray(vao_cube);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            break;
        }
        glBindVertexArray(0);

        /* Swap front/back buffers: double buffering removes tearing */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /* Cleanup (optional before exit; OS reclaims on quit) */
    glDeleteVertexArrays(1, &vao_tri);
    glDeleteVertexArrays(1, &vao_sq);
    glDeleteVertexArrays(1, &vao_cube);
    glDeleteBuffers(1, &vbo_tri);
    glDeleteBuffers(1, &vbo_sq);
    glDeleteBuffers(1, &vbo_cube);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

