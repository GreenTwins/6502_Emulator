#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//This will be an emulator for the 6502- an old 8 bit microprocessor that was used in MOS's KIM-1, Apple 1 & 2 with access to 64k bytes of RAM. Mem addresses are 16 bits in length w/ valid add from 0-65535
//its called a 1 address machine b/c instructions can reference at most 1 addr at a time( its a 1 instruction per clock cycle system. So the exe time of an instruction is tightly coupled with the clock speed)
//, and most of those instructions use the accumulator implicitly w/o the programmer specifying it directly 

//info: https://people.cs.umass.edu/~verts/cmpsci201/spr_2004/Lecture_02_2004-01-30_The_6502_processor.pdf
//info: https://www.masswerk.at/6502/6502_instruction_set.html
//info 2: http://www.6502.org/tutorials/6502opcodes.html#JMP

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
	//write 2 bytes
	void WriteWord(u32& Cycles, Word Value, u32 Address) {
		Data[Address] = Value & 0xFF;
		Data[Address + 1] = (Value >> 8);
		Cycles -= 2;
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
	Word FetchWord(u32& Cycles, Mem& m) {
		//little endian
		Word Data = m[PC];
		++PC;
		--Cycles;

		Data |= (m[PC] << 8);//shift up 8 bits
		++PC;

		Cycles -= 2;

		//if you want to handle endianness you will swap bytes here

		return Data;
	}

	Byte ReadByte(Mem& m, u32& cycles, Byte Address) {
		Byte Data = m[Address];
		--cycles;
		return Data;
	}
	static constexpr Byte
		INS_LDA_IM = 0xA9,//OpCode load accumulator immediate mode 2 cycles
		INS_LDA_ZP = 0xA5, //zero page  3 cycles
		INS_LDA_ZPX = 0xB5, //zero page x 4 cycles
		INS_JSR = 0x20; // 3 bytes and 6 cycles


	void LDASetStatus() {
		Z = (Accum == 0);
		N = (Accum & 0b10000000) > 0;
	}
	void Execute(u32 Cycles, Mem& m) {
		while (Cycles > 0) {
			//fetch the next instruction from mem wherever program counter is
			//Part of LOAD accumulator with memory
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
			case INS_LDA_ZPX: {
				Byte ZeroPageAddress = Fetch(m, Cycles);
				ZeroPageAddress += X;
				--Cycles;
				Accum = ReadByte(m, Cycles, ZeroPageAddress);
				LDASetStatus();
			}break;
			case INS_JSR: {
				Word SubAddress = FetchWord(Cycles, m);
				m.WriteWord(Cycles, PC - 1, SP);//this just does whats below
				/*m[SP] = PC - 1;
				--Cycles;*/
				PC = SubAddress; //JSR returns control back to the "next" addr 
				--Cycles;
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
	//mem[0xFFFC] = CPU::INS_LDA_ZP;
	//mem[0xFFFD] = 0x42;
	//mem[0x0042] = 84;
	mem[0xFFFC] = CPU::INS_JSR;
	mem[0xFFFD] = 0x42;
	mem[0xFFFE] = 0x42;
	mem[0x4242] = CPU::INS_LDA_IM;
	mem[0x4243] = 0x84;
	newCpu.Execute(9, mem); //get total num of instructions required

	return 0;
}