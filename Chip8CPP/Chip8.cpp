#include "Chip8.h"
	
	//Construct/Destruct

	Chip8::Chip8() {
		setupSound();
		populateOpcodeLookup();
		reset();
	}

	Chip8::~Chip8() {
		delete screen;
	}

	//Private
	void Chip8::setupSound() {
		soundEngine.InitialiseAudio(44100, 1);
		sineWave = olc::sound::Wave(1, sizeof(uint8_t), 44100, 44100);

		for (size_t i = 0; i < (44100 / 40); i++)
		{
			double dt = 1.0 / 44100.0;
			double t = double(i) * dt;
			sineWave.file.data()[i] = float(0.5 * sin(2.0 * 440.0 * 3.14159 * t));
		}
	}

	void Chip8::populateOpcodeLookup() {
		using c8 = Chip8;

		opcodeLookup = { 
			{ 0x0000, { "", nullptr, &c8::resolveOpcode0OrF } },
					{ 0x00E0, { "00E0", &c8::_00E0, nullptr } }, { 0x00EE, { "00EE", &c8::_00EE, nullptr } },
			
			{ 0x1000, { "1NNN", &c8::_1NNN, nullptr } }, { 0x2000, { "2NNN", &c8::_2NNN, nullptr } }, { 0x3000, { "3XNN", &c8::_3XNN, nullptr } }, { 0x4000, { "4XNN", &c8::_4XNN, nullptr } }, 
			{ 0x5000, { "5XYN", &c8::_5XYN, nullptr } }, { 0x6000, { "6XNN", &c8::_6XNN, nullptr } }, { 0x7000, { "7XNN", &c8::_7XNN, nullptr } },
			
			{ 0x8000, { "", nullptr, &c8::resolveOpcode8 } }, 
				{ 0x8001, { "8XY1", &c8::_8XY1, nullptr } }, { 0x8002, { "8XY2", &c8::_8XY2, nullptr } }, { 0x8003, { "8XY3", &c8::_8XY3, nullptr } },
				{ 0x8004, { "8XY4", &c8::_8XY4, nullptr } }, { 0x8005, { "8XY5", &c8::_8XY5, nullptr } },{ 0x8006, { "8XY6", &c8::_8XY6, nullptr } }, { 0x8007, { "8XY7", &c8::_8XY7, nullptr } },
				{ 0x800E, { "8XYE", &c8::_8XYE, nullptr } }, { 0x800F, { "8XY0", &c8::_8XY0, nullptr } },
			
			{ 0x9000, { "9XY0", &c8::_9XY0, nullptr } }, { 0xA000, { "ANNN", &c8::_ANNN, nullptr } }, { 0xB000, { "BNNN", &c8::_BNNN, nullptr } }, { 0xC000, { "CXNN", &c8::_CXNN, nullptr } },
			{ 0xD000, {"DXYN", &c8::_DXYN, nullptr} },
			
			{ 0xE000, { "", nullptr, &c8::resolveOpcodeE } }, 
				{ 0xE001, { "EXA1", &c8::_EXA1, nullptr } }, { 0xE00E, { "EX9E", &c8::_EX9E, nullptr } },
			
			{ 0xF000, { "", nullptr, &c8::resolveOpcode0OrF } },
				{ 0xF007, { "FX07", &c8::_FX07, nullptr } }, { 0xF00A, { "FX0A", &c8::_FX0A, nullptr } }, { 0xF015, { "FX15", &c8::_FX15, nullptr } },
				{ 0xF018, { "FX18", &c8::_FX18, nullptr } }, { 0xF01E, { "FX1E", &c8::_FX1E, nullptr } }, { 0xF029, { "FX29", &c8::_FX29, nullptr } }, { 0xF033, { "FX33", &c8::_FX33, nullptr } },
				{ 0xF055, { "FX55", &c8::_FX55, nullptr } }, { 0xF065, { "FX65", &c8::_FX65, nullptr } },
		};
	}

	std::vector<uint8_t> Chip8::systemFont() {
		return {
			0xF0, 0x90, 0x90, 0x90, 0xF0,
			0x20, 0x60, 0x20, 0x20, 0x70,
			0xF0, 0x10, 0xF0, 0x80, 0xF0,
			0xF0, 0x10, 0xF0, 0x10, 0xF0,
			0x90, 0x90, 0xF0, 0x10, 0x10,
			0xF0, 0x80, 0xF0, 0x10, 0xF0,
			0xF0, 0x80, 0xF0, 0x90, 0xF0,
			0xF0, 0x10, 0x20, 0x40, 0x40,
			0xF0, 0x90, 0xF0, 0x90, 0xF0,
			0xF0, 0x90, 0xF0, 0x10, 0xF0,
			0xF0, 0x90, 0xF0, 0x90, 0x90,
			0xE0, 0x90, 0xE0, 0x90, 0xE0,
			0xF0, 0x80, 0x80, 0x80, 0xF0,
			0xE0, 0x90, 0x90, 0x90, 0xE0,
			0xF0, 0x80, 0xF0, 0x80, 0xF0,
			0xF0, 0x80, 0xF0, 0x80, 0x80
		};
	}

	void Chip8::loadFont() {
		std::vector<uint8_t> font = systemFont();
		memory.insert(memory.begin(), font.begin(), font.end());
	}

	//Public

	void Chip8::loadRom(std::string fileName) {
		std::vector<uint8_t> program = std::vector<uint8_t>();
		std::ifstream fileStream;
		
		fileStream.open(fileName, std::ifstream::binary | std::ifstream::ate);

		if (fileStream.is_open()) {
			std::streampos size = fileStream.tellg();
			program.resize(size);
			fileStream.seekg(0);
			fileStream.read((char*)program.data(), size);

			memory.insert(memory.begin() + 0x200, program.begin(), program.end());
			
			fileStream.close();
		}
	}

	bool Chip8::isRunning() {
		return running;
	}

	bool Chip8::frameUpdated() {
		return frameChanged;
	}

	void Chip8::run() {
		running = true;
	}

	void Chip8::stop() {
		running = false;
	}

	void Chip8::reset() {
		screenData = std::array<uint8_t, (64 * 32)>();
		screen = new olc::Sprite(64, 32);
		memory = std::vector<uint8_t>(0xfff, 0);
		registers = std::array<uint8_t, 16>();
		stack = std::stack<uint16_t>();
		counters = { 0, 0x200, 0, 0, 0 };

		std::vector<uint8_t> program = { 0x18, 0x45, 0xF7, 0x18 };

		loadFont();
		loadRom("../test");
	}

	std::vector<uint8_t> Chip8::getMemory() {
		return memory;
	}

	int Chip8::getRandomNumber() {
		std::random_device seeder;
		std::ranlux48 generator(seeder());
		std::uniform_int_distribution<int> distribution(0, 255);

		return distribution(generator);
	}

	olc::Sprite& Chip8::getScreen() {
		if (frameChanged) {
			frameChanged = false;
		}

		return *screen;
	}

	void Chip8::step() {
		counters[0] = getOpcode();
		counters[1] += 2;
		setupOpcode(counters[0]);
		handleOpcode(true);
	}

	void Chip8::clock() {
		handleTimers();

		for (int instruction = 0; instruction < perFrame; instruction++) {
			counters[0] = getOpcode();
			counters[1] += 2;
			setupOpcode(counters[0]);
			handleOpcode(true);
		}
	}

	int Chip8::getKeyPressed() {
		int keyPressed = -1;

		for (int key = 0; key < keyState.size(); key++) {
			if (keyState[key] > 0) {
				keyPressed = key;
				break;
			}
		}

		return keyPressed;
	}

	void Chip8::handleTimers() {
		if (counters[3] > 0) {
			counters[3]--;
		}

		if (counters[4] > 0) {
			playSound();
			counters[4]--;
		}
	}

	void Chip8::playSound() {
		soundEngine.PlayWaveform(&sineWave);
	}

	uint16_t Chip8::getOpcode() {
		return memory[counters[1]] << 8 | memory[counters[1] + 1];
	}

	uint16_t Chip8::getSpecifiedOpcode(uint16_t programPosition) {
		return memory[programPosition] << 8 | memory[programPosition + 1];
	}

	std::array<uint16_t, 5> Chip8::getCounters() {
		return counters;
	}

	std::stack<uint16_t> Chip8::getStack() {
		return stack;
	}

	std::array <uint8_t, 16> Chip8::getRegisters() {
		return registers;
	}

	void Chip8::handleOpcode(bool executeOpcode) {
		currentInstruction = opcodeLookup[o];

		if (currentInstruction.resolver != nullptr) {
			(this->*currentInstruction.resolver)();
		}

		if (executeOpcode) {
			(this->*currentInstruction.operate)();
		}
	}

	void Chip8::setupOpcode(uint16_t opcode) {
		o = opcode & 0xF000;
		x = (opcode & 0x0F00) >> 8;
		y = (opcode & 0x00F0) >> 4;
		n = opcode & 0x000F;
		nn = opcode & 0x00FF;
		nnn = opcode & 0x0FFF;
	}


	void Chip8::resolveOpcode0OrF() {
		currentInstruction = opcodeLookup[o ^ nn];
	}

	void Chip8::resolveOpcode8() {
		if (n == 0x0000) {
			currentInstruction = opcodeLookup[o ^ 0x000F]; //Because 0x8000 is a valid opcode needed to add this slight hack...
		}
		else {
			currentInstruction = opcodeLookup[o ^ n];
		}
	}

	void Chip8::resolveOpcodeE() {
		currentInstruction = opcodeLookup[o ^ n];
	}

	void Chip8::_00E0() {
		screenData = std::array<uint8_t, 64 * 32>();

		for (int posX = 0; posX < 64; x++) {
			for (int posY = 0; posY < 32; y++) {
				screen->SetPixel({ posX, posY }, olc::BLACK);
			}
		}
	}

	void Chip8::_00EE() {
		counters[1] = stack.top();
		stack.pop();
	}

	void Chip8::_1NNN() {
		counters[1] = nnn;
	}

	void Chip8::_2NNN() {
		stack.push(counters[1]);
		counters[1] = nnn;
	}

	void Chip8::_3XNN() {
		if (registers[x] == nn) counters[1] += 2;
	}

	void Chip8::_4XNN() {
		if (registers[x] != nn) counters[1] += 2;
	}

	void Chip8::_5XYN() {
		if (registers[x] == registers[y]) counters[1] += 2;
	}

	void Chip8::_6XNN() {
		registers[x] = nn;
	}

	void Chip8::_7XNN() {
		if ((nn + registers[x]) > 255) nn -= 0x100;

		registers[x] += nn;
	}

	void Chip8::_8XY0() {
		registers[x] = registers[y];
	}

	void Chip8::_8XY1() {
		registers[x] |= registers[y];
	}

	void Chip8::_8XY2() {
		registers[x] &= registers[y];
	}

	void Chip8::_8XY3() {
		registers[x] ^= registers[y];
	}

	void Chip8::_8XY4() {
		registers[0xF] = 0;

		uint8_t value = registers[x] + registers[y];

		if (value > 255) {
			registers[0xF] = 1;
			value -= 0x100;
		}

		registers[x] = value;
	}

	void Chip8::_8XY5() {
		registers[0xF] = 1;

		if (registers[x] < registers[y]) registers[0xF] = 0;

		registers[x] -= registers[y];

		if (registers[x] < 0) registers[x] += 0x100;
	}

	void Chip8::_8XY6() {
		registers[0xF] = registers[x] & 0x1;
		registers[x] >>= 1;
	}

	void Chip8::_8XY7() {
		registers[0xF] = 1;

		if (registers[y] < registers[x]) registers[0xF] = 0;

		registers[x] = registers[y] - registers[x];

		if (registers[x] < 0) registers[x] += 0x100;
	}

	void Chip8::_8XYE() {
		registers[0xF] = registers[x] & 0x80;
		registers[x] <<= 1;

		if (registers[x] > 255) registers[x] -= 0x100;
	}

	void Chip8::_9XY0() {
		if (registers[x] != registers[y]) counters[1] += 2;
	}
	
	void Chip8::_ANNN() {
		counters[2] = nnn;
	}
	
	void Chip8::_BNNN() {
		counters[1] = registers[0] + nnn;
	}
	
	void Chip8::_CXNN() {
		registers[x] = getRandomNumber() & nn;
	}

	void Chip8::_DXYN() {
		bool collision = false;
		uint8_t mask = 0x80;
		uint8_t data;
		registers[0xF] = 0;

		for (int lineY = 0; lineY < n; lineY++) {
			data = memory[counters[2] + lineY];

			for (int lineX = 0; lineX < 8; lineX++) {
				uint8_t posX = registers[x] + lineX;
				uint8_t posY = registers[y] + lineY;

				if ((data & mask) > 0) {
					if (posX > 64) posX -= 64;
					if (posX < 0) posX += 64;
					if (posY > 32) posY -= 32;
					if (posY < 0) posY -= 32;

					olc::Pixel currentPixel = screen->GetPixel({ posX, posY });

					if (currentPixel == olc::WHITE) {
						screen->SetPixel({ posX, posY }, olc::BLACK);
						collision = true;
					}
					else {
						screen->SetPixel({ posX, posY }, olc::WHITE);
					}

					if (collision) registers[0xF] = 1;
				}

				data <<= 1;
			}
		}

		frameChanged = true;
	}
	
	void Chip8::_EX9E() {
		if (keyState[registers[x]] == 1) counters[1] += 2;
	}

	void Chip8::_EXA1() {
		if (keyState[registers[x]] == 0) counters[1] += 2;
	} 	
	
	void Chip8::_FX07() {
		registers[x] = counters[3];
	}
	
	void Chip8::_FX0A() {
		int keyPressed = getKeyPressed();

		if (keyPressed != -1) {
			registers[x] = keyPressed;
		}
		else {
			counters[1] -= 2;
		}
	}
	
	void Chip8::_FX15() {
		counters[3] = registers[x];
	}
	
	void Chip8::_FX18() {
		counters[4] = registers[x];
	}
	
	void Chip8::_FX1E() {
		registers[0xF] = 0;

		if ((counters[2] + registers[x]) > 0xFFF) registers[0xF] = 1;

		counters[2] = ((counters[2] + registers[x]) % 0xF000);
	}
	
	void Chip8::_FX29() {
		counters[2] = registers[x] * 5;
	}
	
	void Chip8::_FX33() {
		uint8_t units = registers[x] % 10;
		uint8_t tens = (registers[x] / 10) % 10;
		uint8_t hundreds = registers[x] / 100;

		memory[counters[2] + 2] = units;
		memory[counters[2] + 1] = tens;
		memory[counters[2]] = hundreds;
	}
	
	void Chip8::_FX55() {
		for (int reg = 0; reg <= x; reg++) {
			memory[counters[2] + reg] = registers[reg];
		}
	}
	
	void Chip8::_FX65() {
		for (int reg = 0; reg <= x; reg++) {
			registers[reg] = memory[counters[2] + reg];
		}
	}

	void Chip8::XXX() {
		std::cout << "This instruction is not implemented. Also, this should never happen...So err...Something probably broke" << std::endl;
	}

	std::map<uint16_t, std::string> Chip8::disassemble(uint16_t start, uint16_t end) {
		uint16_t address = start;
		uint16_t lineAddress = 0;

		std::map<uint16_t, std::string> lines;

		while (address < end) {
			if ((address + 1) >= memory.size()) break;

			lineAddress = address;

			std::string instructionLine = "$" + hex(address, 4) + ": ";
			setupOpcode(getSpecifiedOpcode(address));
			handleOpcode(false);
			address++;

			instructionLine += currentInstruction.name + " ";

			if (currentInstruction.name == "???") {
				instructionLine += "$" + hex(nnn, 4) + " {nnn}";
			}
			else {
				switch (o) {
				case 0x0000:
					break;
				case 0x1000:
				case 0x2000:
				case 0xA000:
				case 0xB000:
					instructionLine += "$" + hex(nnn, 4) + " {nnn}";
					break;
				case 0x3000:
				case 0x4000:
				case 0x6000:
				case 0x7000:
				case 0xC000:
					instructionLine += "$" + hex(x, 2) + " {x} " + "$" + hex(nn, 4) + " {nn} ";
					break;
				case 0x5000:
				case 0x8000:
				case 0x9000:
				case 0xD000:
					instructionLine += "$" + hex(x, 2) + " {x} " + "$" + hex(y, 2) + " {y} ";
					break;
				case 0xE000:
				case 0xF000:
					instructionLine += "$" + hex(x, 2) + " {x}";
					break;
				default:
					break;
				}
			}

			lines[lineAddress] = instructionLine;
		}

		return lines;
	}