#include "nx_platform.h"
#include <switch.h>
#include <cstdio>

namespace {
PadState g_pads[kNxPads];
bool g_ready = false;
bool g_sockets = false;
}  // namespace

void nx_platform_init() {
    if (g_ready) return;
#ifdef MDC_NXLINK
    // printf / stderr go to `nxlink -s` on the PC (see switch/README.md)
    if (R_SUCCEEDED(socketInitializeDefault())) {
        nxlinkStdio();
        g_sockets = true;
    }
#endif
    
    // Configure input to support all controller styles including sideways Joy-Cons
    // Use NpadStandard which supports Joy-Con, Pro Controller, and handheld modes
    padConfigureInput(kNxPads, HidNpadStyleSet_NpadStandard);
    
    // Player 1: controller 1 or the handheld controls
    // This mask includes both standard controllers and handheld mode
    padInitializeWithMask(&g_pads[0], BITL(HidNpadIdType_No1) | BITL(HidNpadIdType_Handheld));
    
    // Players 2-4: controllers 2-4 (supports Joy-Con left/right, Pro Controller, etc.)
    for (int i = 1; i < kNxPads; i++) {
        padInitialize(&g_pads[i], (HidNpadIdType)(HidNpadIdType_No1 + i));
    }
    
    g_ready = true;
}

void nx_platform_exit() {
    if (g_sockets) {
        socketExit();
        g_sockets = false;
    }
    g_ready = false;
}

void nx_pads_update() {
    if (!g_ready) return;
    for (int i = 0; i < kNxPads; i++) padUpdate(&g_pads[i]);
}

bool nx_pad_state(int slot, uint32_t& buttons, float& lx, float& ly) {
    buttons = 0;
    lx = ly = 0;
    if (!g_ready || slot < 0 || slot >= kNxPads) return false;
    
    const PadState* p = &g_pads[slot];
    if (!padIsConnected(p)) return false;
    
    // Get the current button state
    const uint64_t held = padGetButtons(p);
    
    // Mapping from Switch button codes to normalised NxButton codes
    static const struct { uint64_t hid; uint32_t nx; } kMap[] = {
        {HidNpadButton_A, NX_A},        {HidNpadButton_B, NX_B},          {HidNpadButton_X, NX_X},       {HidNpadButton_Y, NX_Y},
        {HidNpadButton_StickL, NX_LS},  {HidNpadButton_StickR, NX_RS},    {HidNpadButton_L, NX_L},       {HidNpadButton_R, NX_R},
        {HidNpadButton_ZL, NX_ZL},      {HidNpadButton_ZR, NX_ZR},        {HidNpadButton_Plus, NX_PLUS}, {HidNpadButton_Minus, NX_MINUS},
        {HidNpadButton_Left, NX_LEFT},  {HidNpadButton_Up, NX_UP},        {HidNpadButton_Right, NX_RIGHT}, {HidNpadButton_Down, NX_DOWN},
    };
    
    // For sideways Joy-Cons, libnx automatically remaps the buttons to standard positions
    // So we can use the same mapping for all controller types
    
    for (const auto& m : kMap) {
        if (held & m.hid) {
            buttons |= m.nx;
        }
    }
    
    // Get analog stick position
    const HidAnalogStickState s = padGetStickPos(p, 0);
    lx = (float)s.x / 32767.0f;
    ly = -(float)s.y / 32767.0f;   // libnx: up is positive
    
    return true;
}
