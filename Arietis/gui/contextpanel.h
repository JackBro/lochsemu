#pragma once

#ifndef __ARIETIS_GUI_REGPANEL_H__
#define __ARIETIS_GUI_REGPANEL_H__

#include "Arietis.h"
#include "gui.h"
#include "parallel.h"
#include "engine.h"

class ContextPanel : public wxPanel {
public:
    ContextPanel(wxWindow *parent);
    ~ContextPanel();

    void        OnPaint(wxPaintEvent &event);
    void        OnEraseBackground(wxEraseEvent& event);
    void        UpdateData(const InstContext &ctx, const char *dataDesc);
private:
    void        InitRender();
    void        Draw(wxBufferedPaintDC &dc);
private:
    MutexCS     m_mutex;
    InstContext m_data;
    wxString    m_dataDesc;

    static const wxString  RegLabels[];
    wxFont      m_font;
    int         m_lineHeight;
    int         m_widthRegName;
    int         m_widthRegValue;

    wxBrush     m_bgBrush;
};

#endif // __ARIETIS_GUI_REGPANEL_H__