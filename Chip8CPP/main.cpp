#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_SOUNDWAVE
#include "olcSoundWaveEngine.h"

#include "Chip8.h"

class PGE : public olc::PixelGameEngine {
	protected:
		Chip8 chip8;
		float residualTime = 0.0f;
		bool showDebugging;
		int debugLayer = 0;
		std::map<uint16_t, std::string> disassembly;

		std::string hex(uint32_t n, uint8_t d)
		{
			std::string s(d, '0');
			for (int i = d - 1; i >= 0; i--, n >>= 4)
				s[i] = "0123456789ABCDEF"[n & 0xF];
			return s;
		};

	public:
		uint16_t startingAddressPage = 0x200;
		uint16_t addressPage;

		PGE() {
			sAppName = "Chip 8";
			showDebugging = true;
		}

		bool OnUserCreate() override {
			disassembly = chip8.disassemble(0x0000, 0xFFFF);
			Clear(olc::BLANK);
			debugLayer = CreateLayer();
			return true;
		}

		bool OnUserUpdate(float fElapsedTime) override {
			if (GetKey(olc::Key::R).bPressed) {
				chip8.isRunning() ? chip8.stop() : chip8.run();
			}

			if (GetKey(olc::Key::D).bPressed) {
				showDebugging = (showDebugging) ? false : true;
			}

			if (GetKey(olc::Key::SPACE).bPressed) {
				chip8.reset();
			}

			if (GetKey(olc::Key::S).bPressed) {
				chip8.step();
				drawScreen();
			}

			if (GetKey(olc::Key::NP_ADD).bPressed) {
				startingAddressPage += (16*16);
			}

			if (GetKey(olc::Key::NP_SUB).bPressed) {
				startingAddressPage -= (16 * 16);
			}

			if (GetKey(olc::Key::ESCAPE).bPressed) {
				return false;
			}

			if (residualTime > 0.0f) {
				residualTime -= fElapsedTime;
			}
			else {
				SetDrawTarget(debugLayer);
				Clear(olc::DARK_BLUE);

				if (showDebugging) {
					drawCpu(450, 10);
					drawRam(2, 240, 16, 16);
					drawCode(450, 130, 26);
				}
				
				EnableLayer(debugLayer, true);
				SetDrawTarget(nullptr);


				if (chip8.isRunning()) {
					residualTime += (1.0f / 60.0f) - fElapsedTime;
					chip8.clock();

					drawScreen();
				}
			}

			return true;
		}

		void drawCode(int x, int y, int numberOfLines) {
			std::map<uint16_t, std::string>::iterator currentIterator = disassembly.find(chip8.getCounters()[1]);
			int lineY = (numberOfLines >> 1) * 10 + y;

			if (currentIterator != disassembly.end()) {
				DrawString(x, lineY, (*currentIterator).second, olc::YELLOW);

				while (lineY < (numberOfLines * 10) + y) {
					lineY += 10;

					if (++currentIterator != disassembly.end()) {
						DrawString(x, lineY, (*currentIterator).second);
					}
				}
			}

			currentIterator = disassembly.find(chip8.getCounters()[1]);
			lineY = (numberOfLines >> 1) * 10 + y;

			if (currentIterator != disassembly.end()) {
				while (lineY > y) {
					lineY -= 10;

					if (--currentIterator != disassembly.end()) {
						DrawString(x, lineY, (*currentIterator).second);
					}
				}
			}
		}


		void drawScreen() {
			DrawSprite({ 10, 10 }, &chip8.getScreen(), 4);
		}

		void drawCpu(int x, int y) {
			std::array<uint16_t, 5> counters = chip8.getCounters();
			std::array<uint8_t, 16> registers = chip8.getRegisters();
			std::stack<uint16_t> stack = chip8.getStack();

			DrawString(x, y, "REGISTERS:", olc::WHITE);
			int currentY = y + 12;
			int currentX = x;
			int regDrawn = 0;

			for (uint8_t reg = 0x00; reg < registers.size(); reg++) {
				DrawString(currentX + (regDrawn * 28), currentY, "V" + hex(reg, 1));
				DrawString(currentX + (regDrawn * 28), currentY + 12, +"$" + hex(registers[reg], 2), olc::GREEN);

				regDrawn++;

				if (regDrawn % 8 == 0) {
					currentY += 24;
					currentX = x;
					regDrawn = 0;
				}
			}

			DrawString(x, currentY, "PC: $" + hex(counters[1], 4));

			DrawString(x, currentY + 12, "STACK: ", olc::WHITE);
			int stackPosition = 0;

			while (stack.size() > 0) {
				DrawString(x + (stackPosition * 14), currentY + 24, "$" + hex(stack.top(), 4) + " ", olc::GREY);
				stack.pop();
				stackPosition++;
			}
		}

		void drawRam(int x, int y, int rows, int columns) {
			addressPage = startingAddressPage;
			std::vector<uint8_t> memory = chip8.getMemory();
			int ramX = x;
			int ramY = y;

			for (int row = 0; row < rows; row++) {
				std::string value = "$" + hex(addressPage, 4) + ":";

				for (int col = 0; col < columns; col++) {
					value += " " + hex(memory[addressPage], 2);
					addressPage += 1;
				}

				DrawString(ramX, ramY, value, olc::WHITE);
				ramY += 10;
			}
		}
};

int main() {
	PGE renderer;

	if (renderer.Construct(720, 400, 2, 2, false, true)) {
		renderer.Start();
	}

	return 0;
}