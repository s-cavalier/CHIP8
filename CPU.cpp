#include "CPU.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <random>

C8_CPU::~C8_CPU() {
    delete gen;
    delete dist;
}

C8_CPU::C8_CPU(const std::string& ROM, GFX_Display* _display) {

    PC = 0x200;
    I = 0;
    stack.reserve(16);
    clear_flag = false;
    delay_timer = 0;
    sound_timer = 0;

    display = _display;
    if (!display) {
        std::cerr << "Failed to enable display: CPU Side" << std::endl;
        exit(1);
    }

    gen = new std::mt19937(std::random_device()());
    dist = new std::uniform_int_distribution<unsigned short>(0, 255);

    std::cout << "Reading rom..." << std::endl;
    std::fstream file("./" + ROM, std::ios::in | std::ios::binary);
    if (file.fail()) {
        std::cerr << "Failed to read ROM " << ROM << "." << std::endl;
        exit(1);
    }
    std::cout << "Done." << std::endl;

    unsigned char fontset[80] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++) mem[i] = fontset[i];
    for (int i = 0; i < 16; i++) V[i] = 0;

    unsigned char* memory_loader = (mem + 0x200);
    while (!file.eof()) {
        char byte;
        file.read(&byte, 1);

        std::memcpy( memory_loader++, &byte, 1 );
    }

    // Def opcode responses
    // https://en.wikipedia.org/wiki/CHIP-8
    opresponse[ 0 ] = [&] (const unsigned short& oc) {

        switch (oc & 0x00FF) {
            case 0xE0:
                clear_flag = true;
                PC += 2;
                break;
            case 0xEE:
                PC = stack.back() + 2;
                stack.pop_back();
                break;
            default:
                std::cout << "Recieved unknown opcode or 0x0NNN. Continuing" << std::endl;
                break;
        }
    
    };

    opresponse[ 0x1000 ] = [&] (const unsigned short& oc) {
        PC = (oc & 0x0FFF);
    };

    opresponse[ 0x2000 ] = [&] (const unsigned short& oc) {
        stack.push_back(PC);
        PC = (oc & 0x0FFF);
    };

    opresponse[ 0x3000 ] = [&] (const unsigned short& oc) {
        PC += 2;
        if ( V[ (oc & 0x0F00) >> 8 ] == (oc & 0x00FF) ) PC += 2;
    };

    opresponse[ 0x4000 ] = [&] (const unsigned short& oc) {
        PC += 2;
        if ( V[ (oc & 0x0F00) >> 8 ] != (oc & 0x00FF) ) PC += 2;
    };

    opresponse[ 0x5000 ] = [&] (const unsigned short& oc) {
        PC += 2;
        if ( V[ (oc & 0x0F00) >> 8 ] == V[ (oc & 0x00F0) >> 4 ] ) PC += 2;
    };

    opresponse[ 0x6000 ] = [&] (const unsigned short& oc) {
        V[ ( oc & 0x0F00 ) >> 8 ] = (oc & 0x00FF);
        PC += 2;
    };

    opresponse[ 0x7000 ] = [&] (const unsigned short& oc) {
        V[ ( oc & 0x0F00 ) >> 8 ] += (oc & 0x00FF);
        PC += 2;
    };

    opresponse[ 0x8000 ] = [&] (const unsigned short& oc) {
        unsigned char& X = V[( oc & 0x0F00 ) >> 8];
        unsigned char& Y = V[( oc & 0x00F0 ) >> 4];

        switch ( oc & 0x000F ) {
            case 0:
                X = Y;
                break;
            case 1:
                X |= Y;
                break;
            case 2:
                X &= Y;
                break;
            case 3:
                X ^= Y;
                break;
            case 4:
                if ( (((unsigned short)X) + ((unsigned short)Y)) > 255 ) V[0xF] = 1;
                X += Y;
                break;
            case 5:
                V[0xF] = (X >= Y);
                X -= Y;
                break;
            case 6:
                V[0xF] = (X & 1);
                X >>= 1;
                break;
            case 7:
                V[0xF] = (Y >= X);
                X = Y - X;
                break;
            case 0xE:
                V[0xF] = (X & 128);
                X <<= 1;
                break;
            default:
                std::cerr << "Unknown 0x8XYN operation." << std::endl;
                break;
        }
        PC += 2;
    };

    opresponse[ 0x9000 ] = [&] (const unsigned short& oc) {
        PC += 2;
        if ( V[ (oc & 0x0F00) >> 8 ] != V[ (oc & 0x00F0) >> 4 ] ) PC += 2;
    };

    opresponse[ 0xA000 ] = [&] (const unsigned short& oc) {
        I = (oc & 0x0FFF);
        PC += 2;
    };

    opresponse[ 0xB000 ] = [&] (const unsigned short& oc) {
        PC = (oc & 0x0FFF) + V[0];
    };

    opresponse[ 0xC000 ] = [&] (const unsigned short& oc) {
        V[ (oc & 0x0F00 ) >> 8 ] = (*dist)(*gen) & (oc & 0x00FF);
        PC += 2;
    };

    opresponse[ 0xD000 ] = [&] (const unsigned short& oc) {
        unsigned char& X = V[( oc & 0x0F00 ) >> 8];
        unsigned char& Y = V[( oc & 0x00F0 ) >> 4];
        V[0xF] = 0;
        
        unsigned char height = (oc & 0x000F);

        // Check to see how the grid translates, I.E if [y][x] might be invalid and we should do [64 - y][x]
        // may be upside down if that is the case
        for (unsigned char y = 0; y < height; y++) {
            unsigned char activation = mem[I + y];
            unsigned char dY = Y + y;
            
            for (unsigned char x = 0; x < 8; x++) {
                unsigned char dX = X + x;

                bool exp_state = (activation & 255);
                bool was_active = display->is_active(dX, dY);

                if (exp_state) display->activate(dX, dY);
                else display->deactivate(dX, dY);

                if ( was_active && !display->is_active(dX, dY) ) V[0xF] = 1;

                activation <<= 1;
            }

        }

        PC += 2;

    };

    opresponse[ 0xE000 ] = [&] (const unsigned short& oc) {
        unsigned char& X = V[( oc & 0x0F00 ) >> 8];

        switch ( oc & 0x00FF ) {
            PC += 2;
            case 0x9E:
                if ( key [ X ] ) PC += 2;
                break;
            case 0xA1:
                if ( !key [ X ] ) PC += 2;
                break;
            default:
                std::cerr << "Unknown 0xEX operation" << std::endl;
                break;
        }

    };

    opresponse[ 0xF000 ] = [&] (const unsigned short& oc) {
        unsigned char& X = V[( oc & 0x0F00 ) >> 8];

        // Exclusively for case 0x0A
        bool cont = false;

        switch ( oc & 0x00FF ) {
            case 0x07:
                X = delay_timer;
                PC += 2;
                break;

            case 0x0A:
                for (int i = 0; i < 16; i++) {
                    if (key[i]) {
                        cont = true;
                        break;
                    }
                }

                if (cont) PC += 2;

                break;


            case 0x15:
                delay_timer = X;
                PC += 2;
                break;
            
            case 0x18:
                sound_timer = X;
                PC += 2;
                break;
            
            case 0x1E:
                I += X;
                PC += 2;
                break;
            
            case 0x29:
                I = V[ (oc & 0x0F00) >> 8 ] * 5;
                PC += 2;
                break;

            case 0x33:
                mem[I] = X / 100;
                mem[I + 1] = (X / 10) % 10;
                mem[I + 2] = X % 10;
                PC += 2;
                break;
            
            case 0x55:
                std::memcpy(mem + I, V, X);
                PC += 2;
                break;
            
            case 0x65:
                std::memcpy(V, mem + I, X);
                PC += 2;
                break;
        }
    };
}

void C8_CPU::cycle() {
    opcode = mem[PC] << 8 | mem[PC + 1];
    std::cout << "Reading opcode: " << std::hex << opcode << std::endl;

    if (!opresponse.count(opcode & 0xF000)) std::cerr << "Bad opcode: " << std::hex << opcode << std::endl;
    else opresponse[opcode & 0xF000](opcode);

    if (delay_timer > 0) --delay_timer;

    if (sound_timer == 1) std::cout << "beep" << std::endl;
    if (sound_timer > 0) --sound_timer;

}

void C8_CPU::setKeys(const bool* keys) {
    key = keys;
};

const bool &C8_CPU::clear() const {
    return clear_flag;
}

void C8_CPU::reset_clear() {
    clear_flag = false;
}