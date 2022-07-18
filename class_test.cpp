#include <iostream>
#include <unordered_set>
#include <queue>
#include <deque>

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

int main()
{
    int i = 3;
    A a;
    B b;
    C c;
    a.d = 5.5;
    deque<char> dq;
    queue<int> qu;
    a.i = i;
    cout << "=============" << endl;
    cout << "A" << endl;
    cout << sizeof(a) << endl;
    // cout << sizeof(a.c) << endl;
    // cout << sizeof(a.d) << endl;
    // cout << sizeof(a.s) << endl;
    // cout << sizeof(a.i) << endl;
    // cout << sizeof(a.us) << endl;
    cout << endl;
    cout << "=============" << endl;
    cout << "B" << endl;
    // cout << sizeof(b.s) << endl;
    // cout << sizeof(b.d) << endl;
    // cout << sizeof(b.c) << endl;
    // cout << sizeof(b.arr) << endl;
    // cout << sizeof(b.i) << endl;
    // cout << sizeof(b.b) << endl;
    // cout << sizeof(b.arr_2) << endl;
    // B* base = &b;
    // cout << (size_t)&(b.s) - (size_t) base << endl;
    // cout << (size_t)&(b.d) - (size_t) base << endl;
    // cout << (size_t)&(b.c) - (size_t)base << endl;
    // cout << (size_t)&(b.arr) - (size_t)base << endl;
    // cout << (size_t)&(b.i) - (size_t)base << endl;
    // cout << (size_t)&(b.b) - (size_t)base << endl;
    // cout << (size_t)&(b.arr_2) - (size_t)base << endl;
    cout << "Total: " << sizeof(b) << endl;
    cout << endl;
    cout << "=============" << endl;
    cout << "C" << endl;
    cout << sizeof(c) << endl;
    // cout << sizeof(c.c) << endl;
    // cout << sizeof(c.g) << endl;
    // cout << sizeof(c.m) << endl;
    // cout << sizeof(c.o) << endl;
    // cout << sizeof(c.hello_world) << endl;
    // cout << sizeof(c.cc) << endl;
    // cout << sizeof(c.ccc) << endl;
    cout << sizeof(c.f) << endl;
    cout << endl;
    cout << "=============" << endl;
    cout << "D" << endl;
    cout << sizeof(D) << endl;
    cout << sizeof(long) << endl;
    cout << endl;
    return 0;
}