#pragma once

#include <QDate>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief 计划中的运动条目（自包含，便于导出报告，不依赖运动库）
 */
struct PlannedExercise {
    QString id;
    QString name;
    double  metValue = 0;
    int     durationMinutes = 0;
    double  caloriesBurned = 0;

    QJsonObject toJson() const;
    static PlannedExercise fromJson(const QJsonObject &o);
};

/**
 * @brief 计划中的食谱条目（自包含）
 */
struct PlannedRecipe {
    QString id;
    QString name;
    QString ingredients;
    double  totalCalories = 0;
    QString mealType;

    QJsonObject toJson() const;
    static PlannedRecipe fromJson(const QJsonObject &o);
};

/**
 * @brief 单日计划：运动 + 早/午/晚/加餐
 */
struct DailyPlan {
    QDate date;
    QList<PlannedExercise> exercises;
    QList<PlannedRecipe> breakfast;
    QList<PlannedRecipe> lunch;
    QList<PlannedRecipe> dinner;
    QList<PlannedRecipe> snacks;
    bool completed = false;

    double totalCaloriesOut() const;  ///< 运动消耗
    double totalCaloriesIn() const;   ///< 饮食摄入
    double netCalories() const;       ///< 净热量 = 摄入 - 消耗

    QJsonObject toJson() const;
    static DailyPlan fromJson(const QJsonObject &o);
};

/**
 * @brief 一周计划（7 天）
 */
class WeeklyPlan
{
public:
    QList<DailyPlan> days;

    double totalCaloriesIn() const;
    double totalCaloriesOut() const;

    QJsonObject toJson() const;
    static WeeklyPlan fromJson(const QJsonObject &o);
};
