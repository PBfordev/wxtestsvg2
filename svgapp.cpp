///////////////////////////////////////////////////////////////////////////////
// Name:        svgapp.cpp
// Purpose:     Application which just shows wxTestSVG2Frame
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>

#include "svgframe.h"

class wxTestSVG2App : public wxApp
{
    bool OnInit() override
    {
        SetVendorName("PB");
        SetAppName("wxTestSVG2");

        (new wxTestSVG2Frame)->Show();
        return true;
    }
}; wxIMPLEMENT_APP(wxTestSVG2App);
