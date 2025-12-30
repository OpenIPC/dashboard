@echo off
setlocal enableextensions enabledelayedexpansion
path %SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SystemRoot%\System32\WindowsPowerShell\v1.0\

:: Unattended install flag. When set, the script will not require user input.
set unattended=no
if "%1"=="/u" set unattended=yes

:: win7 sp1(msdn) may has no mft, but dxva works
set decs=MFT:d3d=11,D3D11,DXVA,CUDA,hap,FFmpeg,dav1d

set ARCH=%PROCESSOR_ARCHITECTURE%
if [%ARCH%] == [AMD64] set ARCH=x64
:: reg add "HKEY_CLASSES_ROOT\*\shell\Open with MDK D3D11\command" /ve /t REG_SZ /d """"%~dp0bin\glfwplay.exe""" """-logfile""" """%~dp0glfwplay.log""" """-colorspace""" """auto""" """-c:v""" """MFT:d3d=11:pool=1,D3D11,FFmpeg""" """-gl""" """%%1"""" /f
:: d3d11 may be not available on win8
set glfwplay_path=%~dp0bin\%ARCH%\glfwplay.exe
:: Get mpv-document.ico location
:: set icon_path=%~dp0libmdk.ico

goto :register

:checkCrt
:: vcruntime140_1.dll is required by x64 for now
if not EXIST "C:\Windows\System32\vcruntime140_1.dll" (
@echo downloading vc and urct runtime %ARCH%...
cscript /nologo %~dp0\get.js https://aka.ms/vs/16/release/VC_redist.%ARCH%.exe vc_redist.%ARCH%.exe
@echo installing vc and urct runtime %ARCH%...
vc_redist.%ARCH%.exe /install /passive
)
call %glfwplay_path%

echo.
echo Installed successfully
echo Default Programs control panel.
echo.
if [%unattended%] == [yes] exit 0
<nul set /p =Press any key to open the Default Programs control panel . . .
pause >nul
control /name Microsoft.DefaultPrograms
echo.
pause >nul
goto :EOF



:register

for /f "delims=" %%i in ('wmic DesktopMonitor get ScreenWidth^,ScreenHeight /value ^| findstr "="') do set /a %%~i
if [%ScreenWidth%] == [] set ScreenWidth=1920
if [%ScreenHeight%] == [] set ScreenHeight=1080
:: Command line arguments to use when launching glfwplay from a file association
set glfwplay_args=-plugins mdk-braw:mdk-r3d -logfile %~dp0glfwplay.log -colorspace auto -c:v %decs%,BRAW:scale=%ScreenWidth%x%ScreenHeight%,R3D:scale=%ScreenWidth%x%ScreenHeight% -d3d11


call :reg add "HKEY_CURRENT_USER\SOFTWARE\Classes\*\shell\Open with MDK\command" /ve /t REG_SZ /d "\"%glfwplay_path%\" %glfwplay_args% -- \"%%%%L\"" /f

powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%ProgramData%\Microsoft\Windows\Start Menu\Programs\glfwplay.lnk');$s.TargetPath='%glfwplay_path%';$s.Save()"


:: Make sure the script is running as admin
call :ensure_admin

:: Register glfwplay.exe under the "App Paths" key, so it can be found by
:: ShellExecute, the run command, the start menu, etc.
set app_paths_key=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\glfwplay.exe
call :reg add "%app_paths_key%" /d "%glfwplay_path%" /f
call :reg add "%app_paths_key%" /v "UseUrl" /t REG_DWORD /d 1 /f

:: Register glfwplay.exe under the "Applications" key to add some default verbs for
:: when glfwplay is used from the "Open with" menu
set classes_root_key=HKLM\SOFTWARE\Classes
set app_key=%classes_root_key%\Applications\glfwplay.exe
call :reg add "%app_key%" /v "FriendlyAppName" /d "glfwplay" /f
call :add_verbs "%app_key%"

:: Add glfwplay to the "Open with" list for all video and audio file types
call :reg add "%classes_root_key%\SystemFileAssociations\video\OpenWithList\glfwplay.exe" /d "" /f
call :reg add "%classes_root_key%\SystemFileAssociations\audio\OpenWithList\glfwplay.exe" /d "" /f

:: Add a capabilities key for glfwplay, which is registered later on for use in the
:: "Default Programs" control panel
set capabilities_key=HKLM\SOFTWARE\Clients\Media\glfwplay\Capabilities
call :reg add "%capabilities_key%" /v "ApplicationName" /d "glfwplay" /f
call :reg add "%capabilities_key%" /v "ApplicationDescription" /d "libmdk glfwplay player" /f


:: Add file types
set supported_types_key=%app_key%\SupportedTypes
set file_associations_key=%capabilities_key%\FileAssociations
:: Image formats
call :add_type "image/DNG"                        "image" "Digital Negative"           ".dng"
:: DVD/Blu-ray audio formats
call :add_type "audio/ac3"                        "audio" "AC-3 Audio"                 ".ac3" ".a52"
call :add_type "audio/eac3"                       "audio" "E-AC-3 Audio"               ".eac3"
call :add_type "audio/vnd.dolby.mlp"              "audio" "MLP Audio"                  ".mlp"
call :add_type "audio/vnd.dts"                    "audio" "DTS Audio"                  ".dts"
call :add_type "audio/vnd.dts.hd"                 "audio" "DTS-HD Audio"               ".dts-hd" ".dtshd"
call :add_type ""                                 "audio" "TrueHD Audio"               ".true-hd" ".thd" ".truehd" ".thd+ac3"
call :add_type ""                                 "audio" "True Audio"                 ".tta"
:: Uncompressed formats
call :add_type ""                                 "audio" "PCM Audio"                  ".pcm"
call :add_type "audio/wav"                        "audio" "Wave Audio"                 ".wav"
call :add_type "audio/aiff"                       "audio" "AIFF Audio"                 ".aiff" ".aif" ".aifc"
call :add_type "audio/amr"                        "audio" "AMR Audio"                  ".amr"
call :add_type "audio/amr-wb"                     "audio" "AMR-WB Audio"               ".awb"
call :add_type "audio/basic"                      "audio" "AU Audio"                   ".au" ".snd"
call :add_type ""                                 "audio" "Linear PCM Audio"           ".lpcm"
call :add_type ""                                 "video" "Raw YUV Video"              ".yuv"
call :add_type ""                                 "video" "YUV4MPEG2 Video"            ".y4m"
:: Free lossless formats
call :add_type "audio/x-ape"                      "audio" "Monkey's Audio"             ".ape"
call :add_type "audio/x-wavpack"                  "audio" "WavPack Audio"              ".wv"
call :add_type "audio/x-shorten"                  "audio" "Shorten Audio"              ".shn"
:: MPEG formats
call :add_type "video/vnd.dlna.mpeg-tts"          "video" "MPEG-2 Transport Stream"    ".m2ts" ".m2t" ".mts" ".mtv" ".ts" ".tsv" ".tsa" ".tts" ".trp"
call :add_type "audio/vnd.dlna.adts"              "audio" "ADTS Audio"                 ".adts" ".adt"
call :add_type "audio/mpeg"                       "audio" "MPEG Audio"                 ".mpa" ".m1a" ".m2a" ".mp1" ".mp2"
call :add_type "audio/mpeg"                       "audio" "MP3 Audio"                  ".mp3"
call :add_type "video/mpeg"                       "video" "MPEG Video"                 ".mpeg" ".mpg" ".mpe" ".mpeg2" ".m1v" ".m2v" ".mp2v" ".mpv" ".mpv2" ".mod" ".tod"
call :add_type "video/dvd"                        "video" "Video Object"               ".vob" ".vro"
call :add_type ""                                 "video" "Enhanced VOB"               ".evob" ".evo"
call :add_type "video/mp4"                        "video" "MPEG-4 Video"               ".mpeg4" ".m4v" ".mp4" ".mp4v" ".mpg4"
call :add_type "audio/mp4"                        "audio" "MPEG-4 Audio"               ".m4a"
call :add_type "audio/aac"                        "audio" "Raw AAC Audio"              ".aac"
call :add_type ""                                 "video" "Raw H.264/AVC Video"        ".h264" ".avc" ".x264" ".264"
call :add_type ""                                 "video" "Raw H.265/HEVC Video"       ".hevc" ".h265" ".x265" ".265"
:: Xiph formats
call :add_type "audio/flac"                       "audio" "FLAC Audio"                 ".flac"
call :add_type "audio/ogg"                        "audio" "Ogg Audio"                  ".oga" ".ogg"
call :add_type "audio/ogg"                        "audio" "Opus Audio"                 ".opus"
call :add_type "audio/ogg"                        "audio" "Speex Audio"                ".spx"
call :add_type "video/ogg"                        "video" "Ogg Video"                  ".ogv" ".ogm"
call :add_type "application/ogg"                  "video" "Ogg Video"                  ".ogx"
:: Matroska formats
call :add_type "video/x-matroska"                 "video" "Matroska Video"             ".mkv"
call :add_type "video/x-matroska"                 "video" "Matroska 3D Video"          ".mk3d"
call :add_type "audio/x-matroska"                 "audio" "Matroska Audio"             ".mka"
call :add_type "video/webm"                       "video" "WebM Video"                 ".webm"
call :add_type "audio/webm"                       "audio" "WebM Audio"                 ".weba"
:: Misc formats
call :add_type "video/avi"                        "video" "Video Clip"                 ".avi" ".vfw"
call :add_type ""                                 "video" "DivX Video"                 ".divx"
call :add_type ""                                 "video" "3ivx Video"                 ".3iv"
call :add_type ""                                 "video" "XVID Video"                 ".xvid"
call :add_type ""                                 "video" "NUT Video"                  ".nut"
call :add_type "video/flc"                        "video" "FLIC Video"                 ".flic" ".fli" ".flc"
call :add_type ""                                 "video" "Nullsoft Streaming Video"   ".nsv"
call :add_type "application/gxf"                  "video" "General Exchange Format"    ".gxf"
call :add_type "application/mxf"                  "video" "Material Exchange Format"   ".mxf"
call :add_type "application/x-braw-clip"          "video" "Blackmagic RAW Clip"        ".braw"
call :add_type "application/x-r3d-clip"          "video"  "RED R3D RAW Clip"           ".r3d"
:: Windows Media formats
call :add_type "audio/x-ms-wma"                   "audio" "Windows Media Audio"        ".wma"
call :add_type "video/x-ms-wm"                    "video" "Windows Media Video"        ".wm"
call :add_type "video/x-ms-wmv"                   "video" "Windows Media Video"        ".wmv"
call :add_type "video/x-ms-asf"                   "video" "Windows Media Video"        ".asf"
call :add_type ""                                 "video" "Microsoft Recorded TV Show" ".dvr-ms" ".dvr"
call :add_type ""                                 "video" "Windows Recorded TV Show"   ".wtv"
:: DV formats
call :add_type ""                                 "video" "DV Video"                   ".dv" ".hdv"
:: Flash Video formats
call :add_type "video/x-flv"                      "video" "Flash Video"                ".flv"
call :add_type "video/mp4"                        "video" "Flash Video"                ".f4v"
call :add_type "audio/mp4"                        "audio" "Flash Audio"                ".f4a"
:: QuickTime formats
call :add_type "video/quicktime"                  "video" "QuickTime Video"            ".qt" ".mov"
call :add_type "video/quicktime"                  "video" "QuickTime HD Video"         ".hdmov"
:: Real Media formats
call :add_type "application/vnd.rn-realmedia"     "video" "Real Media Video"           ".rm"
call :add_type "application/vnd.rn-realmedia-vbr" "video" "Real Media Video"           ".rmvb"
call :add_type "audio/vnd.rn-realaudio"           "audio" "Real Media Audio"           ".ra" ".ram"
:: 3GPP formats
call :add_type "audio/3gpp"                       "audio" "3GPP Audio"                 ".3ga"
call :add_type "audio/3gpp2"                      "audio" "3GPP Audio"                 ".3ga2"
call :add_type "video/3gpp"                       "video" "3GPP Video"                 ".3gpp" ".3gp"
call :add_type "video/3gpp2"                      "video" "3GPP Video"                 ".3gp2" ".3g2"
:: Video game formats
call :add_type ""                                 "audio" "AY Audio"                   ".ay"
call :add_type ""                                 "audio" "GBS Audio"                  ".gbs"
call :add_type ""                                 "audio" "GYM Audio"                  ".gym"
call :add_type ""                                 "audio" "HES Audio"                  ".hes"
call :add_type ""                                 "audio" "KSS Audio"                  ".kss"
call :add_type ""                                 "audio" "NSF Audio"                  ".nsf"
call :add_type ""                                 "audio" "NSFE Audio"                 ".nsfe"
call :add_type ""                                 "audio" "SAP Audio"                  ".sap"
call :add_type ""                                 "audio" "SPC Audio"                  ".spc"
call :add_type ""                                 "audio" "VGM Audio"                  ".vgm"
call :add_type ""                                 "audio" "VGZ Audio"                  ".vgz"
:: Playlist formats
call :add_type "audio/x-mpegurl"                  "audio" "M3U Playlist"               ".m3u" ".m3u8"
call :add_type "audio/x-scpls"                    "audio" "PLS Playlist"               ".pls"
call :add_type ""                                 "audio" "CUE Sheet"                  ".cue"
:: Register "Default Programs" entry
call :reg add "HKLM\SOFTWARE\RegisteredApplications" /v "glfwplay" /d "SOFTWARE\Clients\Media\glfwplay\Capabilities" /f

goto :checkCrt

exit 0

:ensure_admin
	:: 'openfiles' is just a commmand that is present on all supported Windows
	:: versions, requires admin privileges and has no side effects, see:
	:: https://stackoverflow.com/questions/4051883/batch-script-how-to-check-for-admin-rights
	openfiles >nul 2>&1
	if errorlevel 1 (
		echo This batch script requires administrator privileges. Right-click on
		echo install.bat and select "Run as administrator".
		call :die
	)
	goto :EOF

:die
	if not [%1] == [] echo %~1
	if [%unattended%] == [yes] exit 1
	pause
	exit 1

:reg
	:: Wrap the reg command to check for errors
	>nul reg %*
	if errorlevel 1 set error=yes
	if [%error%] == [yes] echo Error in command: reg %*
	if [%error%] == [yes] call :die
	goto :EOF

:reg_set_opt
	:: Set a value in the registry if it doesn't already exist
	set key=%~1
	set value=%~2
	set data=%~3

	reg query "%key%" /v "%value%" >nul 2>&1
	if errorlevel 1 call :reg add "%key%" /v "%value%" /d "%data%"
	goto :EOF

:add_verbs
	set key=%~1

	:: Set the default verb to "play"
	call :reg add "%key%\shell" /d "play" /f

	:: Hide the "open" verb from the context menu, since it's the same as "play"
	call :reg add "%key%\shell\open" /v "LegacyDisable" /f

	:: Set open command
	call :reg add "%key%\shell\open\command" /d "\"%glfwplay_path%\" %glfwplay_args% -- \"%%%%L\"" /f

	:: Add "play" verb
	call :reg add "%key%\shell\play" /d "&Play" /f
	call :reg add "%key%\shell\play\command" /d "\"%glfwplay_path%\" %glfwplay_args% -- \"%%%%L\"" /f

	goto :EOF


:add_progid
	set prog_id=%~1
	set friendly_name=%~2

	:: Add ProgId, edit flags are FTA_OpenIsSafe
	set prog_id_key=%classes_root_key%\%prog_id%
	call :reg add "%prog_id_key%" /d "%friendly_name%" /f
	call :reg add "%prog_id_key%" /v "EditFlags" /t REG_DWORD /d 65536 /f
	call :reg add "%prog_id_key%" /v "FriendlyTypeName" /d "%friendly_name%" /f
	call :reg add "%prog_id_key%\DefaultIcon" /d "%icon_path%" /f
	call :add_verbs "%prog_id_key%"

	goto :EOF

:update_extension
	set extension=%~1
	set prog_id=%~2
	set mime_type=%~3
	set perceived_type=%~4

	:: Add information about the file extension, if not already present
	set extension_key=%classes_root_key%\%extension%
	if not [%mime_type%] == [] call :reg_set_opt "%extension_key%" "Content Type" "%mime_type%"
	if not [%perceived_type%] == [] call :reg_set_opt "%extension_key%" "PerceivedType" "%perceived_type%"
	call :reg add "%extension_key%\OpenWithProgIds" /v "%prog_id%" /f

	:: Add type to SupportedTypes
	call :reg add "%supported_types_key%" /v "%extension%" /f

	:: Add type to the Default Programs control panel
	call :reg add "%file_associations_key%" /v "%extension%" /d "%prog_id%" /f

	goto :EOF

:add_type
	set mime_type=%~1
	set perceived_type=%~2
	set friendly_name=%~3
	set extension=%~4

	echo Adding "%extension%" file type

	:: Add ProgId
	set prog_id=glfwplay.libmdk%extension%
	call :add_progid "%prog_id%" "%friendly_name%"

	:: Add extensions
	:extension_loop
		call :update_extension "%extension%" "%prog_id%" "%mime_type%" "%perceived_type%"

		:: Trailing parameters are additional extensions
		shift /4
		set extension=%~4
		if not [%extension%] == [] goto extension_loop

	goto :EOF
