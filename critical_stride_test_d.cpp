#include <iostream>
#include <functional>
#include <cstdlib>

#define NUM_PTRS 32
#define INT_SIZE sizeof(int)
#define MEM_ALLOCATION 64
#define NUM_ACCESSES 100000

using namespace std;

int main(int argc, char** argv)
{
    int stride = atoi(argv[1]);
    void* mem_ptrs[NUM_PTRS];
    for (int i = 0; i < NUM_PTRS; i++)
    {
        posix_memalign((void**) &mem_ptrs[i], stride, MEM_ALLOCATION);
    }
    for (int i = 0; i < NUM_PTRS; i++)
    {
      int rand_num = rand() % 128;
      *(((int*) (mem_ptrs[i])) + 0) = rand_num;
      *(((int*) (mem_ptrs[i])) + 15) = rand_num;
    }
    int dest;
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
      int ptr_num = rand() % NUM_PTRS;
      dest = *(((int*) (mem_ptrs[i % NUM_PTRS])) + 0);
      dest = *(((int*) (mem_ptrs[i % NUM_PTRS])) + 15);
    }
    for (int i = 0; i < NUM_PTRS; i++)
    {
      free(mem_ptrs[i]);
    }
    return 0;
}
