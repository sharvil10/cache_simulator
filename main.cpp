#include "trace.hpp"
#include "cache.hpp"

using namespace std;

int main()
{
    string path = "../MachineProblem1_Fall2020/traces/compress_trace.txt";
    Trace tr(path);
    uint32_t size = 256;
    uint32_t assoc = 1;
    uint32_t block_size = 32;
    Cache l1(size, block_size, assoc);
    tr.clean();
}
