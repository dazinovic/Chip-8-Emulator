/**
 *	@file	chip8.cpp
 *	@author	Dejan Azinovic (dazinovic)
 *	@date	01.03.2017
 *
 *	Contains an implementation of all methods from the chip8 header.
 */

#include "chip8.h"
#include <cstring>
#include <iostream>
#include <fstream>

Chip8::Chip8()
{
	init();
}

Chip8::~Chip8()
{
}

// Initializes the Chip-8 Emulator
void Chip8::init()
{
	// Reset memory, registers, screen and keys
	memset(memory, 0, 4096);
	memset(V, 0, 16);
	memset(screen, 0, 64 * 32);
	memset(keys, 0, 16);

	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
	delay_timer = 0;
	sound_timer = 0;
	soundEnabled = true;

	// Load the fontset
	memcpy(memory, fontset, 80);
}

// Decodes the opcode 0xxx.
void Chip8::decodeOpcode0()
{
	(this->*(opcode0DecodeTable[(opcode & 0x0002) >> 1]))();
}

// 00E0 - Clears the screen.
void Chip8::clearScreen()
{
	memset(screen, 0, 64 * 32);
}

// 00EE - Returns from a subroutine.
void Chip8::returnFromSubroutine()
{
	pc = stack[--sp];
}

// 1NNN - Jumps to address NNN.
void Chip8::jumpToAddress()
{
	pc = (opcode & 0x0FFF) - 2;
}

// 2NNN - Calls subroutine at NNN.
void Chip8::callSubroutine()
{
	stack[sp++] = pc;
	pc = (opcode & 0x0FFF) - 2;
}

// 3XNN - Skips the next instruction if VX equals NN.
void Chip8::skipInstructionIfEqualsN()
{
	if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
	{
		pc += 2;
	}
}

// 4XNN - Skips the next instruction if VX doesn't equal NN.
void Chip8::skipInstructionIfNotEqualsN()
{
	if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
	{
		pc += 2;
	}
}

// 5XY0 - Skips the next instruction if VX equals VY.
void Chip8::skipInstructionIfEquals()
{
	if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
	{
		pc += 2;
	}
}

// 6XNN - Sets VX to NN.
void Chip8::setToN()
{
	V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
}

// 7XNN - Adds NN to VX.
void Chip8::AddN()
{
	V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
}

// Decodes the opcode 8xxx.
void Chip8::decodeOpcode8()
{
	if ((opcode & 0x0008) == 0)
	{
		(this->*(opcode8DecodeTable[opcode & 0x0007]))();
	}
	else
	{
		(this->*(opcode8DecodeTable[8]))();
	}
}

// 8XY0 - Sets VX to the value of VY.
void Chip8::assign()
{
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
}

// 8XY1 - Sets VX to VX or VY (Bitwise OR operation).
void Chip8::bitwiseOr()
{
	V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
}

// 8XY2 - Sets VX to VX and VY (Bitwise AND operation).
void Chip8::bitwiseAnd()
{
	V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
}

// 8XY3 - Sets VX to VX xor VY (Bitwise XOR operation).
void Chip8::bitwiseXor()
{
	V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
}

// 8XY4 - Adds VY to VX. VF is set to 1 when there's a carry,
//        and to 0 when there isn't.
void Chip8::add()
{
	V[0xF] = (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4]) >> 8;
	V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
}

// 8XY5 - VY is subtracted from VX. VF is set to 0 when there's a borrow,
//        and 1 when there isn't.
void Chip8::subtract()
{
	V[0xF] = (V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4]);
	V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
}

// 8XY6 - Shifts VX right by one. VF is set to the value of the least
//        significant bit of VX before the shift.
void Chip8::bitwiseShiftRight()
{
	V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x0001;
	V[(opcode & 0x0F00) >> 8] >>= 1;
}

// 8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow,
//        and 1 when there isn't.
void Chip8::reverseSubtract()
{
	V[0xF] = (V[(opcode & 0x0F00) >> 8] <= V[(opcode & 0x00F0) >> 4]);
	V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
}

// 8XYE - Shifts VX left by one. VF is set to the value of the most
//        significant bit of VX before the shift.
void Chip8::bitwiseShiftLeft()
{
	V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
	V[(opcode & 0x0F00) >> 8] <<= 1;
}

// 9XY0 - Skips the next instruction if VX doesn't equal VY.
void Chip8::skipInstructionIfNotEquals()
{
	if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
	{
		pc += 2;
	}
}

// ANNN - Sets I to the address NNN.
void Chip8::setI()
{
	I = opcode & 0x0FFF;
}

// BNNN - Jumps to the address NNN plus V0.
void Chip8::jumpToAddressPlus()
{
	pc = (opcode & 0x0FFF) + V[0] - 2;
}

// CXNN - Sets VX to the result of a bitwise and operation on a random number (0 to 255) and NN.
void Chip8::setRandom()
{
	V[(opcode & 0x0F00) >> 8] = rand() & opcode & 0x00FF;
}

// DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
//        Each row of 8 pixels is read as bit-coded starting from memory location I;
//        I value doesn’t change after the execution of this instruction. As described above,
//        VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
//        and to 0 if that doesn’t happen.
void Chip8::drawSprite()
{
	unsigned char N  = opcode & 0x000F;
	unsigned char x = V[(opcode & 0x0F00) >> 8];
	unsigned char y = V[(opcode & 0x00F0) >> 4];

	V[0xF] = 0;
	for (int i = 0; i < N; i++)
	{
		unsigned char row = memory[I + i];
		for (int j = 0; j < 8; j++)
		{
			if ((row & (0x80 >> j)) != 0)
			{
				if (screen[64 * (y + i) + x + j] == 1)
				{
					V[0xF] = 1;
				}
				screen[64 * (y + i) + x + j] ^= 1;
			}
		}
	}
}

// Decodes the opcode Exxx.
void Chip8::decodeOpcodeE()
{
	(this->*(opcodeEDecodeTable[opcode & 0x0001]))();
}

// EX9E - Skips the next instruction if the key stored in VX is pressed.
//        (Usually the next instruction is a jump to skip a code block)
void Chip8::skipIfKeyPressed()
{
	if (keys[V[(opcode & 0x0F00) >> 8]] == 1)
	{
		pc += 2;
	}
}

// EXA1 - Skips the next instruction if the key stored in VX isn't pressed.
//        (Usually the next instruction is a jump to skip a code block)
void Chip8::skipIfKeyNotPressed()
{
	if (keys[V[(opcode & 0x0F00) >> 8]] == 0)
	{
		pc += 2;
	}
}

// Decodes the opcode Fxxx.
void Chip8::decodeOpcodeF()
{
	switch (opcode & 0x00FF)
	{
	case 0x0007:
		getDelay();
		break;
	case 0x000A:
		getKey();
		break;
	case 0x0015:
		setDelay();
		break;
	case 0x0018:
		setSound();
		break;
	case 0x001E:
		addToI();
		break;
	case 0x0029:
		findCharacter();
		break;
	case 0x0033:
		setBCD();
		break;
	case 0x0055:
		storeRegisters();
		break;
	case 0x0065:
		loadRegisters();
		break;
	}
}

// FX07 - Sets VX to the value of the delay timer.
void Chip8::getDelay()
{
	V[(opcode & 0x0F00) >> 8] = delay_timer;
}

// FX0A - A key press is awaited, and then stored in VX.
//        (Blocking Operation. All instruction halted until next key event)
void Chip8::getKey()
{
	bool keyPressed = false;
	for (int i = 0; i < 16; i++)
	{
		if (keys[i] != 0)
		{
			keyPressed = true;
			V[(opcode & 0x0F00) >> 8] = i;
			break;
		}
	}
	if (!keyPressed)
	{
		pc -= 2;
	}
}

// FX15 - Sets the delay timer to VX.
void Chip8::setDelay()
{
	delay_timer = V[(opcode & 0x0F00) >> 8];
}

// FX18 - Sets the sound timer to VX.
void Chip8::setSound()
{
	sound_timer = V[(opcode & 0x0F00) >> 8];
}

// FX1E - Adds VX to I.
void Chip8::addToI()
{
	V[0xF] = (I + V[(opcode & 0x0F00) >> 8]) >> 16;
	I += V[(opcode & 0x0F00) >> 8];
}

// FX29 - Sets I to the location of the sprite for the character in VX.
//        Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void Chip8::findCharacter()
{
	I = (V[(opcode & 0x0F00) >> 8] & 0x0F) * 5;
}

// FX33 - Stores the binary-coded decimal representation of VX, with the most
//        significant of three digits at the address in I, the middle digit at
//        I plus 1, and the least significant digit at I plus 2.
//        (In other words, take the decimal representation of VX, place the
//        hundreds digit in memory at location in I, the tens digit at location I+1,
//        and the ones digit at location I+2.)
void Chip8::setBCD()
{
	memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
	memory[I + 1] = (V[(opcode & 0x0F00) >> 8] % 100) / 10;
	memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
}

// FX55 - Stores V0 to VX (including VX) in memory starting at address I.
void Chip8::storeRegisters()
{
	memcpy(memory + I, V, ((opcode & 0x0F00) >> 8) + 1);
}

// FX65 - Fills V0 to VX (including VX) with values from memory starting at address I.
void Chip8::loadRegisters()
{
	memcpy(V, memory + I, ((opcode & 0x0F00) >> 8) + 1);
}

// Loads a Chip-8 application into memory starting from address 0x200
bool Chip8::LoadApplication(const char * filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in.good())
	{
		in.seekg(0, std::ios::end);
		int length = int(in.tellg());
		if (length > 4096 - 512)
		{
			std::cout << "The application is too big." << std::endl;
			return false;
		}

		in.seekg(0, std::ios::beg);
		in.read(reinterpret_cast<char *>(memory + 512), length);
		in.close();

		return true;
	}

	std::cerr << "Error opening application file." << std::endl;
	return false;
}

// Emulates one cycle of the Chip8-Emulator
void Chip8::EmulateCycle()
{
	try
	{
		// Fetch opcode
		opcode = memory[pc] << 8 | memory[pc + 1];

		// Process opcode
		(this->*(decodeTable[(opcode & 0xF000) >> 12]))();
		pc += 2;

		// Update timers
		if (delay_timer > 0)
		{
			--delay_timer;
		}

		if (sound_timer > 0)
		{
			if (sound_timer == 1)
			{
				std::cout << "BEEP!" << std::endl;
				if (soundEnabled)
				{
					std::cout << '\a';
				}
			}
			--sound_timer;
		}
	}
	catch (const std::exception &exc)
	{
		std::cerr << "Exception: " << exc.what() << std::endl;
	}
}