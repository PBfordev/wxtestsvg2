/////////////////////////////////////////////////////////////////////////////
// Name:        bmpbndl_lunasvg.cpp
// Purpose:     wxBitmapBundleImpl using LunaSVG to rasterize SVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#include <wx/bmpbndl.h>
#if wxUSE_FFILE
    #include <wx/ffile.h>
#elif wxUSE_FILE
    #include <wx/file.h>
#endif

#include <wx/log.h>
#include <wx/rawbmp.h>

#include <memory>

#include <lunasvg.h>

#include "bmpbndl_lunasvg.h"


// ============================================================================
// wxBitmapBundleImplLunaSVG declaration
// ============================================================================

/*
    wxBitmapBundleImpl using LunaSVG to rasterize an SVG to wxBitmap at given size.

    Modelled after wxBitmapBundleImplSVG using NanoSVG.
*/

class wxBitmapBundleImplLunaSVG : public wxBitmapBundleImpl
{
public:
    // data must be 0 terminated, wxBitmapBundleImplLunaSVG doesn't
    // take its ownership and it can be deleted after the ctor was called.
    wxBitmapBundleImplLunaSVG(const char* data, const wxSize& sizeDef);

    // wxBitmapBundleImplLunaSVG doesn't take its ownership of data and it
    // can be deleted after the ctor was called. len is data length in bytes.
    wxBitmapBundleImplLunaSVG(const char* data, size_t len, const wxSize& sizeDef);

    virtual wxSize GetDefaultSize() const override;
    virtual wxSize GetPreferredBitmapSizeAtScale(double scale) const override;
    
    virtual wxBitmap GetBitmap(const wxSize& size) override;
    
    bool IsOk() const;
private:
    const wxSize m_sizeDef;
    wxBitmap m_cachedBitmap;
    std::unique_ptr<lunasvg::Document> m_svgDocument;

    wxBitmap DoRasterize(const wxSize& size);

    wxDECLARE_NO_COPY_CLASS(wxBitmapBundleImplLunaSVG);
};


// Creates wxBitmapBundle from SVG file using wxBitmapBundleImplLunaSVG
wxBitmapBundle CreateFromFileWithLunaSVG(const wxString& path, const wxSize& sizeDef)
{
#if wxUSE_FFILE
    wxFFile file(path, "rb");
#elif wxUSE_FILE
    wxFile file(path);
#else
    #error "wxWidgets must be built with support for wxFFile or wxFile."
#endif
    if ( file.IsOpened() )
    {
        const wxFileOffset lenAsOfs = file.Length();
        if ( lenAsOfs != wxInvalidOffset )
        {
            const size_t len = static_cast<size_t>(lenAsOfs);
            wxMemoryBuffer buf(len);
            char* const  ptr = static_cast<char*>(buf.GetData());

            if ( file.Read(ptr, len) == len )
                return wxBitmapBundle::FromImpl(new wxBitmapBundleImplLunaSVG(ptr, len, sizeDef));
        }
    }

    return wxBitmapBundle();
}


// ============================================================================
// wxBitmapBundleImplLunaSVG implementation
// ============================================================================

wxBitmapBundleImplLunaSVG::wxBitmapBundleImplLunaSVG(const char* data, const wxSize& sizeDef)
    : m_sizeDef(sizeDef)
{
    wxCHECK_RET(data != nullptr, "null data");
    wxCHECK_RET(sizeDef.GetWidth() > 0 && sizeDef.GetHeight() > 0, "invalid default size");

    m_svgDocument = lunasvg::Document::loadFromData(data);
}

wxBitmapBundleImplLunaSVG::wxBitmapBundleImplLunaSVG(const char* data, size_t len, const wxSize& sizeDef)
    : m_sizeDef(sizeDef)
{
    wxCHECK_RET(data != nullptr, "null data");
    wxCHECK_RET(len > 0, "zero length");
    wxCHECK_RET(sizeDef.GetWidth() > 0 && sizeDef.GetHeight() > 0, "invalid default size");

    m_svgDocument = lunasvg::Document::loadFromData(data, len);
}

wxSize wxBitmapBundleImplLunaSVG::GetDefaultSize() const
{
    return m_sizeDef;
};

wxSize wxBitmapBundleImplLunaSVG::GetPreferredBitmapSizeAtScale(double scale) const
{
    return m_sizeDef*scale;
}


wxBitmap wxBitmapBundleImplLunaSVG::GetBitmap(const wxSize& size)
{
    if ( !m_cachedBitmap.IsOk() || m_cachedBitmap.GetSize() != size )
        m_cachedBitmap = DoRasterize(size);

    return m_cachedBitmap;
}

bool wxBitmapBundleImplLunaSVG::IsOk() const
{
    return m_svgDocument != nullptr;
}

wxBitmap wxBitmapBundleImplLunaSVG::DoRasterize(const wxSize& size)
{
    if ( IsOk() )
    {
        const lunasvg::Bitmap lbmp = m_svgDocument->renderToBitmap(size.x, size.y);

        if ( lbmp.valid() )
        {
            const auto width = lbmp.width();
            const auto height = lbmp.height();
            const auto stride = lbmp.stride();
            auto rowData = lbmp.data();

            wxBitmap bmp(width, height, 32);
            wxAlphaPixelData bmpdata(bmp);
            wxAlphaPixelData::Iterator dst(bmpdata);

            for ( std::uint32_t y = 0; y < height; ++y )
            {
                auto data = rowData;
                dst.MoveTo(bmpdata, 0, y);

                for ( std::uint32_t x = 0; x < width; ++x, ++dst )
                {
                    auto b = data[0];
                    auto g = data[1];
                    auto r = data[2];
                    auto a = data[3];
#ifndef wxHAS_PREMULTIPLIED_ALPHA
                    if (a != 0 )
                    {
                        r = (r * 255) / a;
                        g = (g * 255) / a;
                        b = (b * 255) / a;
                    }
#endif
                    dst.Red()   = r;
                    dst.Green() = g;
                    dst.Blue()  = b;
                    dst.Alpha() = a;

                    data += 4;
                }

                rowData += stride;
            }

            return bmp;
        }
        else
            wxLogDebug("invalid lunasvg::Bitmap");
    }

    return wxBitmap();
}