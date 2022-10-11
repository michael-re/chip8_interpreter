#pragma once

#include <random>
#include <cstdint>
#include <iostream>

namespace interpreter_8 {
    class Chip8 final {
    public:
        enum video : unsigned int { WIDTH = 64u, HEIGHT = 32u, RESOLUTION = WIDTH * HEIGHT };

    private:
        enum font    : unsigned int { SIZE  = 80u,  OFFSET  = 5u                                           };
        enum size    : unsigned int { STACK = 16u,  KEYPAD  = 16u, REGISTER = 16u,    MEMORY     = 4096u   };
        enum address : unsigned int { VF    = 0xFu, DEFAULT = 0u,   START    = 0x200u, FONT_START = 0x050u };
    
        /**
         * @brief Fontset will occupy addresses in memory from 0x0050u to 0x00A0u (but anywhere in the first 512 bytes is fine).
         * All assets in the fontset are 4 pixels wide and 5 pixels tall. Once a ROM is loaded, it will be looking for these
         * assets. This will have to be loaded in manually during the initialization of the interpreter.
         */
        const uint8_t FONTSET[SIZE] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0,    // 0
            0x20, 0x60, 0x20, 0x20, 0x70,    // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0,    // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0,    // 3
            0x90, 0x90, 0xF0, 0x10, 0x10,    // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0,    // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0,    // 6
            0xF0, 0x10, 0x20, 0x40, 0x40,    // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0,    // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0,    // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90,    // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0,    // B
            0xF0, 0x80, 0x80, 0x80, 0xF0,    // C
            0xE0, 0x90, 0x90, 0x90, 0xE0,    // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0,    // E
            0xF0, 0x80, 0xF0, 0x80, 0x80     // F
        };

        /**
         * +--------------------------------------------------------------------------------------------------------------------------+
         * |                                                                                                                          |
         *?|                                                Chip-8 Graphics and Input                                                 |
         * |                                                                                                                          |
         * +--------------------------------------------------------------------------------------------------------------------------+
         */

         /**
          * The 16 input keys that match the first 16 hex values (0 - F). A key can either be pressed or not pressed.
          *      +-----------------------------------------------------------------------+
          *      |                             Input Mapping                             |
          *      +----------------------------------+------------------------------------+
          *      |         +---------------+        |        +---------------+           |
          *      |         |    Keypad     |        |        |    Keyboard   |           |
          *      |         +---+---+---+---+        |        +---+---+---+---+           |
          *      |         | 1 | 2 | 3 | C |        |        | 1 | 2 | 3 | 4 |           |
          *      |         +---+---+---+---+        |        +---+---+---+---+           |
          *      |         | 4 | 5 | 6 | D |        |        | Q | W | E | R |           |
          *      |         +---+---+---+---+        |        +---+---+---+---+           |
          *      |         | 7 | 8 | 9 | E |        |        | A | S | D | F |           |
          *      |         +---+---+---+---+        |        +---+---+---+---+           |
          *      |         | A | 0 | B | F |        |        | Z | X | C | V |           |
          *      |         +---+---+---+---+        |        +---+---+---+---+           |
          *      +----------------------------------+------------------------------------+
          */
        uint8_t m_keypad[KEYPAD] = { DEFAULT };

        /**
         * A 32-bit memory buffer stores graphics to display. The display resolution is 64 pixels wide and 32 pixels high.
         * Each pixel can have two states: on or off. Thus, only white and black can be displayed to the screen.
         */
        uint32_t m_graphics[RESOLUTION] = { DEFAULT };

        /**
         * +--------------------------------------------------------------------------------------------------------------------------+
         * |                                                                                                                          |
         *?|                                               Chip-8 General Components                                                  |
         * |                                                                                                                          |
         * +--------------------------------------------------------------------------------------------------------------------------+
         */

         /**
          * @brief CHIP-8 has 4096 bytes of memory addressed from 0x000u to 0x0FFFu.The address space can be segmented into
          * three sections as follows:
          *      - Segment 1: 0x000 - 0x1FF
          *           * Reserved for CHIP-8 interpreter: this portion of memory is left untouched by this program.
          *      - Segment 2: 0x050 - 0x0A0
          *           * Storage space for the 16 built in characters (0 - F).
          *      - Segment 3: 0x200 - 0xFFF
          *           * The instructions from the ROM will be loaded in this segment starting at address 0x200.
          */
        uint8_t m_memory[MEMORY] = { DEFAULT };

        /**
         * @brief Registers are dedicated location on the CPU for storage. Since the operations done by the CPU are carried out
         * in its registers, we need a way to represent the 16 8-bit registers CHIP-8 had. A typical operation will load data
         * from memory into the registers, operate on those registers, and if applicable store the result back into memory.
         * The following are some of the properties of CHIP-8 registers:
         *      - Register labels : V0 - VF
         *      - Range of values : 0x00 - 0xFF
         *      - Register VF     : flag to store the information about the result of operation
         */
        uint8_t m_registers[REGISTER] = { DEFAULT };

        /**
         * @brief The index Register is used to store the memory addresses for use in current operation. It's 16-bits because it must
         * holds values between 0x0000u and 0x0FFFu.
         */
        uint16_t m_indexRegister = DEFAULT;

        /**
         * @brief The program counter holds the address of the next instruction to execute. It's 16-bits because it must holds values
         * between 0x0000u and 0x0FFFu.
         */
        uint16_t m_programCounter = DEFAULT;

        /**
         * @brief A 16-level stack is used to keep track of the order of executions when opcodes are called that allows the program
         * to jump to a certain address or call a subroutine. The program stack holds the program counter value when the CALL instruction
         * was executed. The RETURN statement pulls that address from the stack and puts it back into the program counter so the CPU can
         * execute it during the next cycle. Since the program stack has 16 levels, it can hold up to 16 different program counters.
         */
        uint16_t m_programStack[STACK] = { DEFAULT };

        /**
         * @brief With the program stack represented as a fixed size array (with 16 elements), we'll need a way to store the location of the most
         * recent value placed in our stack. An 8-bit stack pointer will suffice as it only stores the index (current location) in our stack array.
         * When a value is popped off from the stack, the value is not going to be deleted from the array. Instead, the value will be copied and the
         * stack pointer will be decremented by 1 so it points to the previous value in the stack. When a value is pushed to the stack, the program
         * counter will be incremented by 1, and overwrite whatever value is written at the current location.
         */
        uint8_t m_stackPointer = DEFAULT;

        /**
         * @brief The timer used by CHIP-8 has a simple function: If the value in the timer is 0, it stays 0. If it was loaded with a value,
         * it will start decrementing at a rate of 60Hz. However, to keep things simple, this delay timer will start decrementing at the rate
         * of the cycle clock. In effect, this leads to the frame rate of the game being tied to how fast each cycle runs. To decouple the framerate
         * from from each cycle run, you have two options. One, you can call sleep after each cycle (i.e. std::this_thread::sleep_for(TIME)).
         * Two, you can calculate the delta time (elapsed time) between the last and current cycle. If delta time is greater than the cycle delay -
         * run a new cycle. While the second method is a bit more complex, its going to yield a more consistent result than calling thread sleep.
         */
        uint8_t m_delayTimer = DEFAULT;

        /**
         * @brief The sound timer works similarly to the delay timer. However, when the sound timer is not zero, a single tone will buzz.
         * This interpreter will not implement this feature. - TODO: implement at a later time??
         */
        uint8_t m_soundTimer = DEFAULT;

        /**
         * @brief An opcode is two bytes, since the memory is addressed as a single byte, when it comes time to fetch an instruction from memory, a byte
         * has to be fetched form the (program counter) and another from (program counter + 1). The two values will then need to be connected into a single
         * value. The program counter will have to be incremented by 2 since it is incremented by 1 before an instruction is executed.
         */
        uint16_t m_opcode = DEFAULT;

        // Internal flag determines if interpreter has been initalized. For cycle to run, initialization must be successful.
        bool m_initialized = false;

        // Internal flag determines if ROM was successfully loaded. For cycle to run, a valid rom must loaded into memory.
        bool m_romFlag = false;

        /**
         * @brief CHIP-8 has an instruction that places a random number into a register (Cxkk). This interpreter will use
         * C++'s STL default random engine - mt19937 - to generate sudo random number and bind to the values to a byte range.
         */
        std::default_random_engine m_randomEngine;
        std::uniform_int_distribution<uint8_t> randomByte;

    public:
        explicit Chip8();
        ~Chip8() = default;

        /**
         * @brief Initializes interpreter to a clean state so ROM file can be loaded.
         *
         * @returns true  - all values were successfully set to default state
         * @returns false - one or more values were not successfully cleared
         */
        bool Init();

        /**
         * @brief Loads the given ROM file and all of its instructions into memory starting at address 0x200.
         *
         * @param filename - path to ROM file to load
         *
         * @returns true  - ROM file successfully loaded into memory
         * @returns false - Emulator not initalized/ROM can't be located/ROM too big for memory
         */
        bool LoadROM(const char* filename);

        /**
         * @brief Three things happen during an emulation cycle:
         *    1 - Fetch the next instruction as an opcode
         *    2 - Decode the instruction to determine operation to conduct
         *    3 - Execute the instruction
         */
        void RunCycle();

        uint8_t* keyBuffer() { return m_keypad; }
        uint32_t* graphicsBuffer() { return m_graphics; }

    private:
        /**
         * +--------------------------------------------------------------------------------------------------------------------------+
         * |                                                                                                                          |
         *?|                                                 CHIP-8 Helper Functions                                                  |
         * |                                                                                                                          |
         * +--------------------------------------------------------------------------------------------------------------------------+
         */

         /**
          * @brief Decodes opcode (instruction) and executes the operation. Since CHIP-8 has a relatively small set of opcodes to
          * implement, emulate simply uses a switch statement to decode and execute the current opcode. For more complex systems,
          * that have more opcode, function pointer tables offer a more scalable solution.
          */
        void emulate();

        // @brief Fills memory from 0x050 to 0x0A0 with fontset
        void loadFonts();

        // @brief print simple error messages to console
        inline bool error(const char* msg) const {
            std::cerr << msg << std::endl;
            return false;
        }

        // @brief prints debug msg to console, used to print current opcode
        void debug_msg(const char* msg);

        // @brief dumps value store in all variables
        void debug_dump();

    private:
        /**
         * +--------------------------------------------------------------------------------------------------------------------------+
         * |                                                                                                                          |
         *?|                                                     CHIP-8 opcodes                                                       |
         * |                                                                                                                          |
         * +--------------------------------------------------------------------------------------------------------------------------+
         */

         // @brief CLS: clear screen
        void op_00E0();

        // @brief RET: return from subroutine
        void op_00EE();

        // @brief JP addr: jump to location nnn, interpreter sets program counter to nnn
        void op_1nnn();

        // @brief CALL addr: call subroutine at nnn
        void op_2nnn();

        // @brief SE Vx, byte: skip next instruction if Vx == kk
        void op_3xkk();

        // @brief SNE Vx, byte: skip next instruction if Vx != kk
        void op_4xkk();

        // @brief SE Vx, Vy: skip next instruction if Vx = Vy
        void op_5xy0();

        // @brief LD Vx, byte: set Vx = kk
        void op_6xkk();

        // @brief ADD Vx, byte: set Vx = Vx + ll
        void op_7xkk();

        // @brief LD Vx, Vy: set Vx, Vy
        void op_8xy0();

        // @brief OR Vx, Vy: set Vx = Vx OR Vy
        void op_8xy1();

        // @brief AND Vx, Vy: set Vx = Vx AND Vy
        void op_8xy2();

        // @brief XOR Vx, Vy: set Vx = Vx XOR Vy
        void op_8xy3();

        // @brief ADD Vx, Vy: set Vx = Vx + Vy, VF = carry
        void op_8xy4();

        // @brief SUB Vx, VY: set Vx = Vx - Vy, VF = NOT borrow
        void op_8xy5();

        // @brief SR Vx: set Vx = Vx SHR 1
        void op_8xy6();

        // @brief SUBN Vx, Vy: set Vx = Vy - Vx, VF = NOT borrow
        void op_8xy7();

        // @brief SL Vx { , Vy}: set Vx = Vx SHL 1
        void op_8xyE();

        // @brief SNE Vx, Vy: skip next instruction if Vx != Vy
        void op_9xy0();

        // @brief LD I, addr: set I = nnn
        void op_Annn();

        // @brief JP V0, addr: jump to location nnn + V0
        void op_Bnnn();

        // @brief RND Vx, byte: set Vx = random byte AND kk
        void op_Cxkk();

        // @brief DRW Vx, Vy, nibble: display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        void op_Dxyn();

        // @brief SKP Vx: skip next instruction if the key with the value of Vx is pressed
        void op_Ex9E();

        // @brief SKNP  Vx: skip next instruction if key with the value of Vx is not pressed
        void op_ExA1();

        // @brief LP Vx, DT: set delay timer value
        void op_Fx07();

        // @brief LP Vx, K: wait for a key press, store the value of the key in Vx
        void op_Fx0A();

        // @brief LP DT, Vx: set delay timer = Vx
        void op_Fx15();

        // @brief LP ST, Vx: set sound timer to Vx
        void op_Fx18();

        // @brief Add I, Vx: set I = I + Vx
        void op_Fx1E();

        // @brief set I = location of sprite for digit Vx
        void op_Fx29();

        // @brief LP B, Vx: stores BCD representation of Vx in memory location I, I+1, and I+2
        void op_Fx33();

        // @brief LP [I], Vx: stores registers V0 through Vx in memory starting at location I
        void op_Fx55();

        // @brief LP Vx, [I]: read registers V0 through Vx from memory starting at location I
        void op_Fx65();
    };
}
