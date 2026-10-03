cmake_minimum_required(VERSION 3.16)

project(WeightLossAgent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------------------------------------
# 查找 Qt：优先 Qt6，回退到 Qt5 (>=5.14)。题目要求 Qt 5.14+，二者均可。
# 使用方式（VS2022/VS2026 中的 CMake 或 Qt Creator）：
#   在 CMake 配置时指定  CMAKE_PREFIX_PATH=<Qt安装目录>/6.11.2/msvc2022_64
# ---------------------------------------------------------------------------
find_package(Qt6 6.2 QUIET COMPONENTS Widgets)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Widgets)
    message(STATUS "Using Qt6: ${Qt6_VERSION}")
else()
    find_package(Qt5 5.14 REQUIRED COMPONENTS Widgets)
    set(QT_LIBS Qt5::Widgets)
    message(STATUS "Using Qt5: ${Qt5_VERSION}")
endif()

# 收集全部源码（头文件也加入，便于在 IDE 中展示）
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(WeightLossAgent WIN32 ${PROJECT_SOURCES})

target_include_directories(WeightLossAgent PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
)

target_link_libraries(WeightLossAgent PRIVATE ${QT_LIBS})

# Windows 下作为 GUI 程序（不弹出控制台窗口）
set_target_properties(WeightLossAgent PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# MSVC 警告设置
if(MSVC)
    target_compile_options(WeightLossAgent PRIVATE /W3 /utf-8)
endif()
