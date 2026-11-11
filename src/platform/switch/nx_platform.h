// libnx glue for the Switch build, kept in its own translation unit: <switch.h> declares a `PadState` that
// collides with the one in platform/input.h, so nothing else includes it (apart from glyphs_ft.cpp, which needs
// only the font service).
#pragma once
#include <cstdint>

// Controller buttons as libnx reports them, by the label printed on a Nintendo controller.
enum NxButton : uint32_t {
    NX_A = 1u << 0,  NX_B = 1u << 1,  NX_X = 1u << 2,  NX_Y = 1u << 3,
    NX_LS = 1u << 4, NX_RS = 1u << 5, NX_L = 1u << 6,  NX_R = 1u << 7,
    NX_ZL = 1u << 8, NX_ZR = 1u << 9, NX_PLUS = 1u << 10, NX_MINUS = 1u << 11,
    NX_LEFT = 1u << 12, NX_UP = 1u << 13, NX_RIGHT = 1u << 14, NX_DOWN = 1u << 15,
};
constexpr int kNxPads = 4;   // player 1 = the handheld controls or controller 1, then controllers 2-4

void nx_platform_init();     // HID pads (and, in -DMDC_NXLINK builds, stdout over nxlink)
void nx_platform_exit();
void nx_pads_update();       // once per frame
// The pad in `slot` (0..kNxPads-1): false if nothing is connected there. The left stick is -1..1, y up = -1.
bool nx_pad_state(int slot, uint32_t& buttons, float& lx, float& ly);
