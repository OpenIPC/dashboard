cmake_minimum_required(VERSION 3.16)

project(OpenIPC-Dashboard VERSION 0.2.3 LANGUAGES CXX)

# CMake 4 removes compatibility with projects that declare very old minimum
# versions. Some bundled dependencies still do that, so keep their policy floor
# explicit while they are consumed through FetchContent.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy compatibility for bundled dependencies" FORCE)

# Allow overriding app version from CI (e.g., Git tag), fallback to project version.
if(DEFINED APP_VERSION AND NOT APP_VERSION STREQUAL "")
    set(APP_VERSION_STR "${APP_VERSION}")
else()
    set(APP_VERSION_STR "${PROJECT_VERSION}")
endif()

set(APP_AUTHOR_STR "Rinat Ibragimov")
string(TIMESTAMP APP_BUILD_YEAR "%Y")

configure_file(src/config.h.in config.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network Concurrent Multimedia Sql Test)

# Force native rcc executable to avoid picking up python-based rcc from VS Code extensions
if(WIN32 AND TARGET Qt6::rcc)
    get_target_property(QT_RCC_EXECUTABLE Qt6::rcc IMPORTED_LOCATION)
    if(QT_RCC_EXECUTABLE)
        set(CMAKE_AUTORCC_EXECUTABLE "${QT_RCC_EXECUTABLE}")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32 AND MINGW)
    add_link_options("-Wl,--disable-high-entropy-va" "-Wl,--disable-dynamicbase")
endif()

# GStreamer Integration
if(WIN32)
    # 1. Check -D GStreamer_ROOT
    if(GStreamer_ROOT)
        set(GST_ROOT ${GStreamer_ROOT})
    endif()
    
    # 2. Check Environment Variables
    if(NOT GST_ROOT)
        if(DEFINED ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64})
            set(GST_ROOT $ENV{GSTREAMER_1_0_ROOT_MSVC_X86_64})
        elseif(DEFINED ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64})
            set(GST_ROOT $ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64})
        endif()
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
        if(NOT EXISTS "${GST_ROOT}/include/gstreamer-1.0/gst/gst.h")
            message(FATAL_ERROR "GStreamer headers not found at ${GST_ROOT}/include/gstreamer-1.0/gst/gst.h. Pass the devel package root through -DGStreamer_ROOT or GSTREAMER_1_0_ROOT_MINGW_X86_64.")
        endif()
        
        set(GST_INCLUDE_DIRS
            "${GST_ROOT}/include/gstreamer-1.0"
            "${GST_ROOT}/include/glib-2.0"
            "${GST_ROOT}/lib/glib-2.0/include"
            "${GST_ROOT}/lib/gstreamer-1.0/include"
        )
        include_directories(${GST_INCLUDE_DIRS})
        link_directories("${GST_ROOT}/lib")
        set(GST_LIBRARIES gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 gstvideo-1.0 gstbase-1.0 gstpbutils-1.0)
        set(GST_RUNTIME_DLLS
            "${GST_ROOT}/bin/libffi-7.dll"
            "${GST_ROOT}/bin/libglib-2.0-0.dll"
            "${GST_ROOT}/bin/libgmodule-2.0-0.dll"
            "${GST_ROOT}/bin/libgobject-2.0-0.dll"
            "${GST_ROOT}/bin/libgstapp-1.0-0.dll"
            "${GST_ROOT}/bin/libgstbase-1.0-0.dll"
            "${GST_ROOT}/bin/libgstreamer-1.0-0.dll"
            "${GST_ROOT}/bin/libintl-8.dll"
            "${GST_ROOT}/bin/libpcre2-8-0.dll"
        )
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

# libssh Setup via FetchContent
include(FetchContent)
FetchContent_Declare(
    libssh
    GIT_REPOSITORY https://git.libssh.org/projects/libssh.git
    GIT_TAG        libssh-0.10.6
)
# Disable building tests and examples for libssh
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build bundled dependencies statically" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "Disable libssh examples" FORCE)
set(WITH_TESTING OFF CACHE BOOL "Disable libssh testing" FORCE)
set(WITH_SERVER OFF CACHE BOOL "Disable libssh server" FORCE)
set(WITH_ZLIB OFF CACHE BOOL "Disable optional libssh zlib compression" FORCE)
set(WITH_GCRYPT OFF CACHE BOOL "Disable optional libssh GCrypt backend" FORCE)
set(WITH_MBEDTLS OFF CACHE BOOL "Disable optional libssh mbedTLS backend" FORCE)
set(WITH_OPENSSL ON CACHE BOOL "Use OpenSSL for libssh crypto" FORCE)
FetchContent_MakeAvailable(libssh)

# QtKeychain Setup via FetchContent
FetchContent_Declare(
    qtkeychain
    GIT_REPOSITORY https://github.com/frankosterfeld/qtkeychain.git
    GIT_TAG        0.14.2
)
set(BUILD_WITH_QT6 ON CACHE BOOL "Build qtkeychain with Qt6" FORCE)
set(BUILD_TRANSLATIONS OFF CACHE BOOL "Disable qtkeychain translations" FORCE)
set(BUILD_TEST_APPLICATION OFF CACHE BOOL "Disable qtkeychain test app" FORCE)
FetchContent_MakeAvailable(qtkeychain)

set(BUNDLED_RUNTIME_TARGETS)
foreach(_runtime_target ssh qt6keychain)
    if(TARGET ${_runtime_target})
        get_target_property(_runtime_target_type ${_runtime_target} TYPE)
        if(_runtime_target_type STREQUAL "SHARED_LIBRARY" OR _runtime_target_type STREQUAL "MODULE_LIBRARY")
            list(APPEND BUNDLED_RUNTIME_TARGETS $<TARGET_FILE:${_runtime_target}>)
        endif()
    endif()
endforeach()

set(SOURCES
    src/main.cpp
    src/backend/PathUtils.cpp
    src/backend/PathUtils.h
    src/backend/CamexController.cpp
    src/backend/CamexController.h
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
    src/ui/Theme.qml
    src/ui/Main.qml
    src/ui/SettingsDialog.qml
    src/ui/ModulesSettingsPanel.qml
    src/ui/StyledComboBox.qml
    src/ui/GridCell.qml
    src/ui/SidebarIcon.qml
    src/ui/CameraSearchDialog.qml
    src/ui/AddCameraDialog.qml
    src/ui/I18n.qml
    src/ui/AnalyticsView.qml
    src/ui/CamexView.qml
    src/ui/analytics/FaceSnapshotsPanel.qml
    src/ui/analytics/ObjectCounterPanel.qml
    src/ui/analytics/LicensePlatePanel.qml
    src/ui/analytics/AnalyticsOverviewPanel.qml
    src/ui/analytics/AnalyticsCamerasPanel.qml
    src/ui/analytics/AnalyticsModulesPanel.qml
    src/ui/analytics/EventsPanel.qml
    src/ui/analytics/RulesPanel.qml
    src/ui/analytics/SnapshotBrowser.qml
    src/ui/analytics/ImageViewerWindow.qml
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
set_source_files_properties(src/ui/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(QML_RESOURCES
    src/ui/fonts/MaterialIcons-Regular.ttf
)

add_executable(openipc-askpass
    src/backend/askpass_helper.cpp
)

add_executable(appOpenIPC-Dashboard
    ${SOURCES}
    ${APP_ICON}
)
target_include_directories(appOpenIPC-Dashboard PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GST_INCLUDE_DIRS}
    ${qtkeychain_SOURCE_DIR}
    ${qtkeychain_BINARY_DIR}
    ${libssh_SOURCE_DIR}/include
    ${libssh_BINARY_DIR}/include
)

# On Windows we want a GUI app (WinMain entry), on Linux standard main is fine
if(WIN32)
    set_target_properties(appOpenIPC-Dashboard PROPERTIES WIN32_EXECUTABLE TRUE)
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
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network Qt6::Concurrent Qt6::Sql onnxruntime dhnetsdk ${GST_LIBRARIES} ssh qt6keychain
)

if (UNIX AND NOT APPLE)
    # Explicitly link pthread if needed, though Qt usually pulls it in
    target_link_libraries(appOpenIPC-Dashboard PRIVATE pthread)
endif()

# Explicitly add GStreamer includes to the target property so AUTOMOC sees gst/gst.h.
if(GST_INCLUDE_DIRS)
    target_include_directories(appOpenIPC-Dashboard PRIVATE ${GST_INCLUDE_DIRS})
endif()


# Copy ONNX Runtime DLL to build directory
if(WIN32)
    # Prefer DirectML-enabled runtime if present in reference/onnxruntime_directml-*/onnxruntime/capi
    file(GLOB ORT_DML_DIRS "${CMAKE_SOURCE_DIR}/reference/onnxruntime_directml-*/onnxruntime/capi")
    if(ORT_DML_DIRS)
        list(GET ORT_DML_DIRS 0 ORT_DML_DIR)
    endif()

    if(DEFINED ORT_DML_DIR AND EXISTS "${ORT_DML_DIR}/onnxruntime.dll")
        set(ORT_RUNTIME_DIR "${ORT_DML_DIR}")
    else()
        set(ORT_RUNTIME_DIR "${ONNXRUNTIME_DIR}/lib")
    endif()

    set(ORT_DLLS "${ORT_RUNTIME_DIR}/onnxruntime.dll")
    if(EXISTS "${ORT_RUNTIME_DIR}/onnxruntime_providers_shared.dll")
        list(APPEND ORT_DLLS "${ORT_RUNTIME_DIR}/onnxruntime_providers_shared.dll")
    endif()
    if(EXISTS "${ORT_RUNTIME_DIR}/onnxruntime_providers_directml.dll")
        list(APPEND ORT_DLLS "${ORT_RUNTIME_DIR}/onnxruntime_providers_directml.dll")
    endif()
    if(EXISTS "${ORT_RUNTIME_DIR}/DirectML.dll")
        list(APPEND ORT_DLLS "${ORT_RUNTIME_DIR}/DirectML.dll")
    endif()

    add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ORT_DLLS} $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
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

add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:openipc-askpass>
    $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
)

# Copy libssh and qtkeychain DLLs to build directory
if(WIN32)
    if(BUNDLED_RUNTIME_TARGETS)
        add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${BUNDLED_RUNTIME_TARGETS}
            $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
        )
    endif()

    if(GST_RUNTIME_DLLS)
        add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${GST_RUNTIME_DLLS}
                $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
            COMMENT "Deploying GStreamer runtime dependencies"
        )
    endif()
endif()

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

if(WIN32)
    set(APP_EXTRA_RUNTIME_DLLS)

    get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(_runtime_dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        if(EXISTS "${COMPILER_BIN_DIR}/${_runtime_dll}")
            list(APPEND APP_EXTRA_RUNTIME_DLLS "${COMPILER_BIN_DIR}/${_runtime_dll}")
        endif()
    endforeach()

    if(OPENSSL_INCLUDE_DIR)
        get_filename_component(_OPENIPC_OPENSSL_ROOT "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
        foreach(_openssl_dll libcrypto-3-x64.dll libssl-3-x64.dll)
            if(EXISTS "${_OPENIPC_OPENSSL_ROOT}/bin/${_openssl_dll}")
                list(APPEND APP_EXTRA_RUNTIME_DLLS "${_OPENIPC_OPENSSL_ROOT}/bin/${_openssl_dll}")
            endif()
        endforeach()
    endif()

    if(APP_EXTRA_RUNTIME_DLLS)
        add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${APP_EXTRA_RUNTIME_DLLS}
                $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
            COMMENT "Deploying compiler and OpenSSL runtime dependencies"
        )
    endif()
endif()

if(BUILD_TESTING)
    add_executable(user_manager_tests
        tests/UserManagerTests.cpp
        src/backend/UserManager.cpp
        src/backend/UserManager.h
    )

    target_include_directories(user_manager_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
    )

    target_link_libraries(user_manager_tests
        PRIVATE Qt6::Core Qt6::Test
    )

    add_test(NAME user_manager_tests COMMAND user_manager_tests)

    add_executable(path_utils_tests
        tests/PathUtilsTests.cpp
        src/backend/PathUtils.cpp
        src/backend/PathUtils.h
    )

    target_include_directories(path_utils_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
    )

    target_link_libraries(path_utils_tests
        PRIVATE Qt6::Core Qt6::Test
    )

    add_test(NAME path_utils_tests COMMAND path_utils_tests)

    if(WIN32)
        set_tests_properties(user_manager_tests PROPERTIES
            ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        )
        set_tests_properties(path_utils_tests PROPERTIES
            ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
        )
    endif()
endif()
