#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>

#define LAZYFOO_LESSON 50

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    if (pb_init() != 0) {
        Sleep(2000);
        return 1;
    }

    pb_show_front_screen();

    for (;;) {
        pb_wait_for_vbl();
        pb_target_back_buffer();
        pb_reset();
        pb_fill(0, 0, 640, 480, 0x00000000);
        pb_fill(160, 120, 320, 240, 0x00FFFFFF);

        pb_erase_text_screen();
        pb_print("Lazy Foo 50 PBKit\n");
        pb_print("SDL/OpenGL 2 fixed-function quad target\n");
        pb_draw_text_screen();

        while (pb_busy()) {
        }
        while (pb_finished()) {
        }
    }

    pb_kill();
    return 0;
}
