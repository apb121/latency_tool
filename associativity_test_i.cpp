#include <iostream>
#include <functional>
#include <cstdlib>

#define NUM_FUNCTIONS 32
//#define NUM_CALLS 1441440
#define NUM_CALLS 16800

using namespace std;

void function_0()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_1()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_2()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_3()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_4()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_5()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_6()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_7()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_8()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_9()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_10()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_11()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_12()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_13()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_14()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_15()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_16()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_17()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_18()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_19()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_20()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_21()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_22()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_23()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_24()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_25()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_26()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_27()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_28()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_29()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_30()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

void function_31()
{
    int i = 0;
    for (int j = 0; j < 10; j++)
    {
        i = j + 1;
    }
}

int main(int argc, char** argv)
{
    int num_functions = atoi(argv[1]);
    function<void()> f[32]
    {
        function_0,
        function_1,
        function_2,
        function_3,
        function_4,
        function_5,
        function_6,
        function_7,
        function_8,
        function_9,
        function_10,
        function_11,
        function_12,
        function_13,
        function_14,
        function_15,
        function_16,
        function_17,
        function_18,
        function_19,
        function_20,
        function_21,
        function_22,
        function_23,
        function_24,
        function_25,
        function_26,
        function_27,
        function_28,
        function_29,
        function_30,
        function_31
    };
    for (int i = 0; i < NUM_CALLS / num_functions; ++i)
    {
        for (int j = 0; j < num_functions; ++j)
        {
            f[j]();
        }
    }
    return 0;
}