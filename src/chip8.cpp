#include "chip8.h"

#include <chrono>
#include <format>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <functional>

// Toggle to print debug values to console
// #define DEBUG_SHOW_MSG
// #define DEBUG_SHOW_DUMP

// Macros to make life easier
#define Vx ((m_opcode & 0x0F00u) >> 8u)
#define Vy ((m_opcode & 0x00F0u) >> 4u)
#define addr_nnn (m_opcode & 0x0FFFu) 
#define byte_kk (m_opcode & 0x00FFu)
#define reg_Vx m_registers[Vx]
#define reg_Vy m_registers[Vy]
#define reg_VF m_registers[address::VF]

interpreter_8::Chip8::Chip8() :
    m_randomEngine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())),
    m_indexRegister(address::DEFAULT),
    m_programCounter(address::DEFAULT),
    m_stackPointer(address::DEFAULT),
    m_delayTimer(address::DEFAULT),
    m_soundTimer(address::DEFAULT),
    m_opcode(address::DEFAULT),
    m_initialized(false),
    m_romFlag(false)
{ }

bool interpreter_8::Chip8::Init() {
    // Initialize flag - calling Init more than once
    m_initialized = false;

    // Reset key components
    m_programCounter = address::START;
    m_indexRegister = address::DEFAULT;
    m_stackPointer = address::DEFAULT;
    m_opcode = address::DEFAULT;

    // Reset timers
    m_delayTimer = address::DEFAULT;
    m_soundTimer = address::DEFAULT;

    // Clear memory, keypad, graphics, and program stack
    std::fill_n(m_memory, MEMORY, address::DEFAULT);
    std::fill_n(m_keypad, KEYPAD, address::DEFAULT);
    std::fill_n(m_programStack, STACK, address::DEFAULT);
    std::fill_n(m_graphics, RESOLUTION, address::DEFAULT);

    // Load fontset into memory
    loadFonts();

    // Initialization was successful
    return (m_initialized = true);
}

bool interpreter_8::Chip8::LoadROM(const char* filename) {
    // Check if emulator has been properly initalized before trying to load ROM file
    if (m_romFlag || !m_initialized) {
        return error("Error: trying to load in ROM file without (re-)initializing interpreter.");
    }

    // Open ROM file
    std::ifstream rom(filename, std::ios::binary | std::ios::ate);
    if (!rom.is_open()) {
        return error("Error: failed to open ROM file.");
    }

    // Check if ROM will fit in memory
    std::streampos romSize = rom.tellg();

    if (romSize > (size::MEMORY - address::START)) {
        return error("Error: ROM size is too large to fit into memory.");
    }

    // Load rom into memory
    rom.seekg(0, std::ios::beg);
    rom.read((char*)(m_memory + address::START), romSize);
    rom.close();

    // Successfully loaded ROM into memory
    return (m_romFlag = true);
}

void interpreter_8::Chip8::RunCycle() {
    // Fetch
    m_opcode = (m_memory[m_programCounter] << 8u) | m_memory[m_programCounter + 1];

    // Increment program counter by two before executing anything to avoid putting it in opcodes.
    m_programCounter += 2;

    // Decode and Execute
    emulate();

    // Decrement timers if they are set
    if (m_delayTimer > 0) m_delayTimer--;
    if (m_soundTimer > 0) m_soundTimer--;
}

/**
 * +--------------------------------------------------------------------------------------------------------------------------+
 * |                                                                                                                          |
 *?|                                                 CHIP-8 Helper Functions                                                  |
 * |                                                                                                                          |
 * +--------------------------------------------------------------------------------------------------------------------------+
 */

void interpreter_8::Chip8::emulate() {
    // Decode instruction and execute
    switch (m_opcode & 0xF000u) {
        case 0x0000u:
            switch (m_opcode & 0x000Fu) {
                case 0x0000u:
                    debug_msg("\top_00E0 - 0x0000u");
                    op_00E0();
                    break;
                case 0x000Eu:
                    debug_msg("\top_00EE - 0x000Eu");
                    op_00EE();
                    break;
                default:
                    error("Error: encountered unknown opcode.");
                    break;
            }
            break;
        case 0x1000u:
            debug_msg("\top_1nnn - 0x1000u");
            op_1nnn();
            break;
        case 0x2000u:
            debug_msg("\top_2nnn - 0x2000u");
            op_2nnn();
            break;
        case 0x3000u:
            debug_msg("\top_3xkk - 0x3000u");
            op_3xkk();
            break;
        case 0x4000u:
            debug_msg("\top_4xkk - 0x4000u");
            op_4xkk();
            break;
        case 0x5000u:
            debug_msg("\top_5xy0 - 0x5000u");
            op_5xy0();
            break;
        case 0x6000u:
            debug_msg("\top_6xkk - 0x6000u");
            op_6xkk();
            break;
        case 0x7000u:
            debug_msg("\top_7xkk - 0x7000u");
            op_7xkk();
            break;
        case 0x8000u:
            switch (m_opcode & 0x000Fu) {
                case 0x0000u:
                    debug_msg("\top_8xy0 - 0x8000u -> 0x0000u");
                    op_8xy0();
                    break;
                case 0x0001u:
                    debug_msg("\top_8xy1 - 0x8000u -> 0x0001u");
                    op_8xy1();
                    break;
                case 0x0002u:
                    debug_msg("\top_8xy2 - 0x8000u -> 0x0002u");
                    op_8xy2();
                    break;
                case 0x0003u:
                    debug_msg("\top_8xy3 - 0x8000u -> 0x0003u");
                    op_8xy3();
                    break;
                case 0x0004u:
                    debug_msg("\top_8xy4 - 0x8000u -> 0x0004u");
                    op_8xy4();
                    break;
                case 0x0005u:
                    debug_msg("\top_8xy5 - 0x8000u -> 0x0005u");
                    op_8xy5();
                    break;
                case 0x0006u:
                    debug_msg("\top_8xy6 - 0x8000u -> 0x0006u");
                    op_8xy6();
                    break;
                case 0x0007u:
                    debug_msg("\top_8xy7 - 0x8000u -> 0x0007u");
                    op_8xy7();
                    break;
                case 0x000Eu:
                    debug_msg("\top_8xyE - 0x8000u -> 0x000Eu");
                    op_8xyE();
                    break;
                default:
                    error("Error: encountered unknown opcode.");
                    break;
            }
            break;
        case 0x9000u:
            debug_msg("\top_9xy0 - 0x9000u");
            op_9xy0();
            break;
        case 0xA000u:
            debug_msg("\top_Annn - 0xA000u");
            op_Annn();
            break;
        case 0xB000u:
            debug_msg("\top_Bnnn - 0xB000u");
            op_Bnnn();
            break;
        case 0xC000u:
            debug_msg("\top_Cxkk - 0xC000u");
            op_Cxkk();
            break;
        case 0xD000u:
            debug_msg("\top_Dxyn - 0xD000u");
            op_Dxyn();
            break;
        case 0xE000u:
            switch (m_opcode & 0x00FFu) {
                case 0x009Eu:
                    debug_msg("\top_Ex9E - 0xE000u -> 0x009Eu");
                    op_Ex9E();
                    break;
                case 0x00A1u:
                    debug_msg("\top_ExA1 - 0xE000u -> 0x00A1u");
                    op_ExA1();
                    break;
                default:
                    error("Error: encountered unknown opcode.");
                    break;
            }
            break;
        case 0xF000u:
            switch (m_opcode & 0x00FFu) {
                case 0x0007u:
                    debug_msg("\top_Fx07 - 0xF000u -> 0x0007u");
                    op_Fx07();
                    break;
                case 0x000Au:
                    debug_msg("\top_Fx0A - 0xF000u -> 0x000Au");
                    op_Fx0A();
                    break;
                case 0x0015u:
                    debug_msg("\top_Fx15 - 0xF000u -> 0x0015u");
                    op_Fx15();
                    break;
                case 0x0018u:
                    debug_msg("\top_Fx18 - 0xF000u -> 0x0018u");
                    op_Fx18();
                    break;
                case 0x001Eu:
                    debug_msg("\top_Fx1E - 0xF000u -> 0x001Eu");
                    op_Fx1E();
                    break;
                case 0x0029u:
                    debug_msg("\top_Fx29 - 0xF000u -> 0x0029u");
                    op_Fx29();
                    break;
                case 0x0033u:
                    debug_msg("\top_Fx33 - 0xF000u -> 0x0033u");
                    op_Fx33();
                    break;
                case 0x0055u:
                    debug_msg("\top_Fx55 - 0xF000u -> 0x0055u");
                    op_Fx55();
                    break;
                case 0x0065:
                    debug_msg("\top_Fx65 - 0xF000u -> 0x0065");
                    op_Fx65();
                    break;
                default:
                    error("Error: encountered unknown opcode.");
                    break;
            }
            break;
        default:
            error("Error: opcode not supported.");
            std::exit(EXIT_FAILURE);
            break;
    }
}

void interpreter_8::Chip8::loadFonts() {
    for (unsigned int i = 0u; i < font::SIZE; i++) {
        m_memory[FONT_START + i] = FONTSET[i];
    }
}

void interpreter_8::Chip8::debug_msg(const char* msg) {
    #ifdef DEBUG_SHOW_MSG
        std::cerr << msg << std::endl;
    #endif
}

void interpreter_8::Chip8::debug_dump() {
    #ifdef DEBUG_SHOW_DUMP
        using namespace std;

        // Reset stream after changing properties with iomanip
        ios defaultStream(nullptr);
        defaultStream.copyfmt(cerr);

        // Prints given value in hexadecimal format --> 0x0000
        auto printHex = [](const char* msg, uint16_t val, char fill) {
            cerr << msg << setw(4) << setfill(fill) << right << format("{:X}", val);
            return ' ';
        };

        // Prints label and type to console --> Label[type] - 
        auto printLabel = [](const char* label, const char* type, int val) {
            cerr << "\t" << label << '[' << type << format("{:X}", val) << "] - ";
            return "";
        };

        // Dump generic values
        cerr << "General value Dump: \n";
        cerr << "\topcode          - " << printHex("0x", m_opcode, '0') << endl;
        cerr << "\tDelay Timer     - " << printHex("0x", m_delayTimer, '0') << endl;
        cerr << "\tSound timer     - " << printHex("0x", m_soundTimer, '0') << endl;
        cerr << "\tSack Pointer    - " << printHex("0x", m_stackPointer, '0') << endl;
        cerr << "\tIndex Register  - " << printHex("0x", m_indexRegister, '0') << endl;
        cerr << "\tProgram Counter - " << printHex("0x", m_programCounter, '0') << endl;

        // Dump register
        cerr << "\nRegister Dump: \n";
        for (int i = 0; i < size::REGISTER; i++) {
            cerr << printLabel("Register", "V", i) << printHex("0x", m_registers[i], '0') << endl;
        }

        // Dump stack
        cerr << "\nStack Dump: \n";
        for (int i = 0; i < size::STACK; i++) {
            cerr << printLabel("Stack", "0x", i) << printHex("0x", m_programStack[i], '0') << endl;
        }

        // Dump memory
        cerr << "\nMemory Dump:\n";
        for (int i = 0, j = 0; i < size::MEMORY; i++, j++) {
            printHex(" ", i, '0');              // Print memory index
            printHex(": 0x", m_memory[i], '0'); // Print value
            cerr << ", " << (j == 8 ? "\n" : "");
            if (j == 7) j = 0;
        }

        // Rest stream
        cerr << endl << endl;
        cerr.copyfmt(defaultStream);
    #endif
}

/**
 * +--------------------------------------------------------------------------------------------------------------------------+
 * |                                                                                                                          |
 *?|                                                     CHIP-8 op codes                                                      |
 * |                                                                                                                          |
 * +--------------------------------------------------------------------------------------------------------------------------+
 */

void interpreter_8::Chip8::op_00E0() {
    std::fill_n(m_graphics, RESOLUTION, address::DEFAULT);
}

void interpreter_8::Chip8::op_00EE() {
    // Top of stack has address of one instruction past the one that called the subroutine.
    // To return from subroutine, put previous address into program counter.
    m_programCounter = m_programStack[--m_stackPointer];
}

void interpreter_8::Chip8::op_1nnn() {
    // Call routine at location nnn given opcode 1nnn. Jump does NOT remember its origin, no stack interaction required.
    m_programCounter = addr_nnn; 
}

void interpreter_8::Chip8::op_2nnn() {
    m_programStack[m_stackPointer++] = m_programCounter; // Save current program counter on stack in new frame
    m_programCounter = addr_nnn;                         // Call routine at location nnn given opcode 2nnn
}

void interpreter_8::Chip8::op_3xkk() {
    // Compare register Vx with kk (byte): if they are equal, increment program counter by 2
    if (reg_Vx == byte_kk) m_programCounter += 2;
}

void interpreter_8::Chip8::op_4xkk() {
    // Compare register Vx with kk (byte): if they are NOT equal, increment program counter by 2
    if (reg_Vx != byte_kk) m_programCounter += 2;
}

void interpreter_8::Chip8::op_5xy0() {
    // Compare register Vx with register Vy: if they are equal, increment program counter by 2
    if (reg_Vx == reg_Vy) m_programCounter += 2;
}

void interpreter_8::Chip8::op_6xkk() {
    reg_Vx = byte_kk; // Store the value of kk in Vx
}

void interpreter_8::Chip8::op_7xkk() {
    reg_Vx += byte_kk; // Add kk to register Vx
}

void interpreter_8::Chip8::op_8xy0() {
    reg_Vx = reg_Vy; // Store the value of Vy in Vx
}

void interpreter_8::Chip8::op_8xy1() {
    reg_Vx |= reg_Vy; // Perform bitwise OR on values of Vx and Vy, store the result in Vx
}

void interpreter_8::Chip8::op_8xy2() {
    reg_Vx &= reg_Vy; // Perform bitwise AND on values of Vx and Vy, store the result in Vx
}

void interpreter_8::Chip8::op_8xy3() {
    reg_Vx ^= reg_Vy; // Perform bitwise XOR on values of Vx and Vy, store the result in Vx
}

void interpreter_8::Chip8::op_8xy4() {
    uint16_t sum = reg_Vx + reg_Vy;
    reg_VF = sum > 255u ? 1u : 0u; // If the the sum is greater than 255 (8 bits), VF is set to 1, other 0 (bot borrow).
    reg_Vx = sum & 0xFFu;          // Keep lowest 8 bits of the result and store in Vx
}

void interpreter_8::Chip8::op_8xy5() {
    reg_VF = reg_Vx > reg_Vy ? 1u : 0u; // If Vx is greater than Vy, VF is set to 1, otherwise it is set to 0 (not borrow).
    reg_Vx -= reg_Vy;                   // Subtract Vy from Vx and store the result in Vx
}

void interpreter_8::Chip8::op_8xy6() {
    reg_VF = (reg_Vx & 0x1u); // Save least significant bit of Vx in VF
    reg_Vx >>= 1;             // Divide Vx by 2
}

void interpreter_8::Chip8::op_8xy7() {
    reg_VF = reg_Vy > reg_Vx ? 1u : 0u; // If Vy is greater than Vx, VF is set to 1, otherwise it is set to 0 (not borrow).
    reg_Vx = reg_Vy - reg_Vx;           // Subtract Vx from Vy and store the value in Vx
}

void interpreter_8::Chip8::op_8xyE() {
    reg_VF = (reg_Vx & 0x80u) >> 7u; // Save the most significant bit of Vx in VF
    reg_Vx <<= 1;                    // Multiply Vx by two
}

void interpreter_8::Chip8::op_9xy0() {
    // Compare register Vx with register Vy: if they are not equal, increment program counter by 2
    if (reg_Vx != reg_Vy) m_programCounter += 2;
}

void interpreter_8::Chip8::op_Annn() {
    m_indexRegister = addr_nnn; // Set index register to nnn
}

void interpreter_8::Chip8::op_Bnnn() {
    m_programCounter = m_registers[0] + addr_nnn; // Set program counter to V0 + nnn
}

void interpreter_8::Chip8::op_Cxkk() {
    // Set Vx to random number between 0 and 255 ANDed with kk
    auto randomByte = std::bind(std::uniform_int_distribution<unsigned int>(0u, 255u), m_randomEngine);
    reg_Vx = randomByte() & byte_kk;
}

void interpreter_8::Chip8::op_Dxyn() {
    // Conflict about sprite behavior when it reaches the screens edge. Some prefer the 
    // sprite being clipped and others the sprite wrapping. personally, I prefers the
    // sprites wrapping when they reach the screen edge. 
    uint8_t x = reg_Vx % video::WIDTH;
    uint8_t y = reg_Vy % video::HEIGHT;
    uint8_t height = m_opcode & VF;
    uint32_t on = 0xFFFFFFFF;
    reg_VF = 0u;

    // Iterative over each pixel
    for (unsigned int row = 0u; row < height; row++) {
        // Get sprite from memory
        uint8_t nth_spriteByte = m_memory[m_indexRegister + row];

        // Iterating over each pixel in current row
        for (unsigned int pixel = 0u; pixel < 8u; pixel++) {
            // Get current screen pixel
            uint32_t* screenPixel = &m_graphics[(y + row) * video::WIDTH + (x + pixel)];

            // Get current sprite pixel
            uint8_t spritePixel = nth_spriteByte & (0x80u >> pixel);

            // Sprite pixel is on
            if (spritePixel) {
                if (*screenPixel == on) reg_VF = 1u; // Collision at current pixel - screen and sprite pixel on
                *screenPixel ^= on;                  // XOR screen pixel with 0xFFFFFFFF to XOR sprite pixel
            }
        }
    }
}

void interpreter_8::Chip8::op_Ex9E() {
    // If the value of key is currently pressed, skip next instruction
    if (m_keypad[reg_Vx]) m_programCounter += 2;
}

void interpreter_8::Chip8::op_ExA1() {
    // If the value of key is not currently pressed, skip current instruction
    if (!m_keypad[reg_Vx]) m_programCounter += 2;
}

void interpreter_8::Chip8::op_Fx07() {
    reg_Vx = m_delayTimer; // Store the value of the delay timer in Vx
}

void interpreter_8::Chip8::op_Fx0A() {
    // Store the value of the keypress in Vx if keypress is detected. If a keypress is not detected, 
    // decrement program counter by 2 since each execution loop increments the program counter by 2.
    m_keypad[0]  ? reg_Vx = 0  :
    m_keypad[1]  ? reg_Vx = 1  :
    m_keypad[2]  ? reg_Vx = 2  :
    m_keypad[3]  ? reg_Vx = 3  :
    m_keypad[4]  ? reg_Vx = 4  :
    m_keypad[5]  ? reg_Vx = 5  :
    m_keypad[6]  ? reg_Vx = 6  :
    m_keypad[7]  ? reg_Vx = 7  :
    m_keypad[8]  ? reg_Vx = 8  :
    m_keypad[9]  ? reg_Vx = 9  :
    m_keypad[10] ? reg_Vx = 10 :
    m_keypad[11] ? reg_Vx = 11 :
    m_keypad[12] ? reg_Vx = 12 :
    m_keypad[13] ? reg_Vx = 13 :
    m_keypad[14] ? reg_Vx = 14 :
    m_keypad[15] ? reg_Vx = 15 :
    m_programCounter -= 2;
}

void interpreter_8::Chip8::op_Fx15() {
    m_delayTimer = reg_Vx; // Set delay timer is equal to the value of Vx
}

void interpreter_8::Chip8::op_Fx18() {
    m_soundTimer = reg_Vx; // Set sound timer is equal to the value of Vx
}

void interpreter_8::Chip8::op_Fx1E() {
    m_indexRegister += reg_Vx; // Add the value of Vx to the index register
}

void interpreter_8::Chip8::op_Fx29() {
    uint8_t digit = reg_Vx;

    // Since the font character is located at 0x50 and is five bytes the value of I is set to the location 
    // for the hexadecimal sprite corresponding to the value of Vx by taking an offset from the start address
    m_indexRegister = address::FONT_START + (font::OFFSET * digit);
}

void interpreter_8::Chip8::op_Fx33() {
    // The interpreter takes the decimal value of Vx, and places the hundreds digit in memory
    // at location in I, the tens digit at location I + 1, and the ones digit at location I + 2.
    uint8_t value = reg_Vx;

    // Ones place
    m_memory[m_indexRegister + 2] = value % 10;
    value /= 10;

    // Tens place
    m_memory[m_indexRegister + 1] = value % 10;
    value /= 10;

    // Hundreds place
    m_memory[m_indexRegister] = value % 10;
}

void interpreter_8::Chip8::op_Fx55() {
    // Copy the values of registers V0 through Vx into memory starting at index register
    for (uint8_t i = 0; i <= Vx; i++) {
        m_memory[m_indexRegister + i] = m_registers[i];
    }
}

void interpreter_8::Chip8::op_Fx65() {
    // Read the values from memory starting at index register in registers V0 through Vx
    for (uint8_t i = 0; i <= Vx; i++) {
        m_registers[i] = m_memory[m_indexRegister + i];
    }
}
