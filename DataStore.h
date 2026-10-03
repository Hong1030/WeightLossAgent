#pragma once

#include "UserProfile.h"
#include "Exercise.h"
#include "Recipe.h"

#include <QList>
#include <QString>

class DataStore
{
public:
    QList<UserProfile> users;
    QList<Exercise>    exercises;
    QList<Recipe>      recipes;

    QString currentUserId;   ///< 当前选中的用户

    // ---------------- JSON 持久化 ----------------
    bool saveToFile(const QString &path, QString *errMsg = nullptr) const;
    bool loadFromFile(const QString &path, QString *errMsg = nullptr);

    // ---------------- 查找 ----------------
    UserProfile *findUser(const QString &id);
    const UserProfile *findUser(const QString &id) const;
    Exercise *findExercise(const QString &id);
    const Exercise *findExercise(const QString &id) const;
    Recipe *findRecipe(const QString &id);
    const Recipe *findRecipe(const QString &id) const;

    // ---------------- ID 生成 ----------------
    QString nextUserId() const;
    QString nextExerciseId() const;
    QString nextRecipeId() const;

    // ---------------- CSV 导入/导出 ----------------
    bool importExercisesCsv(const QString &path, QString *errMsg = nullptr);
    bool importRecipesCsv(const QString &path, QString *errMsg = nullptr);
    bool exportExercisesCsv(const QString &path, QString *errMsg = nullptr) const;
    bool exportRecipesCsv(const QString &path, QString *errMsg = nullptr) const;

    // ---------------- 统计辅助 ----------------
    int countByMealType(const QString &mealType) const;
    int countByCategory(const QString &category) const;

    /** 由内置示例数据填充（首次运行或用户手动重置时调用） */
    void fillInitialData();
};
