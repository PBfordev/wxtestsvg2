///////////////////////////////////////////////////////////////////////////////
// Name:        svgframe.h
// Purpose:     wxFrame for testing SVG rasterization with NanoSVG and LunaSVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SVG_FRAME_H_DEFINED
#define TEST_SVG_FRAME_H_DEFINED

#include <wx/wx.h>

class wxFileCtrl;
class wxFileCtrlEvent;

class wxBitmapBundlePanel;

class wxTestSVG2Frame : public wxFrame
{
public:
    wxTestSVG2Frame();
    ~wxTestSVG2Frame();
private:
    wxString m_lastSVGFolder;
    long     m_lastRunCount{25};

    wxSize               m_bitmapSize{128, 128};

    wxSlider*            m_bitmapSizeSlider{nullptr};
    wxFileCtrl*          m_fileCtrl{nullptr};
    wxBitmapBundlePanel* m_panelNano{nullptr};
    wxBitmapBundlePanel* m_panelLuna{nullptr};

    void OnBenchmarkFolder(wxCommandEvent&);
    void OnChangeFolder(wxCommandEvent&);
    void OnFileSelected(wxFileCtrlEvent& event);
    void OnFileActivated(wxFileCtrlEvent& event);
    void OnBitmapSizeChanged(wxCommandEvent& event);
};

#endif // #ifndef TEST_SVG_FRAME_H_DEFINED