#pragma once

#ifndef __ARIETIS_STATIC_DISASSEMBLER_H__
#define __ARIETIS_STATIC_DISASSEMBLER_H__

#include "Arietis.h"
#include "instruction.h"
#include "memory.h"
#include "parallel.h"

struct Inst : public Instruction {
    static const int ApiInfoSize = 64;
    u32     Eip;
    u32     Target;
    u32     Entry;
    char    DllName[ApiInfoSize];
    char    FuncName[ApiInfoSize];
    int     Index;

    Inst() : Eip(0), Target(-1), Entry(-1), Index(-1) {
        ZeroMemory(DllName, sizeof(DllName));
        ZeroMemory(FuncName, sizeof(DllName));
    }
};

typedef Inst *  InstPtr;

typedef AllocOnlyPool<Inst>  InstPool;

class InstSection {
public:
    InstSection(InstPool &pool, Mutex &mutex, u32 base, u32 size);
    ~InstSection();

    u32             GetBase() const { return m_base; }
    u32             GetSize() const { return m_size; }
    int             GetCount() const { return m_count; }
    InstPtr         GetInst(u32 addr) const 
    {
        AssertInRanage(addr);
        Assert(m_data[addr - m_base]);
        return m_data[addr - m_base];
    }
    bool            Contains(u32 addr) const 
    {
        AssertInRanage(addr);
        return m_data[addr - m_base] != NULL;
    }
    bool            IsInRange(u32 addr) const
    {
        return m_base <= addr && addr < m_base + m_size; 
    }

    // InstSection is not responsible for cleaning pinst
    InstPtr         Alloc(u32 addr);
    void            Lock() const;
    void            Release() const;

    InstPtr *       Begin() const;
    InstPtr *       Next(InstPtr *curr) const;
    InstPtr *       End() const { return m_data + m_size; }

    void            UpdateIndices() const;
    u32             GetEipFromIndex(int idx) const { return m_indices[idx]; }

private:
    InstSection(const InstSection &);
    InstSection &operator=(const InstSection &);
    void            AssertInRanage(u32 addr) const { Assert(IsInRange(addr)); }
private:
    u32             m_base;
    u32             m_size;
    InstPtr *       m_data;
    u32 *           m_indices;
    int             m_count;
    InstPool &      m_pool;
    Mutex &         m_mutex;
};

class InstMem {
public:
    InstMem();
    ~InstMem();

    InstSection *   GetSection(u32 addr) const { return m_pagetable[PAGE_NUM(addr)]; }
    InstSection *   CreateSection(u32 base, u32 size);

    InstPtr         GetInst(u32 addr) const;
    void            Lock() const { m_mutex.Wait(); }
    void            Release() const { m_mutex.Release(); }
private:
    InstSection *   AddSection(u32 base, u32 size);

private:
    InstSection *   m_pagetable[LX_PAGE_COUNT];
    InstPool        m_pool;
    Mutex           m_mutex;
};

class Disassembler {
public:
    
//     struct Inst {
//         InstPtr     ptr;
//         u32         eip;
//         u32         target;
//         u32         entry;  // proc entry
//         std::string dllName;
//         std::string funcName;
// 
//         Inst() : ptr(NULL), eip(0), target(-1), entry(-1) {}
//         Inst(InstPtr p, u32 e) : ptr(p), eip(e), target(-1), entry(-1) {}
//     };

    //typedef std::vector<Inst>       InstVector;
    //typedef std::map<u32, Inst>     InstDisasmMap;
    
    //typedef std::function<void (const InstVector &vec)>     InstDisasmHandler;
    
    typedef std::function<void (InstSection *insts)>    DataUpdateHandler;
    
public:
    Disassembler();
    virtual ~Disassembler();

    //void        RegisterInstDisasmHandler(InstDisasmHandler h) { m_instDisasmHandler = h; }

    void        RegisterDataUpdateHandler(DataUpdateHandler h) {
        m_dataUpdateHandler = h;
    }

    void        OnPreExecute(const Processor *cpu, const Instruction *inst);
    void        UpdateInstContext(InstContext &ctx) const;
    InstPtr     GetInst(u32 eip) const { return m_instMem.GetInst(eip); }
private:
    void        RecursiveDisassemble(int depth, const Processor *cpu, u32 eip, InstSection *sec, u32 entryEip);
    void        AttachApiInfo(const Processor *cpu, u32 eip, InstSection *sec, InstPtr inst);
private:
    static const int    MaxRecursiveDepth = 3;
    int                 m_currDepth;
    const Processor *   m_currProcessor;
    DataUpdateHandler   m_dataUpdateHandler;
    const Section *     m_lastSec;
    //InstDisasmMap       m_instMap;
    //std::unordered_map<const Section *, InstDisasmMap> m_secMap;

    InstMem             m_instMem;
};

#endif // __ARIETIS_STATIC_DISASSEMBLER_H__
