#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include "Renderer.h"

class C8_CPU {

    unsigned short opcode;
    unsigned char mem[4096];

    unsigned char V[0xF];

    unsigned short I;
    unsigned short PC;

    unsigned char delay_timer;
    unsigned char sound_timer;

    // size = 16
    const bool* key;

    bool clear_flag;

    std::vector<unsigned short> stack;

    GFX_Display* display;

    std::unordered_map<unsigned short, std::function<void(const unsigned short&)>> opresponse;

    // Random generators used for OPCODE 0xC
    std::mt19937* gen;
    std::uniform_int_distribution<unsigned short>* dist;

public:
    C8_CPU(const std::string& ROM, GFX_Display* _display);
    ~C8_CPU();

    void cycle();
    const bool &clear() const;
    void reset_clear();
    void setKeys( const bool* keys ) ;

};