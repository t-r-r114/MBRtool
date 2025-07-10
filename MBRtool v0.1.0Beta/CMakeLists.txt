cmake_minimum_required(VERSION 3.16)

project(MBRtool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)  # 明确设置 C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所需的 Qt 模块
find_package(Qt6 REQUIRED COMPONENTS Quick Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

# 简化项目设置
qt_standard_project_setup(REQUIRES 6.8)

# 添加所有源文件到可执行目标
qt_add_executable(appMBRtool
    main.cpp
    disassemblymain.cpp
    mbrsave.cpp
    mbrwrite.cpp
    disassemblywindows.cpp
    disassembler.cpp
    disassemblymain.cpp
)

# 添加 QML 模块
qt_add_qml_module(appMBRtool
    URI MBRtool
    VERSION 1.0
    QML_FILES
        Main.qml
        disassembly.qml
        SOURCES
        SOURCES FileReader.h FileReader.cpp
)

# 设置目标属性
set_target_properties(appMBRtool PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 链接 Qt 库
target_link_libraries(appMBRtool PRIVATE
    Qt6::Quick
    Qt6::Core
)
target_link_libraries(appMBRtool PRIVATE Qt6::Core)
target_link_libraries(appMBRtool PRIVATE Qt6::Core)

# 添加头文件包含目录
target_include_directories(appMBRtool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 安装规则
include(GNUInstallDirs)
install(TARGETS appMBRtool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
