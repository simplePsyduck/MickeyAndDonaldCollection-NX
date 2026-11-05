// Nintendo Switch homebrew (devkitPro: libnx + SDL2 + Mesa's OpenGL ES): the SDL window, the event loop and the
// few App methods that differ per platform. The rest of the app (launcher, menus, sessions, saves, achievements)
// is shared with the Windows and Android builds; video is the Android build's OpenGL ES 3 back end, audio its SDL
// queue, and controllers come through libnx (platform/switch/input_switch.cpp).
#define SDL_MAIN_HANDLED   // plain main() below: SDL must not rename it
#include "app.h"
#include "../platform/switch/nx_platform.h"
#include <SDL.h>
#include <algorithm>
#include <cstdio>

namespace {
// The widest widescreen mode that fits the screen with square pixels, for the first start (16:9 -> 16:9).
WideMode widest_fitting(int w, int h) {
    if (w <= 0 || h <= 0) return WideMode::W16x9;
    const float aspect = (float)std::max(w, h) / (float)std::min(w, h);
    WideMode best = WideMode::Off;
    for (int m = 0; m < (int)WideMode::Count; m++)
        if ((320.0f + 2 * wide_ext((WideMode)m)) / 224.0f <= aspect + 0.01f) best = (WideMode)m;
    return best;
}
}  // namespace

int App::run() {
    nx_platform_init();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {   // (pads are read through libnx, not SDL)
        fprintf(stderr, "MickeyDonald: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    // The Switch's window always fills the screen (1280x720 handheld, 1920x1080 docked): the size asked for here
    // is only a start; the real one is read back as the drawable size and followed on size-change events.
    window_ = SDL_CreateWindow("Mickey & Donald Collection", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                               SDL_WINDOW_OPENGL);
    if (!window_) {
        fprintf(stderr, "MickeyDonald: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(window_, &w, &h);
    if (w <= 0 || h <= 0) SDL_GetWindowSize(window_, &w, &h);

    make_dir(data_dir());
    make_dir(path_join(data_dir(), L"roms"));
    settings_path_ = path_join(data_dir(), L"settings.ini");
    if (!settings_.load(settings_path_)) settings_.widescreen = widest_fitting(w, h);
    settings_.vsync = true;        // the display is fixed at 60 Hz
    settings_.fullscreen = true;

    games_[0].module.reset(create_coi_module());
    games_[1].module.reset(create_qs_module());
    games_[2].module.reset(create_woi_module());

    if (!video_.init(window_, w, h)) {
        fprintf(stderr, "MickeyDonald: OpenGL ES 3.0 could not be started\n");
        return 1;
    }
    audio_.init(48000);
    input_.init(window_);

    find_game_roms();
    scan_soundtracks();
    apply_settings();
    ach_startup();
    for (int g = 0; g < kGameCount; g++)
        if (settings_.last_game == games_[g].module->id()) launcher_sel_ = launcher_last_card_ = g;

    Uint64 freq = SDL_GetPerformanceFrequency(), last = SDL_GetPerformanceCounter();
#ifdef MDC_NXLINK
    int perf_frames = 0;
    double perf_time = 0;
#endif
    while (running_) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running_ = false; break;
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // docking / undocking changes the size
                    SDL_GL_GetDrawableSize(window_, &w, &h);
                    if (w > 0 && h > 0) video_.resize(w, h);
                } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    save_settings();   // (the app may be closed from the HOME menu while it is in the background)
                    input_.on_focus_lost();
                } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    last = SDL_GetPerformanceCounter();
                }
                break;
            default: break;
            }
        }
        if (!running_) break;
        const Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / (double)freq;
        last = now;
        if (dt > 0.25) dt = 0.25;
        frame(dt);
#ifdef MDC_NXLINK
        perf_frames++;
        if ((perf_time += dt) >= 5.0) {   // every 5 s to `nxlink -s`: the frame rate the display really gets
            printf("MickeyDonald: %.1f fps\n", perf_frames / perf_time);
            fflush(stdout);
            perf_frames = 0;
            perf_time = 0;
        }
#endif
    }
    save_settings();
    jukebox_.close();
    session_.reset();
    for (auto& g : games_) g.preview.reset();
    input_.shutdown();
    audio_.shutdown();
    video_.shutdown();
    SDL_DestroyWindow(window_);
    SDL_Quit();
    nx_platform_exit();
    return 0;
}

// One full-screen window on a fixed 60 Hz display.
void App::set_fullscreen(bool) { is_fullscreen_ = true; }
void App::resize_window_to_scale() {}
void App::move_to_monitor(int) {}
double App::refresh_rate() { return 60.0; }

// Trailer recording is a Windows test feature.
void App::start_recording(const std::string&) {}
void App::record_frame() {}
void App::stop_recording() {}

int main(int, char**) {
    static App app;
    return app.run();
}
