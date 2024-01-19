/////////////////////////////////////////////////////////////////////////////
// Name:        bmpbndl_lunasvg.h
// Purpose:     wxBitmapBundleImpl using LunaSVG to rasterize SVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef BMPBNDL_LUNASVG_H_DEFINED
#define BMPBNDL_LUNASVG_H_DEFINED

#include <wx/types.h>
#include <cstddef>

class wxBitmapBundle;
class wxSize;
class wxString;

// Creates wxBitmapBundle from in-memory SVG using wxBitmapBundleImplLunaSVG
wxBitmapBundle CreateWithLunaSVGFromMemory(const wxByte* data, size_t len, const wxSize& sizeDef);

// Creates wxBitmapBundle from SVG file using wxBitmapBundleImplLunaSVG
wxBitmapBundle CreateWithLunaSVGFromFile(const wxString& path, const wxSize& sizeDef);

#endif // #ifndef BMPBNDL_LUNASVG_H_DEFINED