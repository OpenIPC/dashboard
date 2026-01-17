cmake_minimum_required(VERSION 3.16)

project(OpenIPC-Dashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network Concurrent Multimedia)

# Force native rcc executable to avoid picking up python-based rcc from VS Code extensions
if(WIN32 AND EXISTS "C:/OpenIPC-Dashboard-Cpp/6.4.2/mingw_64/bin/rcc.exe")
    set(QT_RCC_EXECUTABLE "C:/OpenIPC-Dashboard-Cpp/6.4.2/mingw_64/bin/rcc.exe")
    set(CMAKE_AUTORCC_EXECUTABLE "C:/OpenIPC-Dashboard-Cpp/6.4.2/mingw_64/bin/rcc.exe")
    if(TARGET Qt6::rcc)
        set_target_properties(Qt6::rcc PROPERTIES IMPORTED_LOCATION "C:/OpenIPC-Dashboard-Cpp/6.4.2/mingw_64/bin/rcc.exe")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# libVLC Setup (expect layout like libs/libvlc/sdk/{include,lib,bin})
set(LIBVLC_DIR "${CMAKE_SOURCE_DIR}/libs/libvlc/sdk" CACHE PATH "Path to libVLC SDK root (containing include/ lib/ bin/)")
find_path(LIBVLC_INCLUDE_DIR vlc/vlc.h
    PATHS
        "${LIBVLC_DIR}/include"
        "${LIBVLC_DIR}/win64/include"
        "${LIBVLC_DIR}/sdk/include"
)
find_library(LIBVLC_LIBRARY NAMES libvlc
    PATHS "${LIBVLC_DIR}/lib" "${LIBVLC_DIR}/win64/lib" "${LIBVLC_DIR}/sdk/lib")
find_library(LIBVLCCORE_LIBRARY NAMES libvlccore
    PATHS "${LIBVLC_DIR}/lib" "${LIBVLC_DIR}/win64/lib" "${LIBVLC_DIR}/sdk/lib")

if(NOT LIBVLC_INCLUDE_DIR OR NOT LIBVLC_LIBRARY OR NOT LIBVLCCORE_LIBRARY)
#    if(UNIX AND NOT APPLE)
#        message(STATUS "Trying to find libVLC via pkg-config...")
#        find_package(PkgConfig REQUIRED)
#        pkg_check_modules(LIBVLC REQUIRED libvlc)
#        # Fix for some distros where pkg-config returns include path without vlc/vlc.h suffix
#        find_path(LIBVLC_INCLUDE_DIR vlc/vlc.h PATHS ${LIBVLC_INCLUDE_DIRS})
#        set(LIBVLC_LIBRARY ${LIBVLC_LIBRARIES})
#        set(LIBVLCCORE_LIBRARY "") # PkgConfig usually lumps them or we mostly link against libvlc
#    else()
#        message(FATAL_ERROR "libVLC SDK not found. Set LIBVLC_DIR to the SDK root containing include/ and lib/ (e.g. libs/libvlc/sdk).")
#    endif()
endif()

# include_directories(${LIBVLC_INCLUDE_DIR})

if(WIN32)
    # FORCE Hardcoded GStreamer includes for Windows CI environment
    # bypassing any detection logic to ensure paths are present
    set(GST_FORCE_INCLUDE_DIRS
        "C:/gstreamer/1.0/mingw_x86_64/include/gstreamer-1.0"
        "C:/gstreamer/1.0/mingw_x86_64/include/glib-2.0"
        "C:/gstreamer/1.0/mingw_x86_64/lib/glib-2.0/include"
        "C:/gstreamer/1.0/mingw_x86_64/lib/gstreamer-1.0/include"
    )
    message(STATUS "FORCE-ADDING GStreamer Includes: ${GST_FORCE_INCLUDE_DIRS}")
    include_directories(${GST_FORCE_INCLUDE_DIRS})
endif()

# GStreamer Integration
if(WIN32)
    # 1. Check -D GStreamer_ROOT
    if(GStreamer_ROOT)
        set(GST_ROOT ${GStreamer_ROOT})
    endif()
    
    # 2. Check Environment Variables
    if(NOT GST_ROOT)
        set(GST_ROOT $ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64})
    endif()
    if(NOT GST_ROOT)
        set(GST_ROOT $ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64})
    endif()
    
    # 3. Try standard install locations
    if(NOT GST_ROOT)
        if(EXISTS "C:/gstreamer/1.0/mingw_x86_64")
            set(GST_ROOT "C:/gstreamer/1.0/mingw_x86_64")
        elseif(EXISTS "C:/gstreamer/1.0/msvc_x86_64")
            set(GST_ROOT "C:/gstreamer/1.0/msvc_x86_64")
        endif()
    endif()

    if(GST_ROOT)
        # Convert to forward slashes for CMake consistency
        file(TO_CMAKE_PATH "${GST_ROOT}" GST_ROOT)
        message(STATUS "Found GStreamer Root: ${GST_ROOT}")
        
        set(GST_INCLUDE_DIRS
            "${GST_ROOT}/include/gstreamer-1.0"
            "${GST_ROOT}/include/glib-2.0"
            "${GST_ROOT}/lib/glib-2.0/include"
            "${GST_ROOT}/lib/gstreamer-1.0/include"
        )
        # Force include these for the upcoming target
        include_directories(${GST_INCLUDE_DIRS})
        
        link_directories("${GST_ROOT}/lib")
        set(GST_LIBRARIES gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 gstvideo-1.0 gstbase-1.0 gstpbutils-1.0)
    else()
        # Fallback to pkg-config if manual detection failed
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-base-1.0 gstreamer-app-1.0 gstreamer-video-1.0 gstreamer-pbutils-1.0)
        include_directories(${GST_INCLUDE_DIRS})
        link_directories(${GST_LIBRARY_DIRS})
    endif()
endif()

# Fallback for non-Windows (or if logic above was skipped unexpectedly)
if(NOT WIN32 AND NOT GST_LIBRARIES)
    find_package(PkgConfig REQUIRED) 
    pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 glib-2.0 gobject-2.0)
    include_directories(${GST_INCLUDE_DIRS})
    link_directories(${GST_LIBRARY_DIRS})
    set(GST_LIBRARIES ${GST_LIBRARIES})
endif()

if(NOT GST_LIBRARIES)
    # This block is only reached if logic above failed to set libs
    find_package(PkgConfig REQUIRED) 
    pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0 glib-2.0 gobject-2.0)
    include_directories(${GST_INCLUDE_DIRS})
    link_directories(${GST_LIBRARY_DIRS})
    set(GST_LIBRARIES ${GST_LIBRARIES})
endif()

# Explicitly ensure include headers are available to the target (Fixes moc compilation finding headers)
if(GST_INCLUDE_DIRS)
    include_directories(${GST_INCLUDE_DIRS})
endif()
if(GST_LIBRARY_DIRS)
    link_directories(${GST_LIBRARY_DIRS})
endif()


# ONNX Runtime Setup
if(WIN32)
    set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/libs/onnxruntime")
    include_directories(${ONNXRUNTIME_DIR}/include)
    link_directories(${ONNXRUNTIME_DIR}/lib)
elseif(UNIX)
    # Assume system ONNX Runtime or located in libs/onnxruntime-linux
    # proper checking should be here, but for now we look for headers in standard paths or libs/onnxruntime/include
    set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/libs/onnxruntime")
    if(EXISTS "${ONNXRUNTIME_DIR}/include")
        include_directories(${ONNXRUNTIME_DIR}/include)
    endif()
    # If user has put linux libs here:
    if(EXISTS "${ONNXRUNTIME_DIR}/lib/Linux64/libonnxruntime.so")
        link_directories(${ONNXRUNTIME_DIR}/lib/Linux64)
    elseif(EXISTS "${ONNXRUNTIME_DIR}/lib/libonnxruntime.so")
        link_directories(${ONNXRUNTIME_DIR}/lib)
    endif()
endif()

# Dahua SDK Setup
set(DAHUA_DIR "${CMAKE_SOURCE_DIR}/libs/dahua")
include_directories(${DAHUA_DIR}/include/Common)
if(WIN32)
    link_directories(${DAHUA_DIR}/lib/Win64)
elseif(UNIX)
     # User should place .so files in libs/dahua/lib/Linux64 or similar if they have them
    if(EXISTS "${DAHUA_DIR}/lib/Linux64")
        link_directories(${DAHUA_DIR}/lib/Linux64)
    endif()
endif()

set(SOURCES
    src/main.cpp
    src/backend/SystemController.cpp
    src/backend/SystemController.h
    src/backend/DiscoveryController.cpp
    src/backend/DiscoveryController.h
    src/backend/ArchiveController.cpp
    src/backend/ArchiveController.h
    src/backend/CameraModel.cpp
    src/backend/CameraModel.h
    src/backend/StatusChecker.cpp
    src/backend/StatusChecker.h
    src/backend/gst/GstPlayer.cpp
    src/backend/gst/GstPlayer.h
    src/backend/AnalyticsModel.cpp
    src/backend/AnalyticsModel.h
    src/backend/analytics/AnalyticsEngine.cpp
    src/backend/analytics/AnalyticsEngine.h
    src/backend/analytics/InferenceBackend.h
    src/backend/analytics/YoloDetector.cpp
    src/backend/analytics/YoloDetector.h
    src/backend/UserManager.cpp
    src/backend/UserManager.h
    src/backend/LogModel.cpp
    src/backend/LogModel.h
    src/backend/SshClient.cpp
    src/backend/SshClient.h
    src/backend/PtzController.cpp
    src/backend/PtzController.h
    src/backend/RemoteFsModel.cpp
    src/backend/RemoteFsModel.h
)

set(APP_ICON resources/appicon.rc)

set(QML_FILES
    src/ui/Main.qml
    src/ui/SettingsDialog.qml
    src/ui/ModulesSettingsPanel.qml
    src/ui/GridCell.qml
    src/ui/SidebarIcon.qml
    src/ui/CameraSearchDialog.qml
    src/ui/AddCameraDialog.qml
    src/ui/I18n.qml
    src/ui/AnalyticsView.qml
    src/ui/analytics/FaceSnapshotsPanel.qml
    src/ui/analytics/ObjectCounterPanel.qml
    src/ui/analytics/LicensePlatePanel.qml
    src/ui/UserManagementDialog.qml
    src/ui/AddUserDialog.qml
    src/ui/ChangePasswordDialog.qml
    src/ui/LoginView.qml
    src/ui/DashboardView.qml
    src/ui/AddGroupDialog.qml
    src/ui/LogView.qml
    src/ui/CameraContextMenu.qml
    src/ui/SshTerminalDialog.qml
    src/ui/ArchiveView.qml
    src/ui/PtzControlPanel.qml
    src/ui/StyledScrollBar.qml
    src/ui/FileManagerDialog.qml
)

# Mark the localization helper as a QML singleton so it can be imported from the module
set_source_files_properties(src/ui/I18n.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(QML_RESOURCES
    src/ui/fonts/MaterialIcons-Regular.ttf
)


add_executable(appOpenIPC-Dashboard
    ${SOURCES}
    ${APP_ICON}
)

# On Windows we want a GUI app (WinMain entry), on Linux standard main is fine
if(WIN32)
    # set_target_properties(appOpenIPC-Dashboard PROPERTIES WIN32_EXECUTABLE TRUE)
else()
    # Configure RPATH to look in the application directory for shared libraries
    set_target_properties(appOpenIPC-Dashboard PROPERTIES 
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Resource file for QML
qt_add_qml_module(appOpenIPC-Dashboard
    URI OpenIPC
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCES ${QML_RESOURCES}
)

target_link_libraries(appOpenIPC-Dashboard
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network Qt6::Concurrent onnxruntime dhnetsdk ${GST_LIBRARIES}
)

if (UNIX AND NOT APPLE)
    # Explicitly link pthread if needed, though Qt usually pulls it in
    target_link_libraries(appOpenIPC-Dashboard PRIVATE pthread)
endif()

# Explicitly add GStreamer includes to the target property (CRITICAL for AUTOMOC)
if(WIN32)
    target_include_directories(appOpenIPC-Dashboard PRIVATE ${GST_FORCE_INCLUDE_DIRS})
endif()


# Copy ONNX Runtime DLL to build directory
if(WIN32)
    add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ONNXRUNTIME_DIR}/lib/onnxruntime.dll $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
    )
elseif(UNIX)
    file(GLOB ONNX_SOS "${ONNXRUNTIME_DIR}/lib/Linux64/*.so*")
    if(ONNX_SOS)
        add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ONNX_SOS}
            $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
        )
    endif()
endif()

# Copy Dahua SDK DLLs to build directory
if(WIN32)
    file(GLOB DAHUA_DLLS "${DAHUA_DIR}/bin/*.dll")
    add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DAHUA_DLLS}
        $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
    )
elseif(UNIX)
    file(GLOB DAHUA_SOS "${DAHUA_DIR}/lib/Linux64/*.so")
    if(DAHUA_SOS)
        add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DAHUA_SOS}
            $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
        )
    endif()
endif()



# Copy OpenIPC QML module to build directory (for deployment)
# On Linux we might want to instal this, but for local build copying is fine
add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/OpenIPC $<TARGET_FILE_DIR:appOpenIPC-Dashboard>/OpenIPC
)

# Run windeployqt to deploy Qt dependencies
if(WIN32)
    # Find windeployqt relative to the Qt6::Core library location if possible, or use a hardcoded guess based on environment
    get_target_property(QT_CORE_LIB Qt6::Core LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_CORE_LIB}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
        COMMAND ${WINDEPLOYQT_EXECUTABLE} 
            --qmldir "${CMAKE_SOURCE_DIR}/src/ui"
            --no-translations
            --compiler-runtime
            --force
            "$<TARGET_FILE:appOpenIPC-Dashboard>"
        COMMENT "Deploying Qt dependencies using windeployqt"
    )
endif()
