#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int data_red[2500];
int data_ir[2500];

int main(int argc, char **argv)
{
    string datum;
    int incr_red = 0, incr_ir = 0;
    int red_flag = 0, ir_flag = 0;
    
	ifstream inFile("max30102_sample_data_two_minutes.txt");
    
    while(inFile >> datum) {
        
        if(datum == "RED") {
            red_flag = 1;
            ir_flag = 0;
        }
        
        if(datum == "IR") {
            red_flag = 0;
            ir_flag = 1;
        }
        
        if(red_flag) {
            for(int i = incr_red; i < incr_red + 100; i++) {
                inFile >> datum;
                data_red[i] = stoi(datum,nullptr,10);
            }
            incr_red += 100;
        }
        
        if(ir_flag) {
            for(int i = incr_ir; i < incr_ir + 100; i++) {
                inFile >> datum;
                data_ir[i] = stoi(datum,nullptr,10);
            }
            incr_ir += 100;
        }
        
    }
    
    inFile.close();

    ofstream outFile_red("data_two_minutes_red.txt");
    for(int i = 0; i < 2500; i++) {
        outFile_red << data_red[i] << endl;
    }
    outFile_red.close();

    ofstream outFile_ir("data_two_minutes_ir.txt");
    for(int i = 0; i < 2500; i++) {
        outFile_ir << data_ir[i] << endl;
    }
    outFile_ir.close();
    
    

	return 0;
}
