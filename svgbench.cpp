///////////////////////////////////////////////////////////////////////////////
// Name:        svgbench.cpp
// Purpose:     Benchmark SVG rasterization with NanoSVG and LunaSVG
// Author:      PB
// Created:     2024-01-18
// Copyright:   (c) 2024 PB
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <climits>
#include <numeric>

#include <wx/dirdlg.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/stopwatch.h>
#include <wx/textfile.h>
#include <wx/webview.h>

#include "bmpbndl_lunasvg.h"

#include "svgbench.h"

// ============================================================================
// wxTestSVGRasterizationBenchmark
// ============================================================================

wxTestSVGRasterizationBenchmark::wxTestSVGRasterizationBenchmark()
{
}

void wxTestSVGRasterizationBenchmark::Setup(const wxString& dirName,
                                            const wxArrayString& fileNames,
                                            const std::vector<wxSize>& sizes)
{
    m_dirName   = dirName;
    m_fileNames = fileNames;
    m_sizes     = sizes;
}

wxBitmapBundle CreateBitmapBundleNano(const wxString& fileName)
{
    return wxBitmapBundle::FromSVGFile(fileName, wxSize(2, 2));
}

wxBitmapBundle CreateBitmapBundleLuna(const wxString& fileName)
{
    return CreateFromFileWithLunaSVG(fileName, wxSize(2, 2));
}

bool wxTestSVGRasterizationBenchmark::Run(size_t runCount,
                                          wxString& report, wxString& detailedReport)
{
    wxCHECK(!m_fileNames.empty(), false);
    wxCHECK(!m_sizes.empty(), false);
    wxCHECK(runCount, false);

    MatrixLong3 timesNano(m_fileNames.size());
    MatrixLong3 timesLuna(m_fileNames.size());

    for ( size_t f = 0; f < m_fileNames.size(); ++f )
    {
        if ( !BenchmarkFile(CreateBitmapBundleNano, m_fileNames[f], runCount, timesNano[f]) )
            return false;
        if ( !BenchmarkFile(CreateBitmapBundleLuna, m_fileNames[f], runCount, timesLuna[f]) )
            return false;
    }

    MatrixStats statsNano;
    MatrixStats statsLuna;

    statsNano.resize(m_fileNames.size());
    for ( auto& s : statsNano )
        s.resize(m_sizes.size());

    statsLuna.resize(m_fileNames.size());
    for ( auto& s : statsLuna )
        s.resize(m_sizes.size());

    for ( size_t f = 0; f < m_fileNames.size(); ++f )
    {
        for ( size_t s = 0; s < m_sizes.size(); ++s )
        {
            statsNano[f][s] = CalcStatsForVectorLong(timesNano[f][s]);
            statsLuna[f][s] = CalcStatsForVectorLong(timesLuna[f][s]);
        }
    }

    CreateReport(statsNano, statsLuna, runCount, report);

    CreateDetailedReport(timesNano, statsNano,
                         timesLuna, statsLuna,
                         true, detailedReport);

    return true;
}

bool wxTestSVGRasterizationBenchmark::BenchmarkFile(CreateBitmapBundleFn fn,
                                                    const wxString& fileName,
                                                    size_t runCount, MatrixLong2& times)
{
    wxStopWatch stopWatch;

    times.resize(m_sizes.size());
    for ( auto& t : times )
        t.resize(runCount);

    for ( size_t run = 0; run < runCount; ++run )
    {
        const wxBitmapBundle bundle = fn(wxFileName(m_dirName, fileName).GetFullPath());

        wxBitmap   bitmap;
        wxLongLong time;

        for ( size_t s = 0; s < m_sizes.size(); ++s )
        {
            const wxSize& bitmapSize = m_sizes[s];

            stopWatch.Start();
            bitmap = bundle.GetBitmap(bitmapSize);
            time = stopWatch.TimeInMicro();
            times[s][run] = time.ToLong();

            if ( !bitmap.IsOk() )
             {
                wxLogError("Couldn't rasterize file '%s' at size %dx%d.", fileName, bitmapSize.x, bitmapSize.y);
                return false;
            }
        }
    }

    return true;
}

void wxTestSVGRasterizationBenchmark::CreateReport(const MatrixStats& statsNano, const MatrixStats& statsLuna,
                                                   size_t runCount, wxString& reportText)
{
    wxArrayString       result;
    wxString            rowStr;
    std::vector<double> sumsNano(m_sizes.size());
    VectorLong          minsNano(m_sizes.size(), LONG_MAX), maxesNano(m_sizes.size(), LONG_MIN);
    std::vector<double> sumsLuna(m_sizes.size());
    VectorLong          minsLuna(m_sizes.size(), LONG_MAX), maxesLuna(m_sizes.size(), LONG_MIN);

    rowStr = R"(<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="description" content="wxTestSVG2 Report">)";
    rowStr += "<style>";
    rowStr += "table, th, td, tfoot {border: 1px solid black; border-collapse: collapse;} td {text-align: right;} tfoot {color: red;} ";
    rowStr += "body {font-family: Verdana, Arial, Helvetica, sans-serif;}";
    rowStr += "</style></head><body>\n";
    result.push_back(rowStr);

    result.push_back(wxString::Format("<h3>Benchmarked %zu files from folder '%s' (%zu runs)</h1>",
        m_fileNames.size(), m_dirName, runCount));
    result.push_back("<p>Unless indicated otherwise, the times are medians in microseconds</p>");

    // create headers
    rowStr = R"(<table>)";
    rowStr += R"(<thead><tr>)";
    rowStr += R"(<th rowspan="2">File</th>)";
    for ( const auto& s : m_sizes )
    {
        rowStr += wxString::Format(R"(<th colspan="2">%dx%d</th>)", s.x, s.y);
    }
    rowStr += R"(</tr>)";
    rowStr += "\n";
    result.push_back(rowStr);

    rowStr = R"(<tr>)";
    for ( size_t i = 0; i < m_sizes.size(); ++i )
        rowStr += "<th>Nano</th><th>Luna</th>";
    rowStr += R"(</tr>)";
    rowStr += R"(</thead>)";
    rowStr += "\n";
    result.push_back(rowStr);

    result.push_back("<tbody>\n");
    for ( size_t f = 0; f < m_fileNames.size(); ++f )
    {
        rowStr = wxString::Format("<tr><td>%s</td>", wxFileName(m_fileNames[f]).GetName());
        for ( size_t s = 0; s < m_sizes.size(); ++s )
        {
            rowStr += wxString::Format("<td>%ld</td><td>%ld</td>", statsNano[f][s].mdn, statsLuna[f][s].mdn);

            sumsNano[s] += statsNano[f][s].mdn;
            if ( statsNano[f][s].mdn < minsNano[s] )
                minsNano[s] = statsNano[f][s].mdn;
            if ( statsNano[f][s].mdn > maxesNano[s] )
                maxesNano[s] = statsNano[f][s].mdn;

            sumsLuna[s] += statsLuna[f][s].mdn;
            if ( statsLuna[f][s].mdn < minsLuna[s] )
                minsLuna[s] = statsLuna[f][s].mdn;
            if ( statsLuna[f][s].mdn > maxesLuna[s] )
                maxesLuna[s] = statsLuna[f][s].mdn;
        }
        rowStr += "</tr>\n";
        result.push_back(rowStr);
    }
    result.push_back("</tbody>\n");

    wxString sumsStr, minsStr, maxesStr;

    sumsStr = "<tfoot><tr><td>Sum (milliseconds)</td>";
    minsStr = "<tr><td>Min</td>";
    maxesStr = "<tr><td>Max</td>";
    for ( size_t s = 0; s < m_sizes.size(); ++s )
    {
        sumsStr += wxString::Format("<td>%.2f</td><td>%.2f</td>", sumsNano[s] / 1000, sumsLuna[s] / 1000);
        minsStr += wxString::Format("<td>%ld</td><td>%ld</td>", minsNano[s], minsLuna[s]);
        maxesStr += wxString::Format("<td>%ld</td><td>%ld</td>", maxesNano[s], maxesLuna[s]);
    }
    result.push_back(sumsStr + "</tr>\n");
    result.push_back(minsStr + "</tr>\n");
    result.push_back(maxesStr + "</tr>\n");
    result.push_back("<tfoot>");
    result.push_back("</table>\n");
    result.push_back("</body></html>");

    for ( const auto& r : result )
        reportText += r + "\n";
}

// if !asHTML, the result is plaintext with the values separated by tabs
void wxTestSVGRasterizationBenchmark::CreateDetailedReport(const MatrixLong3& timesNano, const MatrixStats& statsNano,
                                                           const MatrixLong3& timesLuna,  const MatrixStats& statsLuna,
                                                           bool asHTML, wxString& reportText)
{
    const size_t runCount = timesNano[0][0].size();

    wxArrayString result;
    wxString      rowStr;

    if ( asHTML )
    {
        rowStr = R"(<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="description" content="wxTestSVG2 Benchmark Detailed Report">)";
        rowStr += "<style>";
        rowStr += "table, th, td {border: 1px solid black; border-collapse: collapse} td {text-align: right}";
        rowStr += "body {font-family: Verdana, Arial, Helvetica, sans-serif}";
        rowStr += "</style></head><body>\n";
        result.push_back(rowStr);
    }

    // create headers
    if ( asHTML )
    {
        result.push_back(wxString::Format("<h3>Benchmarked %zu files from folder '%s'</h1>", m_fileNames.size(), m_dirName));
        result.push_back("<p>All times are in microseconds</p>");
        rowStr = R"(<table style="width:100%">)";
        rowStr += R"(<thead><tr>)";
        rowStr += R"(<th rowspan="3">Run</th>)";
        for ( const auto& f : m_fileNames )
        {
            rowStr += wxString::Format(R"(<th colspan="%zu">%s</th>)",
                m_sizes.size() * 2, wxFileName(f).GetName());
        }
        rowStr += R"(</tr>)";
    }
    else
    {
        rowStr = "Run\t";
        for ( const auto& f : m_fileNames )
        {
            rowStr += wxFileName(f).GetName();
            rowStr += wxString('\t', m_sizes.size() * 2);
        }
    }
    rowStr += "\n";
    result.push_back(rowStr);

    if ( asHTML )
    {
        rowStr = R"(<tr>)";
        for ( const auto& f : m_fileNames )
        {
            wxUnusedVar(f);
            for ( const auto& s : m_sizes )
                rowStr += wxString::Format(R"(<th colspan="2">%dx%d</th>)", s.x, s.y);
        }
        rowStr += R"(</tr>)";
    }
    else
    {
        rowStr = "\t"; // Run column
        for ( const auto& f : m_fileNames )
        {
            wxUnusedVar(f);
            for ( const auto& s : m_sizes )
                rowStr += wxString::Format("%dx%d\t\t", s.x, s.y);
        }
        rowStr.RemoveLast(2); // extra tabs at the end of the row
    }
    rowStr += "\n";
    result.push_back(rowStr);

    if ( asHTML )
    {
        rowStr = R"(<tr>)";
        for ( size_t i = 0; i < m_fileNames.size() * m_sizes.size(); ++i )
            rowStr += "<th>Nano</th><th>Luna</th>";
        rowStr += R"(</tr>)";
        rowStr += R"(</thead>)";
    }
    else
    {
        rowStr.clear();
        for ( size_t i = 0; i < m_fileNames.size() * m_sizes.size(); ++i )
            rowStr += "\tNano\tLuna";
    }
    rowStr += "\n";
    result.push_back(rowStr);

    const wxChar* valueFormatHTML = wxS("<td>%ld</td><td>%ld</td>");
    const wxChar* valueFormatTSV = wxS("%ld\t%ld\t");

    if ( asHTML )
        result.push_back("<tbody>\n");
    for ( size_t run = 0; run < runCount; ++run )
    {
        if ( asHTML )
            rowStr.Printf(R"(<tr><td>%zu</td>)", run + 1);
        else
            rowStr.Printf("%zu\t", run + 1);

        for ( size_t f = 0; f < m_fileNames.size(); ++f )
        {
            for ( size_t s = 0; s < m_sizes.size(); ++s )
            {
                rowStr += wxString::Format(asHTML ? valueFormatHTML : valueFormatTSV,
                    timesNano[f][s][run], timesLuna[f][s][run]);
            }

        }
        if ( asHTML )
            rowStr += R"(</tr>)";
        else
            rowStr.RemoveLast(); // extra tab

        rowStr += "\n";
        result.push_back(rowStr);
    }

    if ( asHTML )
        result.push_back("</tbody>\n");

    wxString mdnRow("Median");
    wxString avgRow("Mean");
    wxString minRow("Min ");
    wxString maxRow("Max");

    if ( asHTML )
    {
        result.push_back("<tfoot>");
        mdnRow.Printf("<tr><td>%s</td>", mdnRow);
        avgRow.Printf("<tr><td>%s</td>", avgRow);
        minRow.Printf("<tr><td>%s</td>", minRow);
        maxRow.Printf("<tr><td>%s</td>", maxRow);
    }
    else
    {
        mdnRow.Printf("%s\t", mdnRow);
        avgRow.Printf("%s\t", avgRow);
        minRow.Printf("%s\t", minRow);
        maxRow.Printf("%s\t", maxRow);
    }

    for ( size_t f = 0; f < m_fileNames.size(); ++f )
    {
        for ( size_t s = 0; s < m_sizes.size(); ++s )
        {
            mdnRow += wxString::Format(asHTML ? valueFormatHTML : valueFormatTSV,
                statsNano[f][s].mdn, statsLuna[f][s].mdn);
            avgRow += wxString::Format(asHTML ? valueFormatHTML : valueFormatTSV,
                statsNano[f][s].avg, statsLuna[f][s].avg);
            minRow += wxString::Format(asHTML ? valueFormatHTML : valueFormatTSV,
                statsNano[f][s].min, statsLuna[f][s].min);
            maxRow += wxString::Format(asHTML ? valueFormatHTML : valueFormatTSV,
                statsNano[f][s].max, statsLuna[f][s].max);
        }
    }
    if ( asHTML )
    {
        mdnRow += "</tr>";
        avgRow += "</tr>";
        minRow += "</tr>";
        maxRow += "</tr>";
    }
    else // extra tabs
    {
        mdnRow.RemoveLast();
        avgRow.RemoveLast();
        minRow.RemoveLast();
        maxRow.RemoveLast();
    }

    result.push_back(mdnRow);
    result.push_back(avgRow);
    result.push_back(minRow);
    result.push_back(maxRow);

    if ( asHTML )
    {
        result.push_back("</tfoot></table>\n");
        result.push_back("</body></html>");
    }

    for ( const auto& r : result )
        reportText += r + "\n";
}

wxTestSVGRasterizationBenchmark::Stats wxTestSVGRasterizationBenchmark::CalcStatsForVectorLong(const VectorLong& data)
{
    VectorLong dataSorted(data);
    Stats      stats;
    wxLongLong sum = 0;

    std::sort(dataSorted.begin(), dataSorted.end());

    stats.min = dataSorted[0];
    stats.max = dataSorted[dataSorted.size() - 1];

    stats.mdn = dataSorted[dataSorted.size() / 2];
    if ( !(data.size() % 2) ) // even number
        stats.mdn = (stats.mdn + dataSorted[(dataSorted.size() / 2) - 1]) / 2;

    sum = std::accumulate(dataSorted.begin(), dataSorted.end(), sum);
    stats.avg = ( sum / dataSorted.size() ).ToLong();

    return stats;
}

// ============================================================================
// wxTestSVGBenchmarkReportFrame
// ============================================================================

wxTestSVGBenchmarkReportFrame::wxTestSVGBenchmarkReportFrame(wxWindow* parent,
                    const wxString& dirName,
                    const wxString& report,
                    const wxString& detailedReport)
    : wxFrame(parent, wxID_ANY, "Benchmark Report"),
      m_dirName(dirName), m_report(report), m_detailedReport(detailedReport)
{
    SetIcon(wxICON(wxICON_AAA)); // from wx.rc
    m_defaultName = "wxTestSVG Benchmark - " + dirName.AfterLast(wxFileName::GetPathSeparator());

    wxMenu* menuFile = new wxMenu;

    menuFile->Append(wxID_SAVEAS);
    menuFile->Append(ID_SAVE_DETAILED, "Save &Detailed Report As...");

    Bind(wxEVT_MENU, &wxTestSVGBenchmarkReportFrame::OnSaveReport, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &wxTestSVGBenchmarkReportFrame::OnSaveDetailedReport, this, ID_SAVE_DETAILED);

    wxMenuBar* menuBar = new wxMenuBar();

    menuBar->Append(menuFile, "&Report");
    SetMenuBar(menuBar);

    wxWebView* webView = wxWebView::New(this, wxID_ANY);
    webView->SetPage(report, wxWebViewDefaultURLStr);
    webView->SetFocus();

    SetMinClientSize(FromDIP(wxSize(800, 600)));
    Show();
}

void wxTestSVGBenchmarkReportFrame::OnSaveReport(wxCommandEvent&)
{
    const wxString fileName = wxFileSelector("Select file name",
        m_dirName, m_defaultName, "html", "HTML files (*.html)|*.html",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if ( fileName.empty() )
        return;

    WriteHTMLReport(fileName, m_report);
}

void wxTestSVGBenchmarkReportFrame::OnSaveDetailedReport(wxCommandEvent&)
{
    const wxString fileName = wxFileSelector("Select file name",
        m_dirName, m_defaultName + "_details", "html", "HTML files (*.html)|*.html",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if ( fileName.empty() )
        return;

    WriteHTMLReport(fileName, m_detailedReport);
}

bool wxTestSVGBenchmarkReportFrame::WriteHTMLReport(const wxString& fileName, const wxString& reportText)
{
    wxFFile reportFile(fileName, "w");

    if ( !reportFile.IsOpened() )
        return false;

    return reportFile.Write(reportText, wxConvUTF8);
}
