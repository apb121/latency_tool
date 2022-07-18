#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>

using namespace std;

struct A
{
    double d;
    char c[7];
    short s;
    char cc;
};

struct B
{
    double d;
    char c[7];
    char cc;
    short s;
};

struct C
{
    vector<bool> v;
};

struct D
{
    vector<double> v;
};

struct E
{
    vector<double> v{5.5, 5.5, 5.5, 5.5, 5.5, 5.5, 5.5};
};

struct F
{
    vector<int> v;
};

struct G
{
    vector<vector<short>> v;
};

struct H
{
    vector<void*> v;
};

struct MAP
{
    map<void*, void*> v;
};

struct UNORDERED_MAP
{
    unordered_map<void*, void*> v;
};

struct SET
{
    set<void*> v;
};

struct UNORDERED_SET
{
    unordered_set<void*> v;
};

struct MULTIMAP
{
    multimap<void*, void*> v;
};

struct UNORDERED_MULTIMAP
{
    unordered_multimap<void*, void*> v;
};

struct MULTISET
{
    multiset<void*> v;
};

struct UNORDERED_MULTISET
{
    unordered_multiset<void*> v;
};

struct STRING
{
    string v = "hello";
};

struct LIST
{
    list<int*> v;
};

struct ARRAY5
{
    array<char*, 5> v;
};

struct ARRAY50
{
    array<char*, 50> v;
};

int main()
{
    using namespace std;
    cout << sizeof(A) << endl;
    cout << sizeof(B) << endl;
    cout << sizeof(C) << endl;
    cout << sizeof(D) << endl;
    cout << sizeof(E) << endl;
    cout << sizeof(F) << endl;
    cout << sizeof(G) << endl;
    cout << sizeof(H) << endl;
    cout << sizeof(MAP) << endl; //48
    cout << sizeof(UNORDERED_MAP) << endl; //56
    cout << sizeof(SET) << endl; //48
    cout << sizeof(UNORDERED_SET) << endl; //56 ...
    cout << sizeof(MULTIMAP) << endl;
    cout << sizeof(UNORDERED_MULTIMAP) << endl;
    cout << sizeof(MULTISET) << endl;
    cout << sizeof(UNORDERED_MULTISET) << endl;
    cout << sizeof(STRING) << endl; //32
    cout << sizeof(LIST) << endl; //32
    cout << sizeof(ARRAY5) << endl;
    cout << sizeof(ARRAY50) << endl;
    
    return 0;
}