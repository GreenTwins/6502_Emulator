#include <stdio.h>
#include <stdlib.h>


//This will be an emulator for the 6502- an old 8 bit microprocessor that was used in MOS's KIM-1, Apple 1 & 2 with access to 64k bytes of RAM. Mem addresses are 16 bits in length w/ valid add from 0-65535
//its called a 1 address machine b/c instructions can reference at most 1 addr at a time( its a 1 instruction per clock cycle system. So the exe time of an instruction is tightly coupled with the clock speed)
//, and most of those instructions use the accumulator implicitly w/o the programmer specifying it directly 

//info: https://people.cs.umass.edu/~verts/cmpsci201/spr_2004/Lecture_02_2004-01-30_The_6502_processor.pdf
//info: https://www.masswerk.at/6502/6502_instruction_set.html

//only has 3 registers and is little endian

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

struct Mem { //the mem the "computer" will need to use"
	static constexpr u32 MAX_MEM = 1024 * 64; //this gives full valid addr to 65535
	Byte Data[MAX_MEM];

	void initialize() {
		for (u32 i = 0; i < MAX_MEM; ++i) {
			Data[i] = 0;
		}
	}
	//read 1 byte
	Byte operator[](u32 Address)const {
		return Data[Address];
	}

	//write 1 byte
	Byte& operator[](u32 Address) {
		return Data[Address];
	}
};

struct CPU {
	

	Word PC;//program counter
	Word SP;//stack pointer

	Byte Accum, X, Y;//8 bit registers

	 //processor status flags
	Byte C : 1;
	Byte Z : 1;
	Byte I : 1;
	Byte D : 1;
	Byte B : 1;
	Byte V : 1;
	Byte N : 1;

	void Reset(Mem& m) {
		PC = 0xFFFC;
		D=Z=I=B=V=N=C= 0; //clear  flag
		SP = 0x0100;
		Accum = X = Y = 0;
		m.initialize();
	}
	Byte Fetch(Mem& m,u32 &Cycles ) {
		Byte Data=m[PC];
		++PC;
		--Cycles;
		return Data;
	}
	Byte ReadByte(Mem& m, u32& cycles, Byte Address) {
		Byte Data = m[Address];
		--cycles;
		return Data;
	}

	static constexpr Byte  INS_LDA_IM = 0xA9;//OpCode load accumulator immediate mode
	static constexpr Byte INS_LDA_ZP = 0xA5;//Opcode load accymulator zero page

	void LDASetStatus() {
		Z = (Accum == 0);
		N = (Accum & 0b10000000) > 0;
	}
	void Execute(u32 Cycles, Mem& m) {
		while (Cycles > 0) {
			//fetch the next instruction from mem wherever program counter is
			Byte Instruction = Fetch(m, Cycles);
			/*(void)Instruction;*/
			switch (Instruction) {
			case INS_LDA_IM: {
				Byte Value = Fetch(m, Cycles);
				Accum = Value;
				LDASetStatus();
			}
			break;
			case INS_LDA_ZP: {
				Byte ZeroPageAddress = Fetch(m, Cycles);
				Accum = ReadByte(m, Cycles, ZeroPageAddress);
				LDASetStatus();
			}break;
			default:
				printf("Instruction not handled");
				break;
			}
		}
	}
};

int main() {

	Mem mem;
	CPU newCpu;

	newCpu.Reset(mem);
	/*mem[0xFFFC] = CPU::INS_LDA_IM;
	mem[0xFFFD] = 0x42;*/
	mem[0xFFFC] = CPU::INS_LDA_ZP;
	mem[0xFFFD] = 0x42;
	mem[0x0042] = 84;
	newCpu.Execute(3, mem);

	return 0;
}