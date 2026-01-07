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

# MDK SDK Setup
set(MDK_DIR "${CMAKE_SOURCE_DIR}/mdk-sdk")
include(${MDK_DIR}/lib/cmake/FindMDK.cmake)

# ONNX Runtime Setup
set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/libs/onnxruntime")
include_directories(${ONNXRUNTIME_DIR}/include)
link_directories(${ONNXRUNTIME_DIR}/lib)

# Dahua SDK Setup
set(DAHUA_DIR "${CMAKE_SOURCE_DIR}/libs/dahua")
include_directories(${DAHUA_DIR}/include/Common)
link_directories(${DAHUA_DIR}/lib/Win64)

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
    src/backend/MdkPlayer.cpp
    src/backend/MdkPlayer.h
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
)

# Mark the localization helper as a QML singleton so it can be imported from the module
set_source_files_properties(src/ui/I18n.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

set(QML_RESOURCES
    src/ui/fonts/MaterialIcons-Regular.ttf
)

add_executable(appOpenIPC-Dashboard
    WIN32
    ${SOURCES}
    ${APP_ICON}
)

# Resource file for QML
qt_add_qml_module(appOpenIPC-Dashboard
    URI OpenIPC
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCES ${QML_RESOURCES}
)

target_link_libraries(appOpenIPC-Dashboard
    PRIVATE Qt6::Quick Qt6::Core Qt6::Network Qt6::Concurrent mdk onnxruntime dhnetsdk
)

if (WIN32)
    target_link_libraries(appOpenIPC-Dashboard PRIVATE psapi)
endif()

# Copy MDK DLLs to build directory
add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${MDK_DIR}/bin/x64 $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
)

# Copy ONNX Runtime DLL to build directory
add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${ONNXRUNTIME_DIR}/lib/onnxruntime.dll $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
)

# Copy Dahua SDK DLLs to build directory
file(GLOB DAHUA_DLLS "${DAHUA_DIR}/bin/*.dll")
add_custom_command(TARGET appOpenIPC-Dashboard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DAHUA_DLLS}
    $<TARGET_FILE_DIR:appOpenIPC-Dashboard>
)

# Copy OpenIPC QML module to build directory (for deployment)
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
