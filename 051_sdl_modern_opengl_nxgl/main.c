#include <hal/video.h>
#include <stdint.h>
#include <windows.h>

#include "nxgl.h"

#define LAZYFOO_LESSON 51

static const GLfloat vertex_data[] = {
    -1.47f, -1.10f,
     1.47f, -1.10f,
     1.47f,  1.10f,
    -1.47f,  1.10f,
};

static const GLuint index_data[] = {
    0, 1, 2, 3,
};

static void reset_gl(void)
{
    glViewport(0, 0, 640, 480);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

static void render_indexed_quad(void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertex_data);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, index_data);
    glDisableClientState(GL_VERTEX_ARRAY);
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    if (nxglInit() != 0) {
        return 1;
    }

    for (;;) {
        reset_gl();
        glClear(GL_COLOR_BUFFER_BIT);
        render_indexed_quad();
        nxglSwapBuffers("Lazy Foo 51 NXGL", "buffer-style indexed quad");
        Sleep(16);
    }

    nxglShutdown();
    return 0;
}
