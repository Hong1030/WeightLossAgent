# ---------------------------------------------------------------
# 面向减重的运动处方与食谱推荐智能体 —— qmake 项目文件
# 可直接用 Qt Creator 打开此 .pro，或命令行 qmake + make 构建
# ---------------------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = WeightLossAgent
TEMPLATE  = app
CONFIG   += c++17

# ---------------- 源文件 ----------------
SOURCES += \
    src/main.cpp \
    src/core/UserProfile.cpp \
    src/core/Exercise.cpp \
    src/core/Recipe.cpp \
    src/core/AgentCore.cpp \
    src/core/WeeklyPlan.cpp \
    src/core/WeeklyPlanManager.cpp \
    src/core/DataStore.cpp \
    src/core/InitialData.cpp \
    src/ui/MainWindow.cpp \
    src/ui/UserEditDialog.cpp \
    src/ui/DataManagerDialog.cpp

# ---------------- 头文件 ----------------
HEADERS += \
    src/core/UserProfile.h \
    src/core/Exercise.h \
    src/core/Recipe.h \
    src/core/AgentCore.h \
    src/core/WeeklyPlan.h \
    src/core/WeeklyPlanManager.h \
    src/core/DataStore.h \
    src/core/InitialData.h \
    src/ui/MainWindow.h \
    src/ui/UserEditDialog.h \
    src/ui/DataManagerDialog.h

# ---------------- 头文件搜索路径 ----------------
INCLUDEPATH += src src/core src/ui

# ---------------- MSVC 专用编译选项（源码含中文，需 UTF-8） ----------------
msvc {
    QMAKE_CXXFLAGS += /utf-8 /permissive- /Zc:__cplusplus
}
