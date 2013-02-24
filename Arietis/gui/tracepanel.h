#pragma once

#ifndef __ARIETIS_GUI_TRACEPANEL_H__
#define __ARIETIS_GUI_TRACEPANEL_H__

#include "Arietis.h"
#include "gui.h"
#include "tracer.h"
#include "custombase.h"

class TraceInfoPanel;
class TracePanel;


class CompositeTracePanel : public wxPanel {
    friend class TraceInfoPanel;
    friend class TracePanel;
public:
    CompositeTracePanel(wxWindow *parent);
    ~CompositeTracePanel();

    const wxFont *GetFont() const { return &m_font; }
    void        SetTracer(const ATracer *t) { m_tracer = t; }
    void        UpdateData();
    void        OnSelectionChange(int index);
private:
    void        Init();
private:
    const ATracer * m_tracer;
    TracePanel *    m_tracePanel;
    TraceInfoPanel *m_infoPanel;
    int             m_total;
};

class TraceInfoPanel : public wxPanel {
    friend class TracePanel;
public:
    TraceInfoPanel(CompositeTracePanel *parent);
    ~TraceInfoPanel() {}

    void        OnPaint(wxPaintEvent &event);
    void        OnEraseBackground(wxEraseEvent &event);
    void        UpdateData(const ATracer::Trace &t);
    void        UpdateData();
private:
    void        Init();
private:
    CompositeTracePanel*    m_parent;
    ATracer::Trace  m_trace;
    wxFont          m_font;
    wxBrush         m_bgBrush;        
    int             m_lineHeight;
};

class TracePanel : public SelectableScrolledControl {
    friend class TraceInfoPanel;
public:
    TracePanel(CompositeTracePanel *parent);
    ~TracePanel();

//     void        OnLeftDown(wxMouseEvent &event);
//     void        OnLeftUp(wxMouseEvent &event);
//     void        OnMouseMove(wxMouseEvent &event);
//     void        OnMouseLeave(wxMouseEvent &event);

    void        UpdateData();
    void        OnSelectionChange() override;
    const static int VertLineOffset = -2;
private:
    void        InitRender();
    void        Draw(wxBufferedPaintDC &dc) override;
    void        DrawTrace(wxBufferedPaintDC &dc, const ATracer::Trace &trace, int index);
private:
    CompositeTracePanel *   m_parent;

    wxPen       m_currPen;
    wxPen       m_currSelPen;
    int         m_widthIp;
    int         m_widthDisasm;
    int         m_width;
    //int         m_currIndex;
    //bool        m_isLeftDown;
};

#endif // __ARIETIS_GUI_TRACEPANEL_H__