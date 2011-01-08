Tools/libraries needed to build TMS Launcher:

Versions 1.0 - 2.7
- Visual Studio 6.0
- WTL 7.1
- Regex++ from Boost library (version 1.28.0)
- Windows Platform SDK 2003

Versions 3.0 - 3.1
- Visual Studio 7.1
- WTL 7.1
- Regex++ from Boost library (version 1.28.0)
- Windows Platform SDK 2003

Versions 3.2 and higher
- Visual Studio 2005 Express
- WTL 8.1 (build 9127)
- Regex++ from Boost library (version 1.45.0)
- Windows Platform SDK 2003

NOTE:
1) Since Express Edition does not have Resource editor, it is necessary to use
either any external editor such as ResEditor or editor from professional
edition, for example from Visual Studio .NET 2003.

2)
Microsoft ATL (Active Template Library), which includes the header 'atlbase.h' 
is included with Professional editions of Visual Studio.
So you should either integrate ATL into Express or use Professional edition
of Visual Studio 2005.

To integrate ATL into Express, you have to download and install the Professional Trial to a 2nd PC.
Do not attempt to install Professional Trial side-by-side with Express, as that will screw up the 
Express installation.
When the installation is done, navigate to the "C:\Program Files (x86)\Microsoft Visual Studio 8\VC" 
directory (or wherever else you've installed it), and look for the "atlmfc" subfolder. Copy it over 
anywhere to your VC Express machine.
Now open the solution in VC Express, and append the (fully qualified pathnames of) "atlmfc\include" and "atlmfc\lib" 
directories to the "Tools -> Options-> Projects and Solutions -> VC++ Directories" 
"Include files" and "Library files", respectively.
