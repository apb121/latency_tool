#include <iostream>
#include <unordered_set>
#include <queue>
#include <deque>
#include <bitset>

using namespace std;

static int i;

struct A
{
    char c;
    double d;
    short s;
    int i;
    unordered_set<void*> us;
};

struct B
{
    char c = {'a'};
    double d;
    short s;
    array < char[5], 5 > arr[5][5];
    int i;
    array < char[3], 7 > arr_2[4][2];
    vector < bool > b;
};

struct C
{
    char c = 'h';
    int func(char p)
    {
        return 4;
    }
    bool g = false;
    bool m = true;
    int** hello_world [6][8];
    char const * cc;
    char o;
    char const * const ccc = &c;
    double f{5.5};
};

struct D
{
    char e;
    long i;
    char c;
    char d[5];
    char f;
    void function ( int g ) const {
        int i = 0;
    }
};

struct E
{
    char c;
    double d;
    char c1;
    double d1;
    char c2;
    double d2;
};

struct BS
{
    std::bitset<8> b;
};

struct Inner
{
    char c;
    int i;
};

struct Outer
{
    Inner i;
    int g;
};

int main()
{
    std::cout << sizeof(Inner) << std::endl;
    std::cout << sizeof(Outer) << std::endl;
    return 0;
}