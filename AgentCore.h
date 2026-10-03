#pragma once

#include "UserProfile.h"
#include "Exercise.h"
#include "Recipe.h"

#include <QList>
#include <QStringList>
#include <random>

class AgentCore
{
public:
    struct CalorieNeed {
        double bmr = 0;                 ///< 基础代谢率
        double tdee = 0;                ///< 总能量消耗
        double recommendedIntake = 0;   ///< 每日推荐摄入热量
        double exerciseTarget = 0;      ///< 每日运动消耗目标
        double deficit = 0;             ///< 每日热量缺口
    };

    struct ExercisePlanItem {
        QString exerciseId;
        QString exerciseName;
        double  metValue = 0;
        int     durationMinutes = 0;
        double  caloriesBurned = 0;
    };

    struct MealPlan {
        QList<Recipe> breakfast;
        QList<Recipe> lunch;
        QList<Recipe> dinner;
        QList<Recipe> snacks;
        double totalCalories() const;
        int itemCount() const;
    };

    AgentCore();

    CalorieNeed calculateCalorieNeeds(const UserProfile &user) const;

    QList<ExercisePlanItem> generateExercisePrescription(
        const UserProfile &user,
        double targetCalories,
        const QList<Exercise> &exerciseDB) const;

    MealPlan generateMealPlan(
        double targetCalories,
        const QList<Recipe> &recipeDB,
        const QStringList &dislikeIds = {},
        const QStringList &likeIds = {}) const;

private:
    mutable std::mt19937 rng_;

    int    randInt(int lo, int hi) const;
    double randDouble(double lo, double hi) const;

    QList<ExercisePlanItem> buildOneCombination(
        const QList<Exercise> &pool, double weightKg, double target) const;

    QList<Recipe> fillOneMeal(const QList<Recipe> &pool, double target) const;
};
