#include <iostream>
#include <iomanip>
#include "Renderer.h"

Window::Window(const std::string& _title, const int& _height, const int& _width, const int &flags) {
    this->_title = _title;
    this->_height = _height;
    this->_width = _width;
    open = true;

    if ( !SDL_Init( SDL_INIT_VIDEO ) ) {
        std::cerr << "Failed at video init." << std::endl;
        return;
    }

    window = SDL_CreateWindow( _title.c_str(), _width, _height, flags );

    if ( !window ) {
        std::cerr << "Failed at window creation." << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, 0);

    if (!renderer) {
        std::cerr << "Failed at renderer creation." << std::endl;
        return;
    }


}

Window::~Window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Window::poll_events() {

    const bool* key_state = SDL_GetKeyboardState(nullptr);
    keys[0x0] = key_state[SDL_SCANCODE_1];
    keys[0x1] = key_state[SDL_SCANCODE_2];
    keys[0x2] = key_state[SDL_SCANCODE_3];
    keys[0x3] = key_state[SDL_SCANCODE_4];
    keys[0x4] = key_state[SDL_SCANCODE_Q];
    keys[0x5] = key_state[SDL_SCANCODE_W];
    keys[0x6] = key_state[SDL_SCANCODE_E];
    keys[0x7] = key_state[SDL_SCANCODE_R];
    keys[0x8] = key_state[SDL_SCANCODE_A];
    keys[0x9] = key_state[SDL_SCANCODE_S];
    keys[0xA] = key_state[SDL_SCANCODE_D];
    keys[0xB] = key_state[SDL_SCANCODE_F];
    keys[0xC] = key_state[SDL_SCANCODE_Z];
    keys[0xD] = key_state[SDL_SCANCODE_X];
    keys[0xE] = key_state[SDL_SCANCODE_C];
    keys[0xF] = key_state[SDL_SCANCODE_F];

    for (int i = 0; i < 16; i++) {
        if (keys[i]) std::cout << "Detected key press " << std::hex << i << std::endl; 
    }



    SDL_Event event;

    if ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                open = false;
                break;
            default:
                break;
        }
    }
}

void Window::clear() const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

const std::string& Window::title() const {
    return _title;
}
const int& Window::height() const {
    return _height;
}
const int& Window::width() const {
    return _width;
}
const bool& Window::is_open() const {
    return open;
}

const bool* Window::keys_pressed() const {
    return keys;
}

GFX_Display::GFX_Display(SDL_Renderer* renderer) {
    this->renderer = renderer;

    for (int i = 0; i < gfx_ylen; i++) {
        for (int j = 0; j < gfx_xlen; j++) gfx_info[i][j] = -1;
    }
}

bool GFX_Display::is_active(const int &x, const int &y) const {
    return gfx_info[y][x] > -1;
}

void GFX_Display::activate(const int& x, const int& y) {
    if (is_active(x, y)) {
        deactivate(x, y);
        return;
    }

    int location = gfx_active.size();
    gfx_info[y][x] = location;

    SDL_FRect push;
    push.x = x * 10;
    push.y = y * 10;
    push.w = 10;
    push.h = 10;
    
    gfx_active.push_back( push );
    active_to_info.push_back( {y, x} );
}

void GFX_Display::deactivate(const int &x, const int &y) {
    if (!is_active(x, y)) {
        activate(x, y);
        return;
    }

    int location = gfx_info[y][x];
    gfx_info[y][x]= -1;

    if (location == (int)gfx_active.size() - 1) {
        gfx_active.pop_back();
        active_to_info.pop_back();
        return;
    }

    // Replace the pixel with the item at the back at pop the item at the back to effectively "remove" the pixel at location
    gfx_active[location] = gfx_active.back();
    gfx_active.pop_back();

    // Repeat the procedure with the corresponding pixel to keep records stable
    active_to_info[location] = active_to_info.back();
    active_to_info.pop_back();
    
    // Fix the location pointer in the gfx_info matrix
    const auto& pixel = active_to_info[location];
    gfx_info[pixel.first][pixel.second] = location;

}

void GFX_Display::render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRects(renderer, pixels(), pixels_len());
}

void GFX_Display::clear() {
    while (!gfx_active.empty()) {
        const auto &point = active_to_info.back();
        gfx_info[point.first][point.second] = -1;
        gfx_active.pop_back();
        active_to_info.pop_back();
    }
}

const SDL_FRect* GFX_Display::pixels() const {
    return gfx_active.data();
}

size_t GFX_Display::pixels_len() const {
    return gfx_active.size();
}