// Nintendo Switch input (see input.h): controllers through libnx's HID pad API (nx_platform.cpp). Pad buttons are
// normalised by the label printed on the controller - PAD_A is the Switch's A (right), PAD_B its B (bottom),
// PAD_X / PAD_Y its X (top) / Y (left), PAD_LB / PAD_RB are L / R, PAD_LT / PAD_RT are ZL / ZR, PAD_BACK is -,
// PAD_START is + - so menus confirm with A and go back with B like other Switch software. There is no keyboard
// support: the key functions exist for the shared code and always report "up".
#include "../input.h"
#include "../keys.h"
#include "nx_platform.h"
#include <cstdio>
#include <cstring>

struct Input::Impl {
    PadState pads[kMaxPads];
};

Input::Input() : impl(new Impl) {}
Input::~Input() { shutdown(); delete impl; }

bool Input::init(NativeWindow) {
    nx_platform_init();
    return true;
}

void Input::shutdown() {
    for (auto& p : impl->pads) p = PadState();
}

void Input::poll() {
    nx_pads_update();
    static const struct { uint32_t nx, bit; } kMap[] = {
        {NX_UP, PAD_UP},     {NX_DOWN, PAD_DOWN}, {NX_LEFT, PAD_LEFT}, {NX_RIGHT, PAD_RIGHT},
        {NX_A, PAD_A},       {NX_B, PAD_B},       {NX_X, PAD_X},       {NX_Y, PAD_Y},
        {NX_L, PAD_LB},      {NX_R, PAD_RB},      {NX_ZL, PAD_LT},     {NX_ZR, PAD_RT},
        {NX_MINUS, PAD_BACK}, {NX_PLUS, PAD_START}, {NX_LS, PAD_LS},   {NX_RS, PAD_RS},
    };
    for (int i = 0; i < kMaxPads; i++) {
        PadState& p = impl->pads[i];
        uint32_t nx = 0;
        float lx = 0, ly = 0;
        if (i >= kNxPads || !nx_pad_state(i, nx, lx, ly)) {
            p = PadState();
            continue;
        }
        uint32_t b = 0;
        for (const auto& m : kMap)
            if (nx & m.nx) b |= m.bit;
        if (ly < -0.5f) b |= PAD_LS_UP;
        if (ly > 0.5f) b |= PAD_LS_DOWN;
        if (lx < -0.5f) b |= PAD_LS_LEFT;
        if (lx > 0.5f) b |= PAD_LS_RIGHT;
        if (!p.connected) p.name = i == 0 ? "Controller 1 / Handheld" : "Controller " + std::to_string(i + 1);
        p.pressed = b & ~p.buttons;
        p.buttons = b;
        p.raw_pressed = p.pressed;
        p.raw_buttons = b;
        p.lx = lx;
        p.ly = ly;
        p.connected = true;
    }
}

void Input::on_device_change() {}
void Input::on_key(int, bool) {}
void Input::on_focus_lost() {}
bool Input::key_down(int) const { return false; }
bool Input::key_pressed(int) const { return false; }
int  Input::last_key_pressed() const { return 0; }

const PadState& Input::pad(int index) const {
    static const PadState none;
    return (index >= 0 && index < kMaxPads) ? impl->pads[index] : none;
}

int Input::first_connected_pad() const {
    for (int i = 0; i < kMaxPads; i++) if (impl->pads[i].connected) return i;
    return -1;
}

std::string Input::key_name(int vk) {
    switch (vk) {
    case 0: return "None";
    case VK_BACK: return "Backspace";  case VK_TAB: return "Tab";      case VK_RETURN: return "Enter";
    case VK_ESCAPE: return "Esc";      case VK_SPACE: return "Space";
    case VK_LEFT: return "Left";       case VK_UP: return "Up";        case VK_RIGHT: return "Right";   case VK_DOWN: return "Down";
    default: break;
    }
    if (vk >= VK_F1 && vk <= VK_F12) return "F" + std::to_string(vk - VK_F1 + 1);
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) return std::string(1, (char)vk);
    char buf[16];
    snprintf(buf, sizeof(buf), "Key 0x%02X", vk & 0xFF);
    return buf;
}

// Nintendo's labels (plain ASCII: the UI font need not have the - / + symbols).
std::string Input::pad_button_name(uint32_t bit) {
    static const char* const names[24] = {
        "Up", "Down", "Left", "Right", "A", "B", "X", "Y",
        "L", "R", "ZL", "ZR", "-", "+", "L Stick", "R Stick",
        "Home", nullptr, nullptr, nullptr, "Stick Up", "Stick Down", "Stick Left", "Stick Right" };
    if (!bit) return "None";
    int i = 0;
    while (!(bit & (1u << i))) i++;
    return (i < 24 && names[i]) ? names[i] : "?";
}
