/**
 *	@file	chip8.h
 *	@author	Dejan Azinovic (dazinovic)
 *	@date	01.03.2017
 *
 *	Header file for the Chip8 class. The class allows for emulation of the
 *	Chip-8 interpreted programming language. Emulation must be done by the
 *	class user one cycle at a time.
 */

#ifndef CHIP8
#define CHIP8

class Chip8 {
	public:
		Chip8();
		~Chip8();
		
		const static unsigned int SCREEN_WIDTH  = 64;
		const static unsigned int SCREEN_HEIGHT = 32;

		void EmulateCycle();									// Emulate one cycle of the emulator.
		bool LoadApplication(const char *filename);				// Load a Chip-8 application from disk into memory.
		void ToggleSound() { soundEnabled = !soundEnabled; }	// Toggles sound off or on.

		unsigned char  screen[SCREEN_WIDTH * SCREEN_HEIGHT];	// Pixel state for all pixels of the emulator screen.
		unsigned char  keys[16];								// Key state for all keys of the emulator keypad.

	private:	
		unsigned short pc;				// Program counter.
		unsigned short opcode;			// Current opcode.
		unsigned short I;				// Index register.
		unsigned short sp;				// Stack pointer.
		
		unsigned char  V[16];			// V-regs (V0-VF).
		unsigned short stack[16];		// Stack (16 levels).
		unsigned char  memory[4096];	// Memory (size = 4k).
				
		unsigned char  delay_timer;		// Delay timer.
		unsigned char  sound_timer;		// Sound timer.
		bool		   soundEnabled;	// Whether or not the emulator will play the beep.

		void init();

		unsigned char fontset[80] =
		{
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

		// Opcode functions
		void decodeOpcode0();				// Decodes the opcode 0xxx.
		void clearScreen();					// 00E0 - Clears the screen.
		void returnFromSubroutine();		// 00EE - Returns from a subroutine.
		void jumpToAddress();				// 1NNN - Jumps to address NNN.
		void callSubroutine();				// 2NNN - Calls subroutine at NNN.
		void skipInstructionIfEqualsN();	// 3XNN - Skips the next instruction if VX equals NN.
		void skipInstructionIfNotEqualsN();	// 4XNN - Skips the next instruction if VX doesn't equal NN.
		void skipInstructionIfEquals();		// 5XY0 - Skips the next instruction if VX equals VY.
		void setToN();						// 6XNN - Sets VX to NN.
		void AddN();						// 7XNN - Adds NN to VX.
		void decodeOpcode8();				// Decodes the opcode 8xxx.
		void assign();						// 8XY0 - Sets VX to the value of VY.
		void bitwiseOr();					// 8XY1 - Sets VX to VX or VY (Bitwise OR operation).
		void bitwiseAnd();					// 8XY2 - Sets VX to VX and VY (Bitwise AND operation).
		void bitwiseXor();					// 8XY3 - Sets VX to VX xor VY (Bitwise XOR operation).
		void add();							// 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		void subtract();					// 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		void bitwiseShiftRight();			// 8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
		void reverseSubtract();				// 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		void bitwiseShiftLeft();			// 8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
		void skipInstructionIfNotEquals();	// 9XY0 - Skips the next instruction if VX doesn't equal VY.
		void setI();						// ANNN - Sets I to the address NNN.
		void jumpToAddressPlus();			// BNNN - Jumps to the address NNN plus V0.
		void setRandom();					// CXNN - Sets VX to the result of a bitwise and operation on a random number (0 to 255) and NN.
		void drawSprite();					// DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
											//        Each row of 8 pixels is read as bit-coded starting from memory location I;
											//        I value doesn’t change after the execution of this instruction. As described above,
											//        VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
											//        and to 0 if that doesn’t happen.
		void decodeOpcodeE();				// Decodes the opcode Exxx.
		void skipIfKeyPressed();			// EX9E - Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
		void skipIfKeyNotPressed();			// EXA1 - Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
		void decodeOpcodeF();				// Decodes the opcode Fxxx.
		void getDelay();					// FX07 - Sets VX to the value of the delay timer.
		void getKey();						// FX0A - A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
		void setDelay();					// FX15 - Sets the delay timer to VX.
		void setSound();					// FX18 - Sets the sound timer to VX.
		void addToI();						// FX1E - Adds VX to I.
		void findCharacter();				// FX29 - Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
		void setBCD();						// FX33 - Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
											//        the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX,
											//        place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
		void storeRegisters();				// FX55 - Stores V0 to VX (including VX) in memory starting at address I.
		void loadRegisters();				// FX65 - Fills V0 to VX (including VX) with values from memory starting at address I.

		// Decode table for the emulator opcodes
		void (Chip8::*decodeTable[16])() =
		{
			&Chip8::decodeOpcode0, &Chip8::jumpToAddress,
			&Chip8::callSubroutine, &Chip8::skipInstructionIfEqualsN,
			&Chip8::skipInstructionIfNotEqualsN, &Chip8::skipInstructionIfEquals,
			&Chip8::setToN, &Chip8::AddN,
			&Chip8::decodeOpcode8, &Chip8::skipInstructionIfNotEquals,
			&Chip8::setI, &Chip8::jumpToAddressPlus,
			&Chip8::setRandom, &Chip8::drawSprite,
			&Chip8::decodeOpcodeE, &Chip8::decodeOpcodeF
		};

		// Decode table for opcodes 0xxx
		void(Chip8::*opcode0DecodeTable[2])() =
		{
			&Chip8::clearScreen, &Chip8::returnFromSubroutine
		};

		// Decode table for opcodes 8xxx
		void(Chip8::*opcode8DecodeTable[9])() =
		{
			&Chip8::assign, &Chip8::bitwiseOr, &Chip8::bitwiseAnd, &Chip8::bitwiseXor,
			&Chip8::add, &Chip8::subtract, &Chip8::bitwiseShiftRight,
			&Chip8::reverseSubtract, &Chip8::bitwiseShiftLeft
		};

		// Decode table for opcodes Exxx
		void(Chip8::*opcodeEDecodeTable[2])() =
		{
			&Chip8::skipIfKeyPressed, &Chip8::skipIfKeyNotPressed
		};
};

#endif