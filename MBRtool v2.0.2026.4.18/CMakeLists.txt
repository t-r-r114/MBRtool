cmake_minimum_required(VERSION 3.16)

project(MBRtool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)  # 明确设置 C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 查找所需的 Qt 模块 (已合并去重，并加入了 Network)
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Core Network)

# 2. 简化项目设置
qt_standard_project_setup(REQUIRES 6.8)

# 3. 添加所有源文件到可执行目标 (移除了重复的 disassemblymain.cpp)
qt_add_executable(appMBRtool
    main.cpp
    disassemblymain.cpp
    mbrsave.cpp
    mbrwrite.cpp
    disassemblywindows.cpp
    disassembler.cpp
)

# 4. 统一链接所有需要的 Qt 库 (替换了之前的 mytarget，并合并了 Core, Quick 等)
target_link_libraries(appMBRtool PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Core
    Qt6::Network
)

# 5. 添加 QML 模块 (整理了层级结构)
qt_add_qml_module(appMBRtool
    URI MBRtool
    VERSION 1.0
    QML_FILES
        Main.qml
        disassembly.qml
        HexEditor.qml
    SOURCES
        FileReader.h FileReader.cpp
        HexEditor.h HexEditor.cpp
        disassembler.h disassemblywindows.h
        mbrsave.h mbrwrite.h
        Public.h
        ntwiter.h ntwiter.cpp
)

# 6. 设置目标属性
set_target_properties(appMBRtool PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 7. 添加头文件包含目录
target_include_directories(appMBRtool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WIN32)
    target_link_options(appMBRtool PRIVATE
        "LINKER:/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
    )
endif()

# 8. 安装规则
include(GNUInstallDirs)
install(TARGETS appMBRtool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
