#include <hal/video.h>
#include <windows.h>

#include "nxgl.h"

#define LAZYFOO_LESSON 50

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

static void render_quad(void)
{
    glBegin(GL_QUADS);
        glVertex2f(-1.47f, -1.10f);
        glVertex2f( 1.47f, -1.10f);
        glVertex2f( 1.47f,  1.10f);
        glVertex2f(-1.47f,  1.10f);
    glEnd();
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
        render_quad();
        nxglSwapBuffers("Lazy Foo 50 NXGL", "SDL/OpenGL 2 fixed-function quad");
        Sleep(16);
    }

    nxglShutdown();
    return 0;
}
