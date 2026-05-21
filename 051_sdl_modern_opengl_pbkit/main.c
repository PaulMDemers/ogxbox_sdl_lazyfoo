#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <math.h>
#include <pbkit/pbkit.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#define LAZYFOO_LESSON 51
#define MASK(mask, val) (((val) << (ffs(mask) - 1)) & (mask))

typedef struct Vertex {
    float pos[3];
    float color[3];
} __attribute__((packed)) Vertex;

static const Vertex quad_vertices[] = {
    {{-0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{-0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    {{-0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}},
};

static uint8_t *gpu_vertices;
static float viewport_matrix[4][4];

static void matrix_viewport(float out[4][4], float x, float y, float width, float height, float z_min, float z_max)
{
    memset(out, 0, 16 * sizeof(float));
    out[0][0] = width / 2.0f;
    out[1][1] = height / -2.0f;
    out[2][2] = z_max - z_min;
    out[3][3] = 1.0f;
    out[3][0] = x + width / 2.0f;
    out[3][1] = y + height / 2.0f;
    out[3][2] = z_min;
}

static void init_shader(void)
{
    uint32_t *p;
    uint32_t vs_program[] = {
        #include "vs.inl"
    };

    p = pb_begin();
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_START, 0);
    p = pb_push1(p, NV097_SET_TRANSFORM_EXECUTION_MODE,
                 MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM) |
                 MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE, NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_CXT_WRITE_EN, 0);
    pb_end(p);

    p = pb_begin();
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_LOAD, 0);
    pb_end(p);

    for (unsigned int i = 0; i < sizeof(vs_program) / 16; ++i) {
        p = pb_begin();
        pb_push(p++, NV097_SET_TRANSFORM_PROGRAM, 4);
        memcpy(p, &vs_program[i * 4], 16);
        p += 4;
        pb_end(p);
    }

    p = pb_begin();
    #include "ps.inl"
    pb_end(p);
}

static void set_attrib_pointer(unsigned int index, unsigned int format, unsigned int size, unsigned int stride, const void *data)
{
    uint32_t *p = pb_begin();
    p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + index * 4,
                 MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE, format) |
                 MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, size) |
                 MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, stride));
    p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_OFFSET + index * 4, (uint32_t)data & 0x03ffffff);
    pb_end(p);
}

static void setup_render_state(void)
{
    uint32_t *p = pb_begin();
    p = pb_push1(p, NV097_SET_DEPTH_TEST_ENABLE, 1);
    p = pb_push1(p, NV097_SET_DEPTH_FUNC, NV097_SET_DEPTH_FUNC_V_LEQUAL);
    p = pb_push1(p, NV097_SET_DEPTH_MASK, 1);
    p = pb_push1(p, NV097_SET_CULL_FACE_ENABLE, 0);
    pb_end(p);
}

static void upload_viewport(void)
{
    uint32_t *p = pb_begin();
    p = pb_push1(p, NV097_SET_TRANSFORM_CONSTANT_LOAD, 96);
    pb_push(p++, NV097_SET_TRANSFORM_CONSTANT, 16);
    memcpy(p, viewport_matrix, 16 * sizeof(float));
    p += 16;
    pb_end(p);
}

static void clear_attribs(void)
{
    uint32_t *p = pb_begin();
    pb_push(p++, NV097_SET_VERTEX_DATA_ARRAY_FORMAT, 16);
    for (unsigned int i = 0; i < 16; ++i) {
        *(p++) = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F;
    }
    pb_end(p);
}

static void draw_quad(void)
{
    uint32_t *p;

    set_attrib_pointer(0, NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F,
                       3, sizeof(Vertex), gpu_vertices + offsetof(Vertex, pos));
    set_attrib_pointer(3, NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F,
                       3, sizeof(Vertex), gpu_vertices + offsetof(Vertex, color));

    p = pb_begin();
    p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_TRIANGLES);
    p = pb_push1(p, 0x40000000 | NV097_DRAW_ARRAYS,
                 MASK(NV097_DRAW_ARRAYS_COUNT, 5) |
                 MASK(NV097_DRAW_ARRAYS_START_INDEX, 0));
    p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
    pb_end(p);
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    if (pb_init() != 0) {
        debugPrint("pb_init failed\n");
        Sleep(2000);
        return 1;
    }

    init_shader();
    setup_render_state();
    gpu_vertices = MmAllocateContiguousMemoryEx(sizeof(quad_vertices), 0, 0x3ffb000, 0, PAGE_READWRITE | PAGE_WRITECOMBINE);
    if (gpu_vertices == NULL) {
        debugPrint("vertex allocation failed\n");
        Sleep(2000);
        return 1;
    }
    memcpy(gpu_vertices, quad_vertices, sizeof(quad_vertices));
    matrix_viewport(viewport_matrix, 0.0f, 0.0f, (float)pb_back_buffer_width(), (float)pb_back_buffer_height(), 0.0f, 65536.0f);

    pb_show_front_screen();

    for (;;) {
        pb_wait_for_vbl();
        pb_reset();
        pb_target_back_buffer();
        pb_erase_depth_stencil_buffer(0, 0, pb_back_buffer_width(), pb_back_buffer_height());
        pb_fill(0, 0, pb_back_buffer_width(), pb_back_buffer_height(), 0x00000000);
        pb_erase_text_screen();

        while (pb_busy()) {
        }

        upload_viewport();
        clear_attribs();
        draw_quad();

        pb_print("Lazy Foo 51 PBKit\n");
        pb_print("shader/vertex-buffer style quad\n");
        pb_draw_text_screen();

        while (pb_busy()) {
        }
        while (pb_finished()) {
        }
    }

    MmFreeContiguousMemory(gpu_vertices);
    pb_kill();
    return 0;
}
