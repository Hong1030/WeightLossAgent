#pragma once

#include "DataStore.h"

/**
 * @brief 构建内置初始数据（首次运行或重置时使用）
 *
 * 数据规模满足课程要求：10 个用户、30 项运动、40 道食谱。
 */
DataStore buildInitialData();
