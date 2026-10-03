#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

class UserProfile
{
public:
    QString id;                 ///< 唯一标识
    QString name;               ///< 姓名
    char    gender = 'M';       ///< 性别：'M' 男 / 'F' 女
    int     age = 25;           ///< 年龄（岁）
    double  height = 170.0;     ///< 身高（cm）
    double  weight = 70.0;      ///< 当前体重（kg）
    double  targetWeight = 65.0;///< 目标体重（kg）
    int     activityLevel = 2;  ///< 日常活动量 1~5
    QString goalType = "lose";  ///< 目标："lose"减重 / "maintain"维持 / "gain"增重
    double  weeklyLossTarget = 0.5; ///< 每周体重变化目标（kg），减重时为正

    QStringList dislikedExerciseIds;   ///< 不喜欢的运动
    QStringList likedExerciseIds;      ///< 喜欢的运动
    QStringList dislikedRecipeIds;     ///< 不喜欢的食谱
    QStringList likedRecipeIds;        ///< 喜欢的食谱

    double calculateBMI() const;

    QString bmiCategory() const;

    double activityFactor() const;

    double getBMR() const;

    double getTDEE() const;

    double dailyCalorieDeficit() const;

    double recommendedIntake() const;

    QJsonObject toJson() const;

    static UserProfile fromJson(const QJsonObject &o);
};
