///////////////////////////////////////////////////////////////////////////////
// Name:        svgframe.h
// Purpose:     wxFrame for testing SVG rasterization with NanoSVG and LunaSVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////


#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/choicdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/dcbuffer.h>
#include <wx/ffile.h>
#include <wx/filectrl.h>
#include <wx/filename.h>
#include <wx/numdlg.h>
#include <wx/slider.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/utils.h>

#include "svgframe.h"
#include "svgbench.h"
#include "bmpbndl_lunasvg.h"


// ============================================================================
// wxBitmapBundlePanel
// ============================================================================

class wxBitmapBundlePanel : public wxScrolledCanvas
{
public:
    wxBitmapBundlePanel(wxWindow* parent, const wxSize& bitmapSize);

    void SetBitmapBundle(const wxBitmapBundle& bundle);
    void SetBitmapSize(const wxSize& size);
private:
    wxBitmapBundle m_bitmapBundle;
    wxSize         m_bitmapSize;

    void OnPaint(wxPaintEvent&);
};

wxBitmapBundlePanel::wxBitmapBundlePanel(wxWindow* parent, const wxSize& bitmapSize)
    : wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      m_bitmapSize(bitmapSize)
{
    wxASSERT(m_bitmapSize.x > 0 && m_bitmapSize.y > 0);

    SetScrollRate(FromDIP(4), FromDIP(4));

    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &wxBitmapBundlePanel::OnPaint, this);
}

void wxBitmapBundlePanel::SetBitmapBundle(const wxBitmapBundle& bundle)
{
    m_bitmapBundle = bundle;
    Refresh(); Update();
}

void wxBitmapBundlePanel::SetBitmapSize(const wxSize& size)
{
    wxCHECK_RET(size.x > 0 && size.y > 0, "invalid bitmapSize");

    m_bitmapSize = size;

    if ( m_bitmapSize != GetVirtualSize() )
    {
        InvalidateBestSize();
        SetVirtualSize(m_bitmapSize);
    }

    Refresh(); Update();
}

void wxBitmapBundlePanel::OnPaint(wxPaintEvent&)
{
    const wxSize   clientSize(GetClientSize());

    wxAutoBufferedPaintDC dc(this);
    wxBitmap              bitmap;

    DoPrepareDC(dc);

    dc.SetBackground(*wxWHITE);
    dc.Clear();
    bitmap = m_bitmapBundle.GetBitmap(m_bitmapSize);


    if ( bitmap.IsOk() )
    {
        wxBrush          hatchBrush(*wxBLUE, wxBRUSHSTYLE_CROSSDIAG_HATCH);
        wxDCBrushChanger bc(dc, hatchBrush);
        wxDCPenChanger   pc(dc, wxNullPen);

        dc.DrawRectangle(wxPoint(0, 0), m_bitmapSize);
        dc.DrawBitmap(bitmap, 0, 0, true);
    }
}

// ============================================================================
// wxTestSVGFrame
// ============================================================================

wxTestSVG2Frame::wxTestSVG2Frame()
    : wxFrame(nullptr, wxID_ANY, "wxTestSVG")
{
    wxConfigBase* config = wxConfigBase::Get();

    m_lastSVGFolder = config->Read("lastSVGFolder", m_lastSVGFolder);
    m_lastRunCount  = config->Read("lastRunCount", m_lastRunCount);

    SetIcon(wxICON(wxICON_AAA)); // from wx.rc

    wxSplitterWindow* splitterMain = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition,
                                                          wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    wxPanel*          controlPanel = new wxPanel(splitterMain); // for controls
    wxPanel*          bitmapPanel  = new wxPanel(splitterMain); // for wxBitmapBundlePanels

    wxBoxSizer*       controlPanelSizer = new wxBoxSizer(wxVERTICAL);

    controlPanelSizer->Add(new wxStaticText(controlPanel, wxID_ANY, "&Bitmap Size"),
                           wxSizerFlags().CenterHorizontal().Border(wxALL & ~wxBOTTOM));

    m_bitmapSizeSlider = new wxSlider(controlPanel, wxID_ANY, m_bitmapSize.x, 16, 512,
                                      wxDefaultPosition, wxDefaultSize,
                                      wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS );
    m_bitmapSizeSlider->SetTickFreq(16);
    m_bitmapSizeSlider->Bind(wxEVT_SLIDER, &wxTestSVG2Frame::OnBitmapSizeChanged, this);
    controlPanelSizer->Add(m_bitmapSizeSlider, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));

    controlPanelSizer->Add(new wxStaticLine(controlPanel), wxSizerFlags().Expand().Border());

    wxButton* benchmarkFolderBtn = new wxButton(controlPanel, wxID_ANY, "Benchmark &Curent Folder...");
    benchmarkFolderBtn->Bind(wxEVT_BUTTON, &wxTestSVG2Frame::OnBenchmarkFolder, this);
    controlPanelSizer->Add(benchmarkFolderBtn, wxSizerFlags().Expand().Border());

    wxButton* changeFolderBtn = new wxButton(controlPanel, wxID_ANY, "Change &Folder...");
    changeFolderBtn->Bind(wxEVT_BUTTON, &wxTestSVG2Frame::OnChangeFolder, this);
    controlPanelSizer->Add(changeFolderBtn, wxSizerFlags().Expand().Border());

    m_fileCtrl = new wxFileCtrl(controlPanel, wxID_ANY, m_lastSVGFolder, ".", "SVG files (*.svg)|*.svg",
                                wxFC_DEFAULT_STYLE | wxFC_NOSHOWHIDDEN);
    m_fileCtrl->Bind(wxEVT_FILECTRL_FILEACTIVATED, &wxTestSVG2Frame::OnFileActivated, this);
    m_fileCtrl->Bind(wxEVT_FILECTRL_SELECTIONCHANGED, &wxTestSVG2Frame::OnFileSelected, this);
    controlPanelSizer->Add(m_fileCtrl, wxSizerFlags(1).Expand().Border());

    controlPanel->SetSizerAndFit(controlPanelSizer);

    m_panelNano = new wxBitmapBundlePanel(bitmapPanel, m_bitmapSize);
    m_panelLuna = new wxBitmapBundlePanel(bitmapPanel, m_bitmapSize);

    wxFlexGridSizer* bitmapPanelSizer = new wxFlexGridSizer(m_panelLuna ? 2 : 1);

    bitmapPanelSizer->Add(new wxStaticText(bitmapPanel, wxID_ANY, "NanoSVG"),
                           wxSizerFlags().CenterHorizontal().Border(wxALL));

    if ( m_panelLuna )
    {
        bitmapPanelSizer->Add(new wxStaticText(bitmapPanel, wxID_ANY, "LunaSVG"),
                               wxSizerFlags().CenterHorizontal().Border(wxALL));
    }

    bitmapPanelSizer->Add(m_panelNano, wxSizerFlags(1).Expand().Border());

    if ( m_panelLuna )
        bitmapPanelSizer->Add(m_panelLuna, wxSizerFlags(1).Expand().Border());

    bitmapPanelSizer->AddGrowableCol(0, 1);
    if ( bitmapPanelSizer->GetCols() > 1 )
        bitmapPanelSizer->AddGrowableCol(1, 1);
    bitmapPanelSizer->AddGrowableRow(1, 1);

    bitmapPanel->SetSizerAndFit(bitmapPanelSizer);

    SetMinClientSize(FromDIP(wxSize(800, 600)));
    splitterMain->SetMinimumPaneSize(FromDIP(100));
    splitterMain->SetSashGravity(0.3);
    splitterMain->SplitVertically(controlPanel, bitmapPanel, FromDIP(256));
}

wxTestSVG2Frame::~wxTestSVG2Frame()
{
    wxConfigBase* config = wxConfigBase::Get();

    config->Write("lastSVGFolder", m_fileCtrl->GetDirectory());
    config->Write("lastRunCount", m_lastRunCount);
}

void wxTestSVG2Frame::OnFileSelected(wxFileCtrlEvent& event)
{
    const wxFileName fileName(event.GetDirectory(), event.GetFile());

    m_panelNano->SetBitmapBundle(wxBitmapBundle::FromSVGFile(fileName.GetFullPath(), m_bitmapSize));
    m_panelLuna->SetBitmapBundle(CreateFromFileWithLunaSVG(fileName.GetFullPath(), m_bitmapSize));
}

void wxTestSVG2Frame::OnFileActivated(wxFileCtrlEvent& event)
{
   const wxFileName fileName(event.GetDirectory(), event.GetFile());

   wxLaunchDefaultApplication(fileName.GetFullPath());
}

void wxTestSVG2Frame::OnBenchmarkFolder(wxCommandEvent&)
{
#ifndef NDEBUG
    if ( wxMessageBox("It appears you are running the debug version of the application, "
                      "which is much slower than the release one. Continue anyway?",
                      "Warning", wxYES_NO | wxNO_DEFAULT) != wxYES )
    {
        return;
    }
#endif
    const wxString dirName = m_fileCtrl->GetDirectory();
    wxArrayString  dirFiles;
    wxArrayString  files;
    wxArrayInt     selections;

    {
        wxBusyCursor bc;
        wxDir::GetAllFiles(dirName, &dirFiles, "*.svg", wxDIR_FILES);
    }

    if ( dirFiles.empty() )
    {
        wxLogMessage("No SVG files found in the current folder.");
        return;
    }

    for ( auto& f : dirFiles )
        f = wxFileName(f).GetFullName();

    dirFiles.Sort(wxNaturalStringSortAscending);

    selections.reserve(dirFiles.size());
    for ( size_t i = 0; i < dirFiles.size(); ++i )
        selections.push_back(i);

    if ( wxGetSelectedChoices(selections, wxString::Format("Select Files (%zu files available)", dirFiles.size()),
                              "Benchmark Rasterization", dirFiles, this) == -1
         || selections.empty() )
    {
        return;
    }

    for ( const auto& s : selections )
        files.push_back(dirFiles[s]);

    static const wxSize bitmapSizes[] =
      { {  16,  16},
        {  24,  24},
        {  32,  32},
        {  48,  48},
        {  64,  64},
        {  96,  96},
        { 128, 128},
        { 256, 256},
        { 512, 512},
        {1024,1024}, };

    wxArrayString bitmapSizesStrings;

    for ( const auto& bs : bitmapSizes)
        bitmapSizesStrings.push_back(wxString::Format("%d x %d", bs.x, bs.y));

    selections.clear();
    selections.push_back(1); //24x24
    selections.push_back(3); //48x48
    selections.push_back(6); //128x128

    if ( wxGetSelectedChoices(selections, "Select Bitmap Sizes", "Benchmark Rasterization", bitmapSizesStrings, this) == -1
         || selections.empty() )
        return;

    long runCount = wxGetNumberFromUser("Number of runs (between 10 and 100)", "Number", "Benchmark Rasterization", m_lastRunCount, 10, 100);

    if ( runCount == -1 )
        return;

    m_lastRunCount = runCount;

    wxTestSVGRasterizationBenchmark benchmark;

    std::vector<wxSize> sizes;

    for ( const auto& s : selections )
        sizes.push_back(bitmapSizes[s]);

    benchmark.Setup(dirName, files, sizes);

    wxString report, detailedReport;
    bool result = false;

    {
        wxBusyInfo info(wxString::Format("Benchmarking %zu files at %zu sizes (%ld runs each, %zu runs total), please wait...",
            files.size(), sizes.size(), runCount, files.size() * sizes.size() * runCount), this);
        result = benchmark.Run(runCount, report, detailedReport);
    }

    if ( result )
        new wxTestSVGBenchmarkReportFrame(this, dirName, report, detailedReport);
}

void wxTestSVG2Frame::OnChangeFolder(wxCommandEvent&)
{
    const wxString dir = wxDirSelector("Select Folder", m_fileCtrl->GetDirectory(), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if ( !dir.empty() )
        m_fileCtrl->SetDirectory(dir);
}

void wxTestSVG2Frame::OnBitmapSizeChanged(wxCommandEvent& event)
{
    m_bitmapSize = wxSize(event.GetInt(), event.GetInt());

    m_panelNano->SetBitmapSize(m_bitmapSize);
    if ( m_panelLuna )
        m_panelLuna->SetBitmapSize(m_bitmapSize);
}