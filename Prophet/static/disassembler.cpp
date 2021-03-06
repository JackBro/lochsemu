#include "stdafx.h"
#include "disassembler.h"
#include "engine.h"
#include "event.h"

#include "processor.h"
#include "instruction.h"
#include "memory.h"
#include "process.h"
#include "emulator.h"

#include "protocol/runtrace.h"

InstSection::InstSection( InstMem *mem, InstPool &pool, u32 base, u32 size )
    : m_mem(mem), m_pool(pool), m_base(base), m_size(size), m_count(0)
{
    m_data      = new InstPtr[m_size];
    m_indices   = new u32[m_size];
    ZeroMemory(m_data, sizeof(InstPtr) * m_size);
    for (u32 i = 0; i < m_size; i++)
        m_indices[i] = -1;
}

InstSection::~InstSection()
{
    SAFE_DELETE_ARRAY(m_data);
    SAFE_DELETE_ARRAY(m_indices);
}

InstPtr InstSection::Alloc( u32 addr )
{
    SyncObjectLock lock(*this);

    AssertInRanage(addr);
    Assert(m_data[addr - m_base] == NULL);
    InstPtr pinst           = m_pool.Alloc();
    pinst->Eip              = addr;
    m_data[addr - m_base]   = pinst;
    m_count++;
    return pinst;
}

InstPtr * InstSection::Next( InstPtr *curr ) const
{
    InstPtr *p = curr;
    while (true) {
        p++;
        if (*p != NULL || p == End()) return p;
    }
}

InstPtr * InstSection::Begin() const
{
    InstPtr *p = m_data;
    while (*p == NULL && p != End()) 
        p++;
    return p;
}

void InstSection::UpdateIndices() const
{
    int idx = 0;
    for (u32 i = 0; i < m_size; i++)
        m_indices[i] = -1;
    for (u32 i = 0; i < m_size; i++) {
        if (m_data[i] != NULL) {
            m_data[i]->Index = idx;
            m_indices[idx++] = m_data[i]->Eip;
        }
    }
}

void InstSection::Lock() const
{
    m_mem->Lock();
}

void InstSection::Unlock() const
{
    m_mem->Unlock();
}

InstMem::InstMem() : m_pool(16384) //, m_mutex(false)
{
    ZeroMemory(m_pagetable, sizeof(m_pagetable));
}

InstMem::~InstMem()
{
    int ptr = 0;
    while (ptr < LX_PAGE_COUNT) {
        if (m_pagetable[ptr] == NULL) {
            ptr++;
            continue;
        }
        int s = m_pagetable[ptr]->GetSize() / LX_PAGE_SIZE;
        Assert(s > 0);
        SAFE_DELETE(m_pagetable[ptr]);
        ptr += s;
    }
}

InstSection * InstMem::CreateSection( u32 base, u32 size )
{
    InstSection *sec = GetSection(base);
    if (sec) {
        Assert(sec->GetBase() == base && sec->GetSize() == size);
        return sec;
    }
    return AddSection(base, size);
}

InstSection * InstMem::AddSection( u32 base, u32 size )
{
    InstSection *sec = new InstSection(this, m_pool, base, size);
    for (uint i = PAGE_NUM(base); i < PAGE_NUM(base + size); i++) {
        Assert(m_pagetable[i] == NULL);
        m_pagetable[i] = sec;
    }
    return sec;
}

InstPtr InstMem::GetInst( u32 addr ) const
{
    InstSection *sec = GetSection(addr);
    if (sec == NULL) return NULL;
    Assert(sec);
    return sec->GetInst(addr);
}

Disassembler::Disassembler(ProEngine *engine)
    : m_engine(engine)
{
}

Disassembler::~Disassembler()
{

}

void Disassembler::Initialize()
{
    m_debugger = m_engine->GetDebugger();
}

void Disassembler::OnPreExecute( PreExecuteEvent &event )
{
    Disassemble(event.Cpu, event.Cpu->EIP);
}

InstPtr Disassembler::Disassemble( const Processor *cpu, u32 eip )
{
    Section *sec = LxEmulator.Mem()->GetSection(eip);

    InstSection *instSec = m_instMem.CreateSection(sec->Base(), sec->Size());


    if (!instSec->Contains(eip)) {
        SyncObjectLock lock(m_instMem);
        std::set<InstSection *> updateSections;
        RecursiveDisassemble(cpu, eip, instSec, eip, updateSections);
        //instSec->UpdateIndices();
        for (auto &s : updateSections) {
            s->UpdateIndices();
        }
    }

    InstPtr pinst = instSec->GetInst(eip);
    Assert(pinst->Index != -1);
    return pinst;
}

void Disassembler::RecursiveDisassemble( const Processor *cpu, u32 eip, 
                                        InstSection *sec, u32 entryEip,
                                        std::set<InstSection *> &sections)
{
    Assert(sec);
    Assert(entryEip != 0);

    bool updateIndex = false;
    while (true) {
        Assert(sec->IsInRange(eip));
        if (sec->Contains(eip)) break;     // already disassembled

        InstPtr inst = sec->Alloc(eip);
        LxDecode(LxEmulator.Mem()->GetRawData(eip), (Instruction *) inst, eip);
        AttachApiInfo(cpu, eip, sec, inst, sections);
        updateIndex = true;

        u32 opcode = inst->Main.Inst.Opcode;
        if (opcode == 0xc3 || opcode == 0xcb || opcode == 0xc2 || opcode == 0xca) {
            // 'ret' is met
            inst->Entry = entryEip;
            break;
        }

        if (opcode == 0xcc || opcode == 0xcd) break;

        u32 addrValue = (u32) inst->Main.Inst.AddrValue;
        if (addrValue != 0) {
            if (IsConstantArg(inst->Main.Argument1)) {
                Section *s = cpu->Mem->GetSection(addrValue);
                if (s != NULL && s->Description() != "heap") {
                    u32 nextEntry = Instruction::IsCall(inst) ? addrValue : entryEip;
                    InstSection *jumpSec = m_instMem.CreateSection(s->Base(), s->Size());
                    RecursiveDisassemble(cpu, addrValue, jumpSec, nextEntry, sections);
                    //jumpSec->UpdateIndices();
                }
            }
        }
        u32 nextEip = eip + inst->Length;
        InstSection *nextSec = m_instMem.GetSection(nextEip);
        if (sec != nextSec) break;
        eip = nextEip;
    }

    if (updateIndex) {
        sections.insert(sec);
    }
}

void Disassembler::AttachApiInfo( const Processor *cpu, u32 eip, InstSection *sec, 
                                 InstPtr inst, std::set<InstSection *> &sections )
{
    u32 target = 0;
    u32 opcode = inst->Main.Inst.Opcode;
    const char *mnemonic = inst->Main.Inst.Mnemonic;

    if (opcode == 0xff) {
        // CALL or JMP r/m32
        if (strstr(mnemonic, "jmp") == mnemonic /*|| Instruction::IsCall(inst)*/) {
            if (IsMemoryArg(inst->Main.Argument1) &&
                inst->Main.Argument1.Memory.BaseRegister == 0 &&
                inst->Main.Argument1.Memory.IndexRegister == 0 &&
                inst->Main.Prefix.FSPrefix == 0) 
            {
                if (cpu->Mem->Contains(inst->Main.Argument1.Memory.Displacement))
                    target = cpu->ReadOperand32(inst, inst->Main.Argument1, NULL);
            }
        }
    } else if (opcode == 0xe8) {
        target = (u32) inst->Main.Inst.AddrValue;
        
        Section *sect = cpu->Mem->GetSection(target);
        if (sect && sect->Description() != "heap") {
            InstSection *callSec = m_instMem.CreateSection(sect->Base(), sect->Size());
            RecursiveDisassemble(cpu, target, callSec, target, sections);

            InstPtr instCalled = callSec->GetInst(target);
            Assert(instCalled != NULL);

            if (instCalled->Main.Inst.Opcode == 0xff &&
                (strstr(instCalled->Main.Inst.Mnemonic, "jmp") == instCalled->Main.Inst.Mnemonic)) 
            {
                target = cpu->ReadOperand32(instCalled, instCalled->Main.Argument1, NULL);
            }
        }
    } else if (*mnemonic == 'j') {
        target = (u32) inst->Main.Inst.AddrValue;
    }

    if (target) {
        inst->Target = target;
//         ModuleInfo *info = cpu->Proc()->GetModuleInfoAddr(target);
//         if (info != NULL)
//             strncpy(inst->TargetModuleName, info->Name, sizeof(inst->TargetModuleName));
    }

    const ApiInfo *info = LxEmulator.Proc()->GetApiInfoFromAddress(target);
    if (info) {
        //Assert(strncmp(inst->TargetModuleName, info->ModuleName.c_str(), sizeof(inst->TargetModuleName)) == 0);
        strncpy(inst->TargetModuleName, info->ModuleName.c_str(), sizeof(inst->TargetModuleName));
        strncpy(inst->TargetFuncName, info->FunctionName.c_str(), sizeof(inst->TargetFuncName));
    }
}

void Disassembler::UpdateInstContext( InstContext *ctx, u32 eip ) const
{
    ctx->Inst = m_instMem.GetInst(eip);
}

void Disassembler::UpdateTContext( TContext *ctx, u32 eip ) const
{
    ctx->Inst = m_instMem.GetInst(eip);
}

InstPtr Disassembler::GetInst( u32 eip )
{
    InstPtr pinst = m_instMem.GetInst(eip);

    const Processor *cpu = m_debugger->GetCurrentThread()->CPU();
    if (cpu == NULL) {
        LxFatal("Current processor is NULL\n");
    }

    if (NULL == pinst)
        pinst = Disassemble(cpu, eip);
    return pinst;
}

InstPtr Disassembler::GetInst( const Processor *cpu, u32 eip )
{
    InstPtr pinst = m_instMem.GetInst(eip);
    if (NULL == pinst)
        pinst = Disassemble(cpu, eip);
    return pinst;
}

const InstSection * Disassembler::GetInstSection( u32 addr )
{
    if (m_instMem.GetInst(addr) == NULL) {
        const Processor *cpu = m_debugger->GetCurrentThread()->CPU();
        if (cpu == NULL) {
            LxFatal("Current processor is NULL\n");
        }
        Disassemble(cpu, addr);
    }
    //InstSection *sec = m_instMem.GetSection(addr);
    //sec->UpdateIndices();
    return m_instMem.GetSection(addr);
}

