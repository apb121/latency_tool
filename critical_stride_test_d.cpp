#include <iostream>
#include <functional>
#include <cstdlib>

#define NUM_PTRS 256
#define INT_SIZE sizeof(int)
#define MEM_ALLOCATION 64
#define NUM_ACCESSES 10000

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
      for (int j = 0; j < MEM_ALLOCATION / INT_SIZE; j++)
      {
        *(((int*) (mem_ptrs[i])) + j) = (rand() % 100);
      }
    }
    int dest;
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
      int ptr_num = rand() % NUM_PTRS;
      for (int j = 0; j < MEM_ALLOCATION / INT_SIZE; j++)
      {
          dest = *(((int*) (mem_ptrs[ptr_num]) + j));
      }
    }
    for (int i = 0; i < NUM_PTRS; i++)
    {
      free(mem_ptrs[i]);
    }
    return 0;
}