#include <iostream>
#include <functional>
#include <cstdlib>

#define MAX_PTRS 32
#define INT_SIZE sizeof(int)
#define MEM_ALLOCATION 64
#define NUM_ACCESSES 100000

using namespace std;

int main(int argc, char** argv)
{
    int ptrs = atoi(argv[1]);
    int alignment = atoi(argv[2]);
    void* mem_ptrs[MAX_PTRS];
    for (int i = 0; i < ptrs; i++)
    {
        posix_memalign((void**) &mem_ptrs[i], alignment, MEM_ALLOCATION);
    }
    int rand_num = rand() % 128;
    for (int i = 0; i < ptrs; i++)
    {
      *(((int*) (mem_ptrs[i])) + 0) = rand_num;
      *(((int*) (mem_ptrs[i])) + 15) = rand_num;
    }
    int dest;
    for (int i = 0; i < NUM_ACCESSES; i++)
    {
      dest = *(((int*) (mem_ptrs[i % ptrs]) + 0));
      dest = *(((int*) (mem_ptrs[i % ptrs]) + 15));
    }
    for (int i = 0; i < ptrs; i++)
    {
      free(mem_ptrs[i]);
    }
    return 0;
}