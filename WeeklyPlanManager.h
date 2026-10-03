#pragma once

#include "WeeklyPlan.h"
#include "AgentCore.h"
#include "Exercise.h"
#include "Recipe.h"

#include <QDate>
#include <QList>

/**
 * @brief 周计划管理：生成、保存/加载、导出报告
 */
class WeeklyPlanManager
{
public:
    /**
     * 为一周（7 天）生成计划。
     * 每天独立调用运动处方与食谱推荐，因此每天的组合会有差异。
     */
    WeeklyPlan generateWeeklyPlan(
        const UserProfile &user,
        const AgentCore &agent,
        const QList<Exercise> &exerciseDB,
        const QList<Recipe> &recipeDB,
        const QDate &startDate) const;

    /** 保存计划为 JSON */
    bool saveToFile(const WeeklyPlan &plan, const QString &path, QString *errMsg = nullptr) const;
    /** 从 JSON 加载计划 */
    WeeklyPlan loadFromFile(const QString &path, QString *errMsg = nullptr) const;

    /** 导出为纯文本报告 */
    bool exportToText(const WeeklyPlan &plan, const QString &path, QString *errMsg = nullptr) const;
    /** 导出为 CSV 报告 */
    bool exportToCsv(const WeeklyPlan &plan, const QString &path, QString *errMsg = nullptr) const;
};
