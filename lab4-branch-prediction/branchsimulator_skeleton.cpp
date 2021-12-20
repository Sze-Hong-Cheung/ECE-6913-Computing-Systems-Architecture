#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

using namespace std;

// #define __LOCAL__

int main (int argc, char** argv) {
	ifstream config;
#ifdef __LOCAL__
	string configPath = "/Users/jesse/Desktop/CSA/lab4-branch-prediction/config.txt";
	string tracePath = "/Users/jesse/Desktop/CSA/lab4-branch-prediction/trace.txt";
#else
	string configPath = argv[1];
	string tracePath = argv[2];
#endif
	config.open(configPath);

	int m;
	config >> m;
 
	config.close();

	ofstream out;
	string out_file_name = string(tracePath) + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(tracePath);
	vector<int> pht(pow(2, m), 3);

	while (!trace.eof()) {
		unsigned long pc; bool taken;
		trace >> std::hex >> pc >> taken;
		bool prediction;
		
		auto lsb = pc & static_cast<unsigned long>(pow(2, m) - 1);

		cout << pc << ": " << taken << endl;
		cout << "lsb: " << lsb << endl;

		prediction = (pht[lsb] >> 1) & 1;

		cout << "pht[lsb]: " << pht[lsb] << endl;
		cout << "prediction: " << prediction << endl;

		out << prediction << endl;

		pht[lsb] = taken ? min(pht[lsb] + 1, 3) : max(pht[lsb] - 1, 0);
		
	}
	 
	trace.close();	
	out.close();
}
