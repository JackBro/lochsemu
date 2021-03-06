#pragma once

#ifndef __PROPHET_GUI_TRACEPANEL_H__
#define __PROPHET_GUI_TRACEPANEL_H__

#include "prophet.h"
#include "gui.h"
#include "dbg/tracer.h"
#include "custombase.h"

class TraceInfoPanel;
class TracePanel;


class CompositeTracePanel : public wxPanel {
    friend class TraceInfoPanel;
    friend class TracePanel;
public:
    CompositeTracePanel(wxWindow *parent, ProphetFrame *dad, ContextPanel *ctxPanel, CpuPanel *cpuPanel);
    ~CompositeTracePanel();

    const wxFont *GetFont() const { return &m_font; }
    void        SetTracer(const ProTracer *t) { m_tracer = t; }
    void        UpdateData();
    void        OnSelectionChange(int index);
    
private:
    void        Init();
private:
    ProphetFrame *  m_dad;
    const ProTracer * m_tracer;
    TracePanel *    m_tracePanel;
    TraceInfoPanel *m_infoPanel;
    ContextPanel *  m_contextPanel;
    CpuPanel *      m_cpuPanel;
    int             m_total;
};

class TraceInfoPanel : public wxPanel {
    friend class TracePanel;
public:
    TraceInfoPanel(CompositeTracePanel *parent);
    ~TraceInfoPanel() {}

    void        OnPaint(wxPaintEvent &event);
    void        OnEraseBackground(wxEraseEvent &event);
    void        UpdateData(const TraceContext &t);
    void        UpdateData();
private:
    void        Init();
private:
    CompositeTracePanel*    m_parent;
    TraceContext    m_trace;
    wxFont          m_font;
    wxBrush         m_bgBrush;        
    int             m_lineHeight;
};

class TracePanel : public SelectableScrolledControl {
    friend class TraceInfoPanel;
public:
    TracePanel(CompositeTracePanel *parent);
    ~TracePanel();

    void        UpdateData();
    void        OnSelectionChange() override;
    void        OnLeftDoubleClick(wxMouseEvent &event);
    void        OnRightDown(wxMouseEvent &event);

    void        OnPopupFindFirstReg(wxCommandEvent &event);
    void        OnPopupFindMrAddr(wxCommandEvent &event);
    void        OnPopupFindMwAddr(wxCommandEvent &event);
private:
    void        InitRender();
    void        InitMenu();
    void        Draw(wxBufferedPaintDC &dc) override;
    void        DrawTrace(wxBufferedPaintDC &dc, const TraceContext &trace, int index);
    void        SelectIndex(int index);
private:
    CompositeTracePanel *   m_parent;

    wxPen       m_currPen;
    wxPen       m_currSelPen;
    int         m_widthIp;
    int         m_widthDisasm;
    int         m_widthMem;
    //int         m_widthTaint;
    int         m_width;
    wxMenu *        m_popup;
};

#endif // __PROPHET_GUI_TRACEPANEL_H__
