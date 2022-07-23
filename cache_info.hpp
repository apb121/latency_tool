#ifndef CACHE_INFO_HPP
#define CACHE_INFO_HPP

#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>

class L1d_cache;
class L1i_cache;
class L2_cache;
class L3_cache;
class L4_cache;

class Cache
{
  std::string name = "generic_cache";
  int size = -1;
  int linesize = -1;
  int assoc = -1;
  int critical_stride = -1;
  public:
  Cache(std::string name, int size, int linesize, int assoc);
  virtual ~Cache() = default;
  const int get_linesize() const { return linesize; }
  const int get_assoc() const { return assoc; }
  const int get_size() const { return size; }
  const int get_critical_stride() const { return critical_stride; }
  void set_linesize(int l) { linesize = l; }
  void set_assoc(int a) { assoc = a; }
  void set_critical_stride(int cs) { critical_stride = cs; }
};

class L1_cache : public Cache
{
    public:
    L1_cache(std::string name, int size, int linesize, int assoc)
        : Cache(name, size, linesize, assoc) {}
    virtual int empirical_assoc_test() = 0; // these are interesting from the perspective of the class parser!
    virtual int empirical_stride_test() = 0;
};

class L1d_cache : public L1_cache
{
    public:
    L1d_cache(int size, int linesize, int assoc)
        : L1_cache("L1d", size, linesize, assoc) {}
    int empirical_assoc_test() override;
    int empirical_stride_test() override;
};

class L1i_cache : public L1_cache
{
    public:
    L1i_cache(int size, int linesize, int assoc)
        : L1_cache("L1i", size, linesize, assoc)
    {

    }
    int empirical_assoc_test() override;
    int empirical_stride_test() override;
    
};

class L2_cache : public Cache
{
    public:
    L2_cache(int size, int linesize, int assoc)
        : Cache("L2", size, linesize, assoc) {}
};

class L3_cache : public Cache
{
    public:
    L3_cache(int size, int linesize, int assoc)
        : Cache("L3", size, linesize, assoc) {}
};

class L4_cache : public Cache
{
    public:
    L4_cache(int size, int linesize, int assoc)
        : Cache("L4", size, linesize, assoc) {}
};

struct Processor
{
    L1d_cache* l1d = nullptr;
    L1i_cache* l1i = nullptr;
    L2_cache* l2 = nullptr;
    L3_cache* l3 = nullptr;
    L4_cache* l4 = nullptr;
    ~Processor();
};

#endif