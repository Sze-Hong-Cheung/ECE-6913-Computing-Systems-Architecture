/*
   Cache Simulator
   Level one L1 and level two L2 cache parameters are read from file
   (block size, line per set and set per cache).
   The 32 bit address is divided into:
   -tag bits (t)
   -set index bits (s)
   -block offset bits (b)

   s = log2(#sets)   b = log2(block size)  t=32-s-b

   
*/
// #define __LOCAL__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>


using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


struct config{
  int L1blocksize;
  int L1setsize;
  int L1size;
  int L2blocksize;
  int L2setsize;
  int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
   class cache {

   }
*/       
struct block {
  unsigned int addr;
  bool valid;
  bool dirty;
};

struct cache {
  u_int32_t blockSize;
  u_int32_t setSize;
  u_int32_t cacheSize;
  u_int32_t offset;
  u_int32_t index;
  u_int32_t tag;
  u_int32_t evictionCounter;
  vector<vector<block> > data;

  cache(int blockSize, int setSize, int cacheSize);
};

cache::cache(int blockSize, int setSize, int cacheSize) : blockSize(blockSize), setSize(setSize), cacheSize(cacheSize), evictionCounter(0) {

  if (setSize == 0) {
    this->setSize = 1;
  }

  data.resize(setSize);
  for (auto &&s : data) {
    s.resize(pow(2, 10) * cacheSize / blockSize / setSize);
  }

  offset = log2(blockSize);
  index = log2(pow(2, 10) * cacheSize / blockSize / setSize);
  tag = 32 - offset - index;
  cout << "Tag: " << tag << ", Offset: " << offset << ", Index: " << index << endl;
}

__wrap_iter<vector<block, allocator<block> > *> find_empty_way(cache &_cache, int index) {
  return find_if(begin(_cache.data), end(_cache.data), [&](const auto &way) {
    return !way[index].valid;
  });
}


int main(int argc, char* argv[]){



  config cacheconfig;
  ifstream cache_params;
  string dummyLine;
  #ifdef __LOCAL__
    cache_params.open("/Users/jesse/Desktop/CSA/lab3/cacheconfig.txt");
  #else
    cache_params.open(argv[1]);
  #endif
  while(!cache_params.eof())  // read config file
  {
    cache_params>>dummyLine;
    cache_params>>cacheconfig.L1blocksize;
    cache_params>>cacheconfig.L1setsize;              
    cache_params>>cacheconfig.L1size;
    cache_params>>dummyLine;              
    cache_params>>cacheconfig.L2blocksize;           
    cache_params>>cacheconfig.L2setsize;        
    cache_params>>cacheconfig.L2size;
  }



  // Implement by you: 
  // initialize the hirearch cache system with those configs
  // probably you may define a Cache class for L1 and L2, or any data structure you like

  cache L1_cache{cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size};
  cache L2_cache{cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size};


  int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;


  ifstream traces;
  ofstream tracesout;
  string outname;

  #ifdef __LOCAL__
    outname = "/Users/jesse/Desktop/CSA/lab3/trace.txt.out";
    traces.open("/Users/jesse/Desktop/CSA/lab3/trace.txt");
  #else
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
  #endif
  tracesout.open(outname.c_str());

  string line;
  string accesstype;  // the Read/Write access type from the memory trace;
  string xaddr;       // the address from the memory trace store in hex;
  unsigned int addr;  // the address from the memory trace store in unsigned int;        
  bitset<32> accessaddr; // the address from the memory trace store in the bitset;
  int countLine = 0;

  if (traces.is_open()&&tracesout.is_open()){    
    cout<<"traces is open"<<endl;
    while (getline (traces,line)){
      cout<<" "<< endl;
      cout<<"line "<< countLine << ": " << line << endl;
      countLine++;
      istringstream iss(line); 
      if (!(iss >> accesstype >> xaddr)) {break;}
      stringstream saddr(xaddr);
      saddr >> std::hex >> addr;
      accessaddr = bitset<32> (addr);
      L1AcceState = 0;
      L2AcceState = 0;

      unsigned int l1_offset = addr & (static_cast<int>(pow(2, L1_cache.offset) - 1));
      unsigned int l1_index = (addr >> L1_cache.offset) & (static_cast<int>(pow(2, L1_cache.index) - 1));
      unsigned int l1_tag = addr >> (L1_cache.offset + L1_cache.index);

      unsigned int l2_offset = addr & (static_cast<int>(pow(2, L2_cache.offset) - 1));
      unsigned int l2_index = (addr >> L2_cache.offset) & (static_cast<int>(pow(2, L2_cache.index) - 1));
      unsigned int l2_tag = addr >> (L2_cache.offset + L2_cache.index);

      // access the L1 and L2 Cache according to the trace;

      // match L1_cache
      auto match_way1 = -1;
      for (int i = 0; i < L1_cache.data.size(); i++) {
        if (((L1_cache.data[0][l1_index].addr) >> (L1_cache.index + L1_cache.offset)) == (addr >> (L1_cache.index + L1_cache.offset))) {
          match_way1 = i;
          break;
        }
      } 

      if (accesstype.compare("R")==0) {    
        //Implement by you:
        // read access to the L1 Cache, 
        //  and then L2 (if required), 
        //  update the L1 and L2 access state variable;

        if (match_way1 != -1) {
          // L1 Read Hit
          L1AcceState = RH;
          cout<<"L1: RH"<< endl;
        } else {
          // L1 Read Miss
          L1AcceState = RM;
          cout<<"L1: RM"<< endl;

          // mMtch L2
          int match_way2 = -1;
          for (int i = 0; i < L2_cache.data.size(); i++) {
            if (((L2_cache.data[0][l2_index].addr) >> (L2_cache.index + L2_cache.offset)) == (addr >> (L2_cache.index + L2_cache.offset))) {
              match_way2 = i;
              break;
            }
          }

          if (match_way2 != -1) {
            // L2: Read Hit
            L2AcceState = RH;
          } else {
            // L2: Read Miss
            L2AcceState = RM;
            cout<<"L2: RM"<< endl;

            // Find empty way in L2
            auto empty_way2 = find_empty_way(L2_cache, l2_index);

            if (empty_way2 != L2_cache.data.end()) {
              // L2: Write Hit
              cout<<"L2: Empty way found"<< endl;
              (*empty_way2)[l2_index].addr = addr;
              (*empty_way2)[l2_index].valid = true;
            } else {
              // L2: Write Miss, Evict
              cout<<"L2: Evict"<< endl;
              L2_cache.data[L2_cache.evictionCounter][l2_index] = {.addr = addr, .valid = true};
              L2_cache.evictionCounter = (++L2_cache.evictionCounter) >= L2_cache.setSize ? 0 : L2_cache.evictionCounter;
            }
          }

          // Write back to L1
          // Find empty way in L1
          auto empty_way1 = find_empty_way(L1_cache, l1_index);

          if (empty_way1 != L1_cache.data.end()) {
            // L1: Write Hit
            cout<<"L1: Empty way found"<< endl;
            (*empty_way1)[l1_index].addr = addr;
            (*empty_way1)[l1_index].valid = true;
          } else {
            // L1: Write Miss
            cout<<"L1: Evict"<< endl;
            L1_cache.data[L1_cache.evictionCounter][l1_index] = {.addr = addr, .valid = true};
            L1_cache.evictionCounter = (++L1_cache.evictionCounter) >= L1_cache.setSize ? 0 : L1_cache.evictionCounter;
          }
        
        } 
      }
      else {    
        //Implement by you:
        // write access to the L1 Cache, 
        //and then L2 (if required), 
        //update the L1 and L2 access state variable;

        if (match_way1 != -1) {
          // L1: Write Hot
          L1AcceState = WH;
          L1_cache.data[match_way1][l1_index].dirty = true;
          cout<<"L1: WH"<< endl;
        } else {
          // L1: Write Miss
          L1AcceState = WM;
          cout<<"L1: WM"<< endl;
          
          // Match L2
          int match_way2 = -1;
          for (int i = 0; i < L2_cache.data.size(); i++) {
            if (((L2_cache.data[0][l2_index].addr) >> (L2_cache.index + L2_cache.offset)) == (addr >> (L2_cache.index + L2_cache.offset))) {
              match_way2 = i;
              break;
            }
          } 
          if (match_way2 != -1) {
            // L2: Wirte Hit
            L2AcceState = WH;
            cout<<"L2: WH"<< endl;
            L2_cache.data[match_way2][l2_index].dirty = true;
          } else {
            // L2: Wirte Miss
            L2AcceState = WM;
            cout<<"L2: WM"<< endl;
          }
        }
      }
      // Output hit/miss results for L1 and L2 to the output file
      tracesout<< L1AcceState << " " << L2AcceState << endl;


    }
    traces.close();
    tracesout.close(); 
  }
  else cout<< "Unable to open trace or traceout file ";

  return 0;
}
