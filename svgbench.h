///////////////////////////////////////////////////////////////////////////////
// Name:        svgbench.h
// Purpose:     Benchmark SVG rasterization with NanoSVG and LunaSVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SVG_BENCH_H_DEFINED
#define TEST_SVG_BENCH_H_DEFINED

#include <vector>

#include <wx/wx.h>
#include <wx/buffer.h>

// ============================================================================
// wxTestSVGRasterizationBenchmark
// ============================================================================

class wxTestSVGRasterizationBenchmark
{
public:
    wxTestSVGRasterizationBenchmark();

    void Setup(const wxString& dirName, const wxArrayString& fileNames,
               const std::vector<wxSize>& sizes);

    bool Run(size_t runCount, wxString& report, wxString& detailedReport);

private:
    // times in ms for one file and one bitmap size
    using VectorLong  = std::vector<long>;
    using MatrixLong2 = std::vector<VectorLong>;
    using MatrixLong3 = std::vector<MatrixLong2>;

    struct Stats
    {
        long min{0};
        long max{0};
        long mdn{0};
        long avg{0};
    };
    using VectorStats = std::vector<Stats>;
    using MatrixStats = std::vector<VectorStats>;

    using CreateBitmapBundleFn = wxBitmapBundle (*) (const wxMemoryBuffer&);

    wxString            m_dirName;
    wxArrayString       m_fileNames;
    std::vector<wxSize> m_sizes;

    // benchmarks a single file for all bitmap sizes
    bool BenchmarkFile(CreateBitmapBundleFn createBundleFn,
                       const wxString& fileName,
                       size_t runCount, MatrixLong2& times);

    void CreateReport(const MatrixStats& statsNano, const MatrixStats& statsD2D,
                      size_t runCount, wxString& reportText);

    void CreateDetailedReport(const MatrixLong3& timesNano, const MatrixStats& statsNano,
                              const MatrixLong3& timesLuna, const MatrixStats& statsLuna,
                              bool asHTML, wxString& reportText);

    static Stats CalcStatsForVectorLong(const VectorLong& data);
};


// ============================================================================
// wxTestSVGBenchmarkReportFrame
// ============================================================================

class wxTestSVGBenchmarkReportFrame: public wxFrame
{
public:
    wxTestSVGBenchmarkReportFrame(wxWindow* parent, const wxString& dirName,
                                  const wxString& report, const wxString& detailedReport);
private:
    enum
    {
        ID_SAVE_DETAILED = wxID_HIGHEST + 2
    };

    wxString m_dirName;
    wxString m_defaultName;
    wxString m_report, m_detailedReport;

    void OnSaveReport(wxCommandEvent&);
    void OnSaveDetailedReport(wxCommandEvent&);

    static bool WriteHTMLReport(const wxString& fileName, const wxString& reportText);
};

#endif // #ifndef TEST_SVG_BENCH_H_DEFINED