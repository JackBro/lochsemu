#include "stdafx.h"

#include "logpanel.h"
#include "mainframe.h"


LogPanel::LogPanel( wxWindow *parent )
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(600, 150))
{
    InitUI();
}

void LogPanel::InitUI()
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *sizer0 = new wxBoxSizer(wxHORIZONTAL);
    wxButton *buttonClear = new wxButton(this, wxID_CLEAR, "Clear", wxDefaultPosition,
        wxSize(35, 20));
    sizer0->Add(buttonClear, 0, wxALIGN_RIGHT);
    sizer->Add(sizer0, 0, wxALL | wxALIGN_RIGHT);

    m_log = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_RICH2 | wxNO_BORDER);
    wxFont f(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Lucida Console");
    m_log->SetFont(f);
    sizer->Add(m_log, 1, wxALL | wxEXPAND);
    
    SetSizer(sizer);
    
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LogPanel::OnClear, this, wxID_CLEAR);
}

void LogPanel::OnClear(wxCommandEvent &event)
{
    m_log->Clear();
}

void LogPanel::Log( const wxString &s )
{
    m_log->AppendText(s + "\n");
}
