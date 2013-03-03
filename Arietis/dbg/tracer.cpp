#include "stdafx.h"
#include "tracer.h"
#include "instruction.h"
#include "processor.h"
#include "engine.h"

ATracer::ATracer( AEngine *engine )
    : m_engine(engine),  m_seq(-1)
{
}

ATracer::~ATracer()
{
}

void ATracer::OnPreExecute( const Processor *cpu )
{
    m_currEip   = cpu->EIP;
    if (!IsEnabled()) return;
}

void ATracer::OnPostExecute( const Processor *cpu, const Instruction *inst )
{
    ++m_seq;

    if (!IsEnabled()) return;

    TraceContext ctx;
    m_engine->GetTraceContext(&ctx, m_currEip);
    ctx.seq     = m_seq;

    {
        SyncObjectLock lock(*this);
        m_traces.push_back(ctx);
    }
}

void ATracer::Serialize( Json::Value &root ) const 
{
    root["enabled"] = m_enabled;
}

void ATracer::Deserialize( Json::Value &root )
{
    m_enabled = root.get("enabled", m_enabled).asBool();
}
