#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

// author: Sze Hong Cheung<sc8235@nyu.edu>
// #define __LOCAL_ENV__
#ifdef __LOCAL_ENV__
const string cwd = "/Users/jesse/Desktop/CSA/lab2/";
#endif

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
            #ifdef __LOCAL_ENV__
                string path = cwd + "RFresult.txt";
            #else
                string path = "RFresult.txt";
            #endif
			ofstream rfout;
			rfout.open(path,std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open RFresult file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
            #ifdef __LOCAL_ENV__
                string path = cwd + "imem.txt";
            #else
                string path = "imem.txt";
            #endif
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open(path);
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
                    if(line.length() >= 8) {
                        IMem[i] = bitset<8>(line.substr(0, 8));
                        i++;
                    }
				}                    
			}
            else cout<<"Unable to open imem file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            #ifdef __LOCAL_ENV__
                string path = cwd + "dmem.txt";
            #else
                string path = "dmem.txt";
            #endif
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(path);
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    if(line.length() >= 8) {
                        DMem[i] = bitset<8>(line.substr(0, 8));
                        i++;
                    }
                }
            }
            else cout<<"Unable to open dmem file"<<endl;;
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            #ifdef __LOCAL_ENV__
                string path = cwd + "dmemresult.txt";
            #else
                string path = "dmemresult.txt";
            #endif
            dmemout.open(path);
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open dmemresult file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    #ifdef __LOCAL_ENV__
        string path = cwd + "stateresult.txt";
    #else
        string path = "stateresult.txt";
    #endif
    printstate.open(path, std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open stateresult file";
    printstate.close();
}
 

int main()
{
    cout<< "Running" << endl;
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
			
    stateStruct state;
    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.IF.PC = 0;
    int cycle = 0;
    bool isHalt = false;
    bool stall = false;
    while (1) {
        cout<< "cycle: " << cycle << endl;
        stateStruct newState;
        newState.IF.nop = false;
        newState.ID.nop = true;
        newState.EX.nop = true;
        newState.EX.wrt_enable = false;
        newState.MEM.nop = true;
        newState.WB.nop = true;
        auto& id = state.ID;
        auto& ex = state.EX;
        auto& wb = state.WB;
        auto& mem = state.MEM;
        stall = false;
        /* --------------------- WB stage --------------------- */
        if (!wb.nop) {
            if (wb.wrt_enable) {
                myRF.writeRF(wb.Wrt_reg_addr, wb.Wrt_data);
            }
        }

        /* --------------------- MEM stage --------------------- */
        if (!mem.nop) {
            if (mem.wrt_mem) {
                cout << "mem.rt: " << mem.Rt.to_ulong() <<endl;
                cout << "wb.Wrt_reg_addr: " << wb.Wrt_reg_addr.to_ulong() <<endl;
                if (mem.Rt.to_ulong() == wb.Wrt_reg_addr.to_ulong()) {
                    cout << "store by forwarding: " << wb.Wrt_data <<endl;
                    myDataMem.writeDataMem(mem.ALUresult, wb.Wrt_data); //forwarding
                } else {
                    cout << "store normally: " << mem.Store_data <<endl;
                    myDataMem.writeDataMem(mem.ALUresult, mem.Store_data); 
                }
            }
            newState.WB.Wrt_data = mem.rd_mem ? myDataMem.readDataMem(mem.ALUresult) : mem.ALUresult;
            newState.WB.Rs = mem.Rs;
            newState.WB.Rt = mem.Rt;
            newState.WB.Wrt_reg_addr = mem.Wrt_reg_addr;
            newState.WB.wrt_enable = mem.wrt_enable;
            newState.WB.nop = mem.nop; 
        }
        newState.MEM.nop = ex.nop || mem.wrt_mem; 

        /* --------------------- EX stage --------------------- */
        
        if (!ex.nop) {
            // check if 
            long data1;
            long data2;
            if (ex.Rs == mem.Wrt_reg_addr && mem.wrt_enable) {
                //EX-EX forwarding
                data1 = mem.ALUresult.to_ulong();
            } else if (ex.Rs == wb.Wrt_reg_addr && wb.wrt_enable) {
                //MEM-EX forwarding
                data1 = wb.Wrt_data.to_ulong();
            } else {
                data1 = ex.Read_data1.to_ulong();
            }
            if (ex.Rt == mem.Wrt_reg_addr && mem.wrt_enable) {
                //EX-EX forwarding
                data2 = mem.ALUresult.to_ulong();
            } else if (ex.Rt == wb.Wrt_reg_addr && wb.wrt_enable) {
                //MEM-EX forwarding
                data2 = wb.Wrt_data.to_ulong();
            } else {
                data2 = ex.Read_data2.to_ulong();
            }
            if (ex.is_I_type) {
                newState.MEM.ALUresult = data1 + (ex.Imm.to_ulong()); 
            } else {
                newState.MEM.ALUresult = data1 + (ex.alu_op ? 1 : -1) * data2;
            }
                newState.MEM.Store_data = ex.Read_data2; 
                if (ex.wrt_mem && ex.Rt.to_ulong() == wb.Wrt_reg_addr.to_ulong()) {
                    newState.MEM.Store_data = wb.Wrt_data;
                }
                newState.MEM.Rs = ex.Rs;
                newState.MEM.Rt = ex.Rt;
                newState.MEM.Wrt_reg_addr = ex.Wrt_reg_addr;
                newState.MEM.rd_mem = ex.rd_mem;
                newState.MEM.wrt_mem = ex.wrt_mem;
                newState.MEM.wrt_enable = ex.wrt_enable;
        }
        newState.MEM.nop = ex.nop; 

        /* --------------------- ID stage --------------------- */
        bool quash = false;
        int jumpAddr = 0;
        
        if (!id.nop) {
            const auto instr = id.Instr.to_ulong();
            const auto opCode = (instr & 0xfc000000) >> 26;
            const auto rs = (instr & 0x03e00000) >> 21;
            const auto rt = (instr & 0x001f0000) >> 16;
            const auto rd = (instr & 0x0000f800) >> 11;
            const auto funct = instr & 0x0000003f;
            const auto imm = instr & 0x0000ffff;
            const auto signExtendImm = (int32_t) (id.Instr[15] == 1 ? (imm | 0xffff0000) : imm);
            const auto isRType = opCode == 0;
            const auto isIType = !isRType;
            const auto isBranch = opCode == 0x04;
            const auto wrt_enable = !(opCode == 0x2b || isBranch); // not sw, beq;
            jumpAddr = state.IF.PC.to_ulong() + signExtendImm * 4;

            newState.EX.Read_data1 = myRF.readRF(rs);
            newState.EX.Read_data2 = myRF.readRF(rt);
            newState.EX.Imm = signExtendImm;
            newState.EX.Rs = rs;
            newState.EX.Rt = rt;
            newState.EX.Wrt_reg_addr = isIType ? rt : rd;
            newState.EX.is_I_type = isIType;
            newState.EX.rd_mem = opCode == 0x23;
            newState.EX.wrt_mem = opCode == 0x2b;
            newState.EX.alu_op = funct != 0x23;
            newState.EX.wrt_enable = wrt_enable;
            cout << "stall before id: " << stall <<endl;
            if (isBranch) {
                if (newState.EX.Read_data1 != newState.EX.Read_data2) {
                    quash = true;
                }
            } else if ((newState.EX.rd_mem && ex.wrt_enable && (ex.Wrt_reg_addr == rs || ex.Wrt_reg_addr == rt))
                || (ex.rd_mem && (ex.Wrt_reg_addr.to_ulong() == newState.EX.Rt.to_ulong() || ex.Wrt_reg_addr.to_ulong() == newState.EX.Rs.to_ulong()))) 
                {
                stall = true;
            }
            cout << "ex.wrt_enable: " << ex.wrt_enable << endl;
            cout << "ex.Wrt_reg_addr: " << ex.Wrt_reg_addr << endl;
            cout << "rt: " << rt << endl;
            cout << "stall after id: " << stall << endl;
        }
        newState.EX.nop = id.nop;

        /* --------------------- IF stage --------------------- */
        newState.ID.nop = state.IF.nop;
        if (state.IF.nop || isHalt) {
            newState.ID.nop = true;
            newState.IF.nop = true;
        } else {
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            const auto instr = newState.ID.Instr.to_ulong();
            const auto opCode = (instr & 0xfc000000) >> 26;
            isHalt = isHalt || (opCode == 0x3F);
            if (stall) {
                newState.ID.Instr = id.Instr;
                if (isHalt) {
                    newState.IF.nop = true;
                } else {
                    newState.IF.PC = state.IF.PC;
                }
            } else {
                if (quash) {
                    newState.ID.nop = true;
                    newState.IF.PC = jumpAddr;
                    isHalt = false;
                } else if (isHalt) {
                    newState.ID.nop = true;
                    newState.IF.nop = true;
                } else {
                    newState.IF.PC = state.IF.PC.to_ulong() + 4;
                }
            }
        }
       
        
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle++); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
        // cout << "stall: " << stall << endl;
        // cout << "isHalt: " << isHalt << endl;
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}