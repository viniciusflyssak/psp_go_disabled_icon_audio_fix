/* Short 6x9 notification, bottom-right, drawn after VBlank starts. No display
 * mode changes, swaps, screen clearing or hooks. Rendering remains best-effort:
 * the app can overwrite it and a static screen may retain pixels until redraw. */
#include <pspkernel.h>
#include <pspdisplay.h>
#include "config.h"
#include "osd.h"

/* Original 5x7 glyph table, columns, low bit at top; uppercase only. */
static const unsigned char letters[26][5] = {
    {0x7e,0x09,0x09,0x09,0x7e}, {0x7f,0x49,0x49,0x49,0x36},
    {0x3e,0x41,0x41,0x41,0x22}, {0x7f,0x41,0x41,0x22,0x1c},
    {0x7f,0x49,0x49,0x49,0x41}, {0x7f,0x09,0x09,0x09,0x01},
    {0x3e,0x41,0x49,0x49,0x3a}, {0x7f,0x08,0x08,0x08,0x7f},
    {0x41,0x41,0x7f,0x41,0x41}, {0x20,0x40,0x41,0x3f,0x01},
    {0x7f,0x08,0x14,0x22,0x41}, {0x7f,0x40,0x40,0x40,0x40},
    {0x7f,0x02,0x0c,0x02,0x7f}, {0x7f,0x04,0x08,0x10,0x7f},
    {0x3e,0x41,0x41,0x41,0x3e}, {0x7f,0x09,0x09,0x09,0x06},
    {0x3e,0x41,0x51,0x21,0x5e}, {0x7f,0x09,0x19,0x29,0x46},
    {0x26,0x49,0x49,0x49,0x32}, {0x01,0x01,0x7f,0x01,0x01},
    {0x3f,0x40,0x40,0x40,0x3f}, {0x1f,0x20,0x40,0x20,0x1f},
    {0x3f,0x40,0x38,0x40,0x3f}, {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};
static const unsigned char digits[10][5] = {
    {0x3e,0x51,0x49,0x45,0x3e}, {0x00,0x42,0x7f,0x40,0x00},
    {0x62,0x51,0x49,0x49,0x46}, {0x22,0x41,0x49,0x49,0x36},
    {0x18,0x14,0x12,0x7f,0x10}, {0x27,0x45,0x45,0x45,0x39},
    {0x3e,0x49,0x49,0x49,0x32}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x26,0x49,0x49,0x49,0x3e}
};

static char status[39];
static int status_result;
static unsigned int first_draw;
static int active, drawn;

void audiofix_osd_show(const char *message, int result)
{
    unsigned int i = 0;
    while (message[i] && i < sizeof(status) - 1) {
        status[i] = message[i];
        ++i;
    }
    status[i] = 0;
    status_result = result;
    drawn = 0;
    active = 1;
}

static unsigned int column(char ch, int x)
{
    if (ch >= 'A' && ch <= 'Z')
        return letters[ch - 'A'][x];
    if (ch >= '0' && ch <= '9')
        return digits[ch - '0'][x];
    if (ch == ':')
        return x == 2 ? 0x24 : 0;
    if (ch == '.')
        return x == 2 ? 0x40 : 0;
    if (ch == '-')
        return 0x08;
    return 0;
}

static void line(void *buffer, int stride, int format, const char *text, int length)
{
    int c, x, py, px;
    unsigned int index;
    int origin_x = 480 - 8 - length * 6;
    for (c = 0; c < length; ++c) {
        for (py = 0; py < 9; ++py) {
            for (px = 0; px < 6; ++px) {
                int lit = px < 5 && py < 7 && (column(text[c], px) & (1u << py));
                x = origin_x + c * 6 + px;
                index = (unsigned int)((252 + py) * stride + x);
                if (format == PSP_DISPLAY_PIXEL_FORMAT_8888)
                    ((volatile unsigned int *)buffer)[index] =
                        lit ? 0xffffffffu : 0xff000000u;
                else
                    ((volatile unsigned short *)buffer)[index] =
                        lit ? 0xffffu : (format == PSP_DISPLAY_PIXEL_FORMAT_5551 ?
                        0x8000u : (format == PSP_DISPLAY_PIXEL_FORMAT_4444 ?
                        0xf000u : 0));
            }
        }
    }
}

static void hex8(char *out, unsigned int value)
{
    static const char hex[] = "0123456789ABCDEF";
    unsigned int i;
    for (i = 0; i < 8; ++i)
        out[i] = hex[(value >> (28 - 4 * i)) & 15];
}

static int append(char *out, int used, const char *text)
{
    while (*text && used < 76)
        out[used++] = *text++;
    out[used] = 0;
    return used;
}

int audiofix_osd_tick(int log_result)
{
    void *buffer;
    int stride, format, mode, width, height;
    unsigned int address, bytes, now;
    char popup[77], code[9];
    int length;
    if (!AUDIOFIX_OSD_ENABLED || !active)
        return 0;
    now = sceKernelGetSystemTimeLow();
    if (drawn && now - first_draw >= AUDIOFIX_OSD_US) {
        active = 0;
        return 0;
    }
    if (sceDisplayGetMode(&mode, &width, &height) < 0 || mode != PSP_DISPLAY_MODE_LCD ||
        width != 480 || height != 272)
        return 0;
    /* One draw per vblank while visible, rather than drawing 50 times/s over
     * a 60 Hz renderer. The worker skips its normal delay on this path. */
    if (sceDisplayWaitVblankStart() < 0)
        return 0;
    if (sceDisplayGetFrameBuf(&buffer, &stride, &format, PSP_DISPLAY_SETBUF_IMMEDIATE) < 0 ||
        !buffer || stride < 480 || stride > 2048 || format < 0 || format > 3)
        return 1;
    address = (unsigned int)buffer & 0x1fffffffu;
    bytes = (unsigned int)stride * 272u * (format == 3 ? 4u : 2u);
    if ((address & 3u) || address < 0x04000000u || address >= 0x04200000u ||
        bytes > 0x04200000u - address)
        return 1;
    /* Uncached alias of validated VRAM; application buffers are not owned. */
    buffer = (void *)(address | 0x40000000u);
    length = append(popup, 0, "AUDIOFIX 1.2 ");
    length = append(popup, length, status);
    if (status_result < 0) {
        hex8(code, (unsigned int)status_result);
        code[8] = 0;
        length = append(popup, length, " ");
        length = append(popup, length, code);
    }
    if (log_result < 0)
        length = append(popup, length, " LOG ERROR");
    line(buffer, stride, format, popup, length);
    if (!drawn) {
        first_draw = now;
        drawn = 1;
    }
    return 1;
}
