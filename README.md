About
---------
wxTestSVG2 compares appearance and performance of [NanoSVG](https://github.com/memononen/nanosvg) and [LunaSVG](https://github.com/sammycage/lunasvg),
when used as a rasterization tool for `wxBitmapBundle`.

![wxTestSVG2 Screenshot](wxtestsvg2-screenshot.png?raw=true)

Surprisingly, LunaSVG seems consistently noticeably faster when using on popular
icon sets (Tango, flat-color-icons, fluentui-system-icons, material-design-icons)
when testing at resolutions expected for GUI icons.

Tested only 64-bit release build on Windows with MSVS 2022 and GCC 13.2.

Build Requirements
---------
* CMake v3.24 or newer.
* wxWidgets v3.2.0 or newer.
* LunaSVG is included in the repo (physically, not as a GIT submodule).

Licence
---------
[wxWidgets licence](https://github.com/wxWidgets/wxWidgets/blob/master/docs/licence.txt)