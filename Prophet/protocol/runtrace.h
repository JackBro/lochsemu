#pragma once
 
#ifndef __PROPHET_TAINT_TTRACE_H__
#define __PROPHET_TAINT_TTRACE_H__
 
#include "prophet.h"
#include "parallel.h"
#include "utilities.h"
#include "tcontext.h"

class RunTrace : public MutexSyncObject, public ISerializable {
public:
    RunTrace(Protocol *engine);
    ~RunTrace();

    void        Begin();
    void        Trace(const Processor *cpu);
    void        End();
    int         Count() const { return m_count; }
    TContext *  Get(int n) { Assert(n >= 0 && n < m_count); return m_traces + n; }
    const TContext * Get(int n) const { Assert(n >= 0 && n < m_count); return m_traces + n; }

    void        Serialize(Json::Value &root) const override;
    void        Deserialize(Json::Value &root) override;
    
    void        Dump(File &f) const;
    void        DumpMsg(Message *msg, File &f) const;

private:
    int         m_count;
    int         m_maxTraces;
    Protocol *  m_engine;
    TContext *  m_traces;
    bool        m_mergeCallJmp;
};
 
#endif // __PROPHET_TAINT_TTRACE_H__