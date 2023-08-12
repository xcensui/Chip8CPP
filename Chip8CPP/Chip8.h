#pragma once
#include <ranges>
#include <map>
#include <fstream>
#include <random>
#include <stack>
#include "olcPixelGameEngine.h"
#include "olcSoundWaveEngine.h"

class Chip8
{
	private:
		std::string hex(uint32_t n, uint8_t d)
		{
			std::string s(d, '0');
			for (int i = d - 1; i >= 0; i--, n >>= 4)
				s[i] = "0123456789ABCDEF"[n & 0xF];
			return s;
		};

		olc::sound::WaveEngine soundEngine;
		olc::sound::Wave sineWave;
		olc::Sprite* screen;
		std::vector<uint8_t> memory;
		std::array<uint16_t, 5> counters;
		std::array<uint8_t, 16> registers;
		std::stack<uint16_t> stack;
		std::array<uint8_t, 16> keyState;

		std::array<uint8_t, 64 * 32> screenData;

		const float perFrame = std::floor((400 / 60));

		struct INSTRUCTION {
			INSTRUCTION() : name("???"), operate(&Chip8::XXX), resolver(nullptr) {}
			INSTRUCTION(std::string n, void(Chip8::*o)(), void(Chip8::* r)()) : name(n), operate(o), resolver(r) {}
			std::string name;
			void(Chip8::* operate)(void) = nullptr;
			void(Chip8::* resolver)(void) = nullptr;
		};

		std::unordered_map<uint16_t, INSTRUCTION> opcodeLookup;

		INSTRUCTION currentInstruction;

		bool frameChanged;
		bool running;

		uint16_t n;
		uint16_t nn;
		uint16_t nnn;
		uint16_t o;
		uint16_t x;
		uint16_t y;

		std::vector<uint8_t> systemFont();
		void loadFont();
		void populateOpcodeLookup();
		void setupSound();

		void loadRom(std::string fileName);

		void resolveOpcode0OrF();
		void resolveOpcode8();
		void resolveOpcodeE();

		uint16_t getOpcode();
		uint16_t getSpecifiedOpcode(uint16_t programPosition);
		void setupOpcode(uint16_t opcode);
		void handleOpcode(bool executeOpcode);
		void handleTimers();
		int getRandomNumber();
		int getKeyPressed();
		void playSound();

		void XXX(); 
		
		void _00E0(); void _00EE();
		void _1NNN(); void _2NNN(); void _3XNN(); void _4XNN(); void _5XYN(); void _6XNN(); void _7XNN();
		void _8XY0(); void _8XY1(); void _8XY2(); void _8XY3(); void _8XY4(); void _8XY5(); void _8XY6(); void _8XY7(); void _8XYE();
		void _9XY0(); void _ANNN(); void _BNNN(); void _CXNN(); void _DXYN(); void _DXYNB();
		void _EXA1(); void _EX9E();
		void _FX07(); void _FX0A(); void _FX15(); void _FX18(); void _FX1E(); void _FX29(); void _FX33(); void _FX55(); void _FX65();

	public:
		Chip8();
		~Chip8();
		void reset();
		olc::Sprite& getScreen();
		void clock();
		void step();
		void run();
		void stop();
		bool isRunning();
		bool frameUpdated();
		std::vector<uint8_t> getMemory();
		std::array<uint16_t, 5> getCounters();
		std::stack<uint16_t> getStack();
		std::array <uint8_t, 16> getRegisters();

		std::map<uint16_t, std::string> disassemble(uint16_t start, uint16_t end);
};

