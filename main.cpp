#include "simulator.hpp"

using namespace std;

string my_filename(string& path)
{
    size_t tmp = path.find_last_of("/\\");
    return path.substr(tmp+1);
}

int main(int argc, char **argv)
{
    unsigned int block_size = atoi(argv[1]);
    unsigned int l1_size = atoi(argv[2]);
    unsigned int l1_assoc = atoi(argv[3]);
    unsigned int l2_size = atoi(argv[4]);
    unsigned int l2_assoc = atoi(argv[5]);
    char replacement_policy = atoi(argv[6]);
    char inclusion_policy = atoi(argv[7]);
    string repl_string;
    string incl_string;

    switch(replacement_policy)
    {
        case LRU:
            repl_string = "LRU";
            break;
        case PLRU:
            repl_string = "Pseudo-LRU";
            break;
        case OPT:
            repl_string = "Optimal";
            break;
    }

    switch(inclusion_policy)
    {
        case NON_INCLUSIVE:
            incl_string = "non-inclusive";
            break;
        case INCLUSIVE:
            incl_string = "inclusive";
            break;
    }

    string path = argv[8];
    cout << "===== Simulator configuration =====" << endl;
    cout << "BLOCKSIZE:             " << block_size << endl; 
    cout << "L1_SIZE:               " << l1_size << endl;
    cout << "L1_ASSOC:              " << l1_assoc << endl;
    cout << "L2_SIZE:               " << l2_size << endl;
    cout << "L2_ASSOC:              " << l2_assoc << endl;
    cout << "REPLACEMENT POLICY:    " << repl_string << endl;
    cout << "INCLUSION PROPERTY:    " << incl_string << endl;
    cout << "trace_file:            " << my_filename(path) << endl;
    Simulator sim(block_size, l1_size, l1_assoc, l2_size, l2_assoc, replacement_policy, inclusion_policy, path);
    sim.execute();
}


