#include "stdafx.h"
#include "tracepanel.h"


TracePanel::TracePanel( CompositeTracePanel *parent )
    : SelectableScrolledControl(parent, wxSize(600, 200)), m_parent(parent)
{
    InitRender();

//     Bind(wxEVT_LEFT_DOWN,   &TracePanel::OnLeftDown,    this, wxID_ANY);
//     Bind(wxEVT_LEFT_UP,     &TracePanel::OnLeftUp,      this, wxID_ANY);
//     Bind(wxEVT_MOTION,      &TracePanel::OnMouseMove,   this, wxID_ANY);
//     Bind(wxEVT_LEAVE_WINDOW,&TracePanel::OnMouseLeave,  this, wxID_ANY);
//     m_currSelIndex     = -1;
//     m_isLeftDown    = false;
}

TracePanel::~TracePanel()
{

}

void TracePanel::InitRender()
{
    wxFont f =    wxFont(g_config.GetInt("TracePanel", "FontSize", 8), 
        wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, 
        g_config.GetString("TracePanel", "FontName", "Lucida Console"));
    UpdateFont(f);
    m_currPen       = wxPen(wxColour(g_config.GetString("TracePanel", "CurrentColor", "#0080ff")));
    m_currSelPen    = wxPen(wxColour(g_config.GetString("TracePanel", "CurrentSelectionColor", "#808080")));
    m_widthIp       = g_config.GetInt("TracePanel", "WidthIp", 70);
    m_widthDisasm   = g_config.GetInt("TracePanel", "WidthDisasm", 300);
    m_width         = m_widthIp + m_widthDisasm;
}

void TracePanel::Draw( wxBufferedPaintDC &dc )
{
    if (m_parent->m_tracer == NULL) return;
    m_parent->m_tracer->Lock();

    const ATracer::TraceVec &vec = m_parent->m_tracer->GetData();
    const int N = vec.size();
    SetVirtualSize(m_width, m_lineHeight * N);
    m_parent->m_total = N;

    wxPoint viewStart   = GetViewStart();
    wxSize clientSize   = GetClientSize();
    const int istart    = viewStart.y;
    const int iend      = istart + clientSize.GetHeight() / m_lineHeight;

    // draw traces
    dc.SetBrush(m_bgBrush);
    for (int i = istart; i <= min(N-1, iend); i++) {
        DrawTrace(dc, vec[i], i);
    }

    // draw vertical lines
    dc.SetPen(*wxGREY_PEN);
    int px, py;
    GetScrollPixelsPerUnit(&px, &py);

    int lineX = m_widthIp + VertLineOffset;
    int lineY0 = viewStart.y * py, lineY1 = lineY0 + clientSize.GetHeight();
    dc.DrawLine(lineX, lineY0, lineX, lineY1);
    lineX += m_widthDisasm;
    dc.DrawLine(lineX, lineY0, lineX, lineY1);

    m_parent->m_tracer->Unlock();
}

void TracePanel::DrawTrace( wxBufferedPaintDC &dc, const ATracer::Trace &trace, int index )
{
    int h = m_lineHeight * index; // plus 1 for header
    wxRect rectToDraw(0, h, m_width, m_lineHeight);

    if (index == m_parent->m_total - 1) {
        dc.SetPen(m_currPen);
        dc.DrawRectangle(rectToDraw);
    } else if (index == m_currSelIndex) {
        dc.SetPen(m_currSelPen);
        dc.DrawRectangle(rectToDraw);
    }
    dc.SetPen(*wxBLACK_PEN);

    int w = 0;
    dc.DrawText(wxString::Format("%08X", trace.inst->Eip), w, h);
    w += m_widthIp;
    dc.DrawText(trace.inst->Main.CompleteInstr, w, h);
}

void TracePanel::UpdateData()
{
    Refresh();
    Update();
    Scroll(0, m_parent->m_total);
}

void TracePanel::OnSelectionChange()
{
    m_parent->OnSelectionChange(m_currSelIndex);
}

// 
// void TracePanel::OnLeftDown( wxMouseEvent &event )
// {
//     wxPoint p       = event.GetPosition();
//     wxPoint ps      = GetViewStart();
//     m_currIndex     = ps.y + p.y / m_lineHeight;
//     m_isLeftDown    = true;
// 
//     Refresh();
//     m_parent->OnSelectionChange(m_currIndex);
// }
// 
// void TracePanel::OnLeftUp( wxMouseEvent &event )
// {
//     m_isLeftDown    = false;
// }
// 
// void TracePanel::OnMouseMove( wxMouseEvent &event )
// {
//     if (!m_isLeftDown) return;
//     wxPoint p = event.GetPosition();
//     wxPoint ps = GetViewStart();
//     int index = ps.y + p.y / m_lineHeight;
//     if (index != m_currIndex) {
//         m_currIndex = index;
//         Refresh();
//         m_parent->OnSelectionChange(m_currIndex);
//     }
// }
// 
// void TracePanel::OnMouseLeave( wxMouseEvent &event )
// {
//     m_isLeftDown    = false;
// }


TraceInfoPanel::TraceInfoPanel( CompositeTracePanel *parent )
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 35)), m_parent(parent)
{
    Init();

    Bind(wxEVT_PAINT, &TraceInfoPanel::OnPaint, this, wxID_ANY);
    Bind(wxEVT_ERASE_BACKGROUND, &TraceInfoPanel::OnEraseBackground, this, wxID_ANY);
}

void TraceInfoPanel::Init()
{
    m_font =    wxFont(g_config.GetInt("TraceInfoPanel", "FontSize", 8), 
        wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, 
        g_config.GetString("TracePanel", "FontName", "Lucida Console"));

    m_bgBrush   = wxBrush(wxColour(g_config.GetString("TraceInfoPanel", "BgColor", "#b0b0b0")));

    SetFont(m_font);

    wxClientDC dc(this);
    dc.SetFont(m_font);
    m_lineHeight = dc.GetFontMetrics().height;
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void TraceInfoPanel::OnPaint( wxPaintEvent &event )
{
    static const wxString Header[] = { "Ip", "Disassembly" };

    wxBufferedPaintDC dc(this);
    dc.SetBackground(m_bgBrush);
    dc.Clear();

    int h = 0, w = 0;

    // draw header
    const int v = m_parent->m_tracePanel->VertLineOffset;
    int u = w + m_parent->m_tracePanel->m_widthIp + v;
    dc.DrawText(Header[0], w, h);
    dc.DrawLine(u, 0, u, m_lineHeight);
    w += m_parent->m_tracePanel->m_widthIp;
    u += m_parent->m_tracePanel->m_widthDisasm;
    dc.DrawText(Header[1], w, h);
    dc.DrawLine(u, 0, u, m_lineHeight);
    h += m_lineHeight;
    dc.DrawLine(v, h, v + m_parent->m_tracePanel->m_width, h);

    h += 1;
    dc.DrawText(wxString::Format("Total: %d", m_parent->m_total), 0, h);
    h += m_lineHeight;
    dc.DrawText(wxString::Format("Seq: %I64d", m_trace.seq), 0, h);
}

void TraceInfoPanel::OnEraseBackground( wxEraseEvent &event )
{

}

void TraceInfoPanel::UpdateData(const ATracer::Trace &t)
{
    m_trace = t;
    UpdateData();
}

void TraceInfoPanel::UpdateData()
{
    Refresh();
    Update();
}


CompositeTracePanel::CompositeTracePanel( wxWindow *parent )
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(600, 600))
{
    Init();
    m_tracer = NULL;
}

CompositeTracePanel::~CompositeTracePanel()
{

}

void CompositeTracePanel::Init()
{
    m_tracePanel    = new TracePanel(this);
    m_infoPanel     = new TraceInfoPanel(this);
    wxBoxSizer * vsizer = new wxBoxSizer(wxVERTICAL);
    vsizer->Add(m_tracePanel, 1, wxEXPAND | wxALL);
    vsizer->Add(m_infoPanel, 0, wxEXPAND | wxALL);
    SetSizer(vsizer);
}

void CompositeTracePanel::UpdateData()
{
    m_tracePanel->UpdateData();
    m_infoPanel->UpdateData();
}

void CompositeTracePanel::OnSelectionChange( int index )
{
    if (index < 0) return;
    const ATracer::TraceVec & v = m_tracer->GetData();
    if (index >= (int) v.size()) return;
    m_infoPanel->UpdateData(v[index]);
}

