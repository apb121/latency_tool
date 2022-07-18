#include <iostream>

static int i;

struct A
{
    char c;
    double d;
    short s;
    int i;
};

int main()
{
    int i = 3;
    A a;
    a.d = 5.5;
    a.i = i;
    return 0;
}


class C
{
    char c = 'h';
    int func(char p)
    {
        return 4;
    }
    bool g = false;
    char *o;
    int hello_world = 5;
    char const * cc;
    char const * const ccc = &c;
    double f{5.5};
};

struct B
{
    char c = {'a'};
    short s;
    int i;
    double d;
};