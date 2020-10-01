#include "simulator.hpp"

using namespace std;

int main()
{
    string path = "../MachineProblem1_Fall2020/traces/compress_trace.txt";
    unsigned int block_size = 32;
    unsigned int l1_size = 8192;
    unsigned int l1_assoc = 4;
    unsigned int l2_size = 262144;
    unsigned int l2_assoc = 8;
    char inclusion_policy = 0;
    char replacement_policy = 0;    
    Simulator sim(block_size, l1_size, l1_assoc, l2_size, l2_assoc, replacement_policy, inclusion_policy, path);
}
