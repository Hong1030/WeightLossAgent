#include "WeeklyPlanManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

static void setStreamUtf8(QTextStream &ts)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Utf8);
#else
    ts.setCodec("UTF-8");
#endif
}

// ---------------------------------------------------------------------------
// 生成周计划
// ---------------------------------------------------------------------------

WeeklyPlan WeeklyPlanManager::generateWeeklyPlan(
    const UserProfile &user, const AgentCore &agent,
    const QList<Exercise> &exerciseDB, const QList<Recipe> &recipeDB,
    const QDate &startDate) const
{
    WeeklyPlan plan;
    AgentCore::CalorieNeed need = agent.calculateCalorieNeeds(user);

    for (int i = 0; i < 7; ++i) {
        DailyPlan day;
        day.date = startDate.addDays(i);

        // 运动处方
        auto exList = agent.generateExercisePrescription(user, need.exerciseTarget, exerciseDB);
        for (const AgentCore::ExercisePlanItem &it : exList) {
            PlannedExercise pe;
            pe.id = it.exerciseId;
            pe.name = it.exerciseName;
            pe.metValue = it.metValue;
            pe.durationMinutes = it.durationMinutes;
            pe.caloriesBurned = it.caloriesBurned;
            day.exercises.append(pe);
        }

        // 食谱推荐
        AgentCore::MealPlan mp = agent.generateMealPlan(
            need.recommendedIntake, recipeDB,
            user.dislikedRecipeIds, user.likedRecipeIds);

        auto toPlanned = [](const QList<Recipe> &src, const QString &mealType) {
            QList<PlannedRecipe> out;
            for (const Recipe &r : src) {
                PlannedRecipe pr;
                pr.id = r.id;
                pr.name = r.name;
                pr.ingredients = r.ingredients;
                pr.totalCalories = r.totalCalories;
                pr.mealType = mealType;
                out.append(pr);
            }
            return out;
        };
        day.breakfast = toPlanned(mp.breakfast, "breakfast");
        day.lunch     = toPlanned(mp.lunch, "lunch");
        day.dinner    = toPlanned(mp.dinner, "dinner");
        day.snacks    = toPlanned(mp.snacks, "snack");

        plan.days.append(day);
    }
    return plan;
}

// ---------------------------------------------------------------------------
// 保存 / 加载
// ---------------------------------------------------------------------------

bool WeeklyPlanManager::saveToFile(const WeeklyPlan &plan, const QString &path, QString *errMsg) const
{
    QJsonObject root;
    root["plan"] = plan.toJson();

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入文件：%1").arg(path);
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

WeeklyPlan WeeklyPlanManager::loadFromFile(const QString &path, QString *errMsg) const
{
    WeeklyPlan plan;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法打开文件：%1").arg(path);
        return plan;
    }
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    f.close();
    if (pe.error != QJsonParseError::NoError) {
        if (errMsg) *errMsg = QStringLiteral("JSON 解析失败：%1").arg(pe.errorString());
        return plan;
    }
    plan = WeeklyPlan::fromJson(doc.object().value("plan").toObject());
    return plan;
}

// ---------------------------------------------------------------------------
// 导出报告
// ---------------------------------------------------------------------------

bool WeeklyPlanManager::exportToText(const WeeklyPlan &plan, const QString &path, QString *errMsg) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入文件：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);

    ts << "========== 一周减重计划 ==========\n\n";
    for (const DailyPlan &d : plan.days) {
        ts << "【" << d.date.toString("yyyy-MM-dd ddd") << "】\n";
        ts << "  运动处方（消耗 " << QString::number(d.totalCaloriesOut(), 'f', 1) << " kcal）：\n";
        if (d.exercises.isEmpty())
            ts << "    （无）\n";
        for (const PlannedExercise &e : d.exercises)
            ts << QStringLiteral("    - %1  %2 分钟  ≈%3 kcal\n")
                  .arg(e.name)
                  .arg(e.durationMinutes)
                  .arg(QString::number(e.caloriesBurned, 'f', 1));

        ts << "  饮食计划（摄入 " << QString::number(d.totalCaloriesIn(), 'f', 1) << " kcal）：\n";
        auto printMeal = [&](const char *label, const QList<PlannedRecipe> &list) {
            if (list.isEmpty()) return;
            ts << QStringLiteral("    %1：").arg(QString::fromUtf8(label));
            QStringList names;
            for (const PlannedRecipe &r : list)
                names << QStringLiteral("%1(%2kcal)").arg(r.name).arg(QString::number(r.totalCalories, 'f', 0));
            ts << names.join("、") << "\n";
        };
        printMeal("早餐", d.breakfast);
        printMeal("午餐", d.lunch);
        printMeal("晚餐", d.dinner);
        printMeal("加餐", d.snacks);

        ts << QStringLiteral("  净热量：%1 kcal\n\n")
              .arg(QString::number(d.netCalories(), 'f', 1));
    }
    ts << QStringLiteral("一周运动总消耗：%1 kcal\n")
          .arg(QString::number(plan.totalCaloriesOut(), 'f', 1));
    ts << QStringLiteral("一周饮食总摄入：%1 kcal\n")
          .arg(QString::number(plan.totalCaloriesIn(), 'f', 1));
    f.close();
    return true;
}

bool WeeklyPlanManager::exportToCsv(const WeeklyPlan &plan, const QString &path, QString *errMsg) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入文件：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);

    ts << "日期,餐次,菜品,食材,热量(kcal),运动项目,时长(分钟),运动消耗(kcal),净热量(kcal)\n";
    for (const DailyPlan &d : plan.days) {
        QString dateStr = d.date.toString("yyyy-MM-dd");
        auto writeMeal = [&](const QString &meal, const QList<PlannedRecipe> &list) {
            if (list.isEmpty()) {
                ts << dateStr << ',' << meal << ",,,,,\n";
                return;
            }
            for (const PlannedRecipe &r : list) {
                ts << dateStr << ',' << meal << ',' << r.name << ','
                   << r.ingredients << ',' << r.totalCalories << ",,\n";
            }
        };
        writeMeal("早餐", d.breakfast);
        writeMeal("午餐", d.lunch);
        writeMeal("晚餐", d.dinner);
        writeMeal("加餐", d.snacks);

        for (const PlannedExercise &e : d.exercises) {
            ts << dateStr << ",运动,,,,," << e.name << ',' << e.durationMinutes
               << ',' << e.caloriesBurned << ',' << d.netCalories() << '\n';
        }
    }
    f.close();
    return true;
}
