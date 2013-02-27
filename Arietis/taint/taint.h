#pragma once
 
#ifndef __TAINT_TAINT_H__
#define __TAINT_TAINT_H__
 
#include "Arietis.h"
#include "memory.h"

// Per BYTE Taint structure
class Taint {
public:
    
    Taint();
    Taint(const Taint &t);
    //Taint(Taint &&t);
    Taint &operator=(Taint rhs);
    virtual ~Taint();

    int         GetWidth() const { return Width; }

    //int         GetIndices() const { return m_nIndices; }
    bool        IsTainted(int index) const { 
        Assert(index < Width); return m_data[index] != 0; 
    }
    void        Set(int index) { 
        Assert(index < Width); m_data[index] = 1; 
    }

    void        SetAll() {
        memset(m_data, 1, sizeof(m_data));
    }

    void        Reset(int index) { 
        Assert(index < Width); m_data[index] = 0; 
    }
    Taint       operator&(const Taint &rhs) const;
    Taint       operator|(const Taint &rhs) const;
    Taint       operator^(const Taint &rhs) const;
    Taint&      operator&=(const Taint &rhs);
    Taint&      operator|=(const Taint &rhs);
    Taint&      operator^=(const Taint &rhs);

    std::string ToString() const;
    static Taint    FromBinString(const std::string &s);
private:
    //int         m_nIndices;
    static const int    Width = 16;
    byte        m_data[Width];
};

// class TaintFactory {
// public:
//     Taint *     Create();
//     //Taint *     Create(int nIndices);
// private:
//     FactoryImpl<Taint> m_factory;
// };
 
// typedef Taint *     TaintPtr;
// ��Ӧ��ʹ��AllocOnlyPool, Taint��ʹ���ڴ�ᶯ̬����

#endif // __TAINT_TAINT_H__