/////////////////////////////////////////////////////////////////////////////
// Name:        bmpbndl_lunasvg.h
// Purpose:     wxBitmapBundleImpl using LunaSVG to rasterize SVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef BMPBUNDLE_IMPL_LUNA_H_DEFINED
#define BMPBUNDLE_IMPL_LUNA_H_DEFINED

class wxBitmapBundle;
class wxSize;
class wxString;

// Creates wxBitmapBundle from SVG file using wxBitmapBundleImplLunaSVG
wxBitmapBundle CreateFromFileWithLunaSVG(const wxString& path, const wxSize& sizeDef);

#endif // #ifndef BMPBUNDLE_IMPL_LUNA_H_DEFINED