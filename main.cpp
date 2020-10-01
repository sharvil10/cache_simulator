#include "simulator.hpp"

using namespace std;

int main()
{
    string path = "../MachineProblem1_Fall2020/traces/perl_trace.txt";
    unsigned int block_size = 16;
    unsigned int l1_size = 1024;
    unsigned int l1_assoc = 1;
    unsigned int l2_size = 0;
    unsigned int l2_assoc = 0;
    char inclusion_policy = 0;
    char replacement_policy = 0;    
    Simulator sim(block_size, l1_size, l1_assoc, l2_size, l2_assoc, replacement_policy, inclusion_policy, path);
    sim.execute();
}
