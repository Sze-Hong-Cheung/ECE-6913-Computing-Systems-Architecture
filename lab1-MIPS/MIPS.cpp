#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include<typeinfo>
using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

//#define __LOCAL_ENV__

#ifdef __LOCAL_ENV__
    string cwd = "/Users/jesse/Desktop/lab1/";
#else
    string cwd = "";
#endif

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.

#define MemSize 65536

unsigned long getBits(bitset<32> instruction, int start, int end) {
    unsigned long mask = 0;
    for (int i = 0; i <= start - end; i++) {
        mask = mask << 1;
        mask += 1;
    }
    for (int i = 0; i < end; i++) {
        mask = mask << 1;
    }
    return (instruction.to_ulong() & mask) >> end;
}

class RF {
	public:
	    bitset<32> ReadData1, ReadData2;
	RF() {
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}
	void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable) {
		// TODO: implement!
		ReadData1 = Registers[RdReg1.to_ulong()];
		ReadData2 = Registers[RdReg2.to_ulong()];
		if (WrtEnable.test(0)) {
			Registers[WrtReg.to_ulong()] = WrtData;
		}
	}
	void OutputRF() {
		ofstream rfout;

		rfout.open(cwd + "RFresult.txt", std::ios_base::app);
		if (rfout.is_open()) {
			rfout<<"A state of RF:"<<endl;
			for (int j = 0; j<32; j++) {
				rfout << Registers[j] << endl;
			}
		} else cout << "Unable to open file RFresult" << endl;
		rfout.close();
	}
	private:
	    vector<bitset<32> > Registers;
};


class ALU {
	public:
	    bitset<32> ALUresult;
	bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2) {
		// TODO: implement!
        
		const unsigned long op = ALUOP.to_ulong();
		if (op == ADDU) {
			ALUresult = oprand1.to_ulong() + oprand2.to_ulong();
		} else if (op == SUBU) {
			ALUresult = oprand1.to_ulong() - oprand2.to_ulong();
		} else if (op == AND) {
			ALUresult = oprand1 & oprand2;
		} else if (op == OR) {
			ALUresult = oprand1 | oprand2;
		} else if (op == NOR) {
			ALUresult = (oprand1 | oprand2).flip();
		} else {
			ALUresult = oprand2;
		}
        cout << "op: " << op << endl;
        cout << "oprand1: " << oprand1 << endl;
        cout << "oprand2: " << oprand2 << endl;
        cout << "ALUresult: " << ALUresult << endl;
		return ALUresult;
	}
}
;
class INSMem {
	public:
	    bitset<32> Instruction;
	INSMem() {
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i=0;

		imem.open(cwd + "imem.txt");
		if (imem.is_open()) {
			while (getline(imem,line)) {
				if(line.length() >= 8) {
					IMem[i] = bitset<8>(line.substr(0, 8));
					i++;
				}
			}
		} else cout << "Unable to open file imem.txt";
		imem.close();
	}
	bitset<32> ReadMemory (bitset<32> ReadAddress) {
		// TODO: implement!
		// (Read the byte at the ReadAddress and the following three byte).
		unsigned long pc = ReadAddress.to_ulong();
		string bits = "";
		for (int i = 0; i < 4; i++) {
			bits += IMem[pc + i].to_string();
		}
		Instruction = bitset<32>(bits);
		cout << "Instruction: " << bits << endl;
		return Instruction;
	}
	private:
	    vector<bitset<8> > IMem;
}
;
class DataMem {
	public:
	    bitset<32> readdata;
	DataMem() {
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i=0;

		dmem.open(cwd + "dmem.txt");
		if (dmem.is_open()) {
			while (getline(dmem,line)) {
                if (line.length() >= 8) {
                    DMem[i] = bitset<8>(line.substr(0, 8));
                    i++;
                }
			}
		} else cout << "Unable to open file dmem.txt" << endl;
		dmem.close();
	}
	bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) {
		// TODO: implement!
		const unsigned long address = Address.to_ulong();
		if (readmem.test(0)) {
			cout << "readmem: true" << endl;
			string bits = "";
			for (int i = 0; i < 4; i++) {
				bits += DMem[address + i].to_string();
			}
			readdata = bitset<32>(bits);
			cout << "readdata: " << readdata << endl;
		} else if (writemem.test(0)) {
			cout << "writemem: true" << endl;

			DMem[address] = WriteData.to_ulong() >> 24;
			DMem[address + 1] = (WriteData.to_ulong() >> 16) & 0xff;
			DMem[address + 2] = (WriteData.to_ulong() >> 8) & 0xff;
			DMem[address + 3] = WriteData.to_ulong() & 0xff;
		}
		return readdata;
	}
	void OutputDataMem() {
		ofstream dmemout;
		dmemout.open(cwd + "dmemresult.txt");
		if (dmemout.is_open()) {
			cout << "Open file dmemresult.txt" << endl;
			for (int j = 0; j< 1000; j++) {
				dmemout << DMem[j]<<endl;
			}
		} else cout << "Unable to open file dmemresult.txt" << endl;
		dmemout.close();
	}
	private:
	    vector<bitset<8> > DMem;
}
;


int main() {
	RF myRF;
	ALU myALU;
	INSMem myInsMem;
	DataMem myDataMem;
	int pc = 0;
	while (1) {
		// Fetch
	
		bitset<32> instruction = myInsMem.ReadMemory(pc);

		pc += 4;
        
        // If current insturciton is "11111111111111111111111111111111", then break;
		if (instruction.all()) {
			cout << "halt" << endl;
			break;
		}

		// decode(Read RF)
		unsigned long opCode = getBits(instruction, 31, 26);
        bool isRType = opCode == 0;
        bool isJType = opCode == 2;
        bool isIType = !(isRType || isJType);
        bool isLoad = opCode == 35;
        bool isStore = opCode == 43;
        unsigned long aluOp = 0;
        if (isLoad || isStore) {
            aluOp = ADDU;
        } else if (isRType) {
            aluOp = getBits(instruction, 3, 0);
        } else {
            aluOp = getBits(instruction, 28, 26);
        }
        bool isBranch = opCode == 4;
        bool wrtEnable = !(isStore || isBranch || isJType);

		// Execute
		// J-Type 0x02
		if (isJType) {
			int jumpAddress = ((pc & 0xf0000000) << 28) | ((instruction.to_ulong() & 0x03ffffff) << 2) & 0xfffffffc;
			pc = jumpAddress;
		} else {
            unsigned long rs = getBits(instruction, 25, 21);
            unsigned long rt = getBits(instruction, 20, 16);
            unsigned long rd = getBits(instruction, 15, 11);
            unsigned long shamt = getBits(instruction, 10, 6);		
            unsigned long funct = getBits(instruction, 5, 0);		
            unsigned long immediate = getBits(instruction, 15, 0);	
			unsigned long signExtendedImmediate = (int32_t) (instruction[15] == 1 ? (immediate | 0xffff0000) : immediate);

			bitset<32> loadData;
			myRF.ReadWrite(rs, rt, isRType ? rd : rt, loadData, wrtEnable);

			// I-Type 0x04
			if (opCode == 4 && myRF.ReadData1 == myRF.ReadData2) {
				pc += (signExtendedImmediate << 2);
			} else {
				bitset<32> output = myALU.ALUOperation(aluOp, myRF.ReadData1, isIType ? signExtendedImmediate : myRF.ReadData2);
				// Read/Write Mem
				loadData = myDataMem.MemoryAccess(output, myRF.ReadData2, isLoad, isStore);
				// Write back to RF
				myRF.ReadWrite(rs, rt, isRType ? rd : rt, isLoad ? loadData : output, wrtEnable);
			}
		}
		myRF.OutputRF();
		// dump RF;
	}
	myDataMem.OutputDataMem();
	// dump data mem
	return 0;
}
