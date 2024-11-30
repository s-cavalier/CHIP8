#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <list>

class Window {
    SDL_Window* window;
    std::string _title;
    int _height, _width;
    bool open;
    bool keys[16];

public:
    Window(const std::string& _title, const int& _height, const int& _width, const int &flags = 0);
    ~Window();

    const std::string& title() const;
    const int& height() const;
    const int& width() const;
    const bool& is_open() const;

    void poll_events();
    void clear() const;
    const bool* keys_pressed() const;

    SDL_Renderer* renderer;
    
};

class GFX_Display {
    const int gfx_xlen = 64;
    const int gfx_ylen = 32;

    // Holds the struct data for a block at a given "pixel", is never actually used to draw. 
    // For drawing, if active the second value will be >= 0 detailing where in gfx_active the pixel is located.
    // This is used so GPU parrallelization can be utilized by calling FRects and the utility that the display is automatically black on init.
    int gfx_info[32][64];

    // Holds the underlying arary of FRects that will be rendered. Order is not garaunteed and switches based on deactivation. Use active_to_info to identify where we can find the object.
    std::vector< SDL_FRect > gfx_active;

    // the rect in gfx_active[i] corresponds to the rect at active_to_info[i]
    std::vector< std::pair<int, int> > active_to_info;

    // Renderer to draw to
    SDL_Renderer* renderer;

public:
    GFX_Display( SDL_Renderer* renderer );

    bool is_active(const int &x, const int &y) const;
    void activate(const int &x, const int &y);
    void deactivate(const int &x, const int &y);
    void render();
    void clear();

    // Returns a pointer to the array of pixels that need to be rendered.
    inline const SDL_FRect* pixels() const;

    // Returns the size of the pixels array.
    inline size_t pixels_len() const;

};