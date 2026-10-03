#include "UserProfile.h"

#include <QJsonArray>

double UserProfile::calculateBMI() const
{
    if (height <= 0.0)
        return 0.0;
    double hMeter = height / 100.0;
    return weight / (hMeter * hMeter);
}

QString UserProfile::bmiCategory() const
{
    double bmi = calculateBMI();
    if (bmi < 18.5)
        return QStringLiteral("偏瘦");
    if (bmi < 24.0)
        return QStringLiteral("正常");
    if (bmi < 28.0)
        return QStringLiteral("超重");
    return QStringLiteral("肥胖");
}

double UserProfile::activityFactor() const
{
    switch (activityLevel) {
    case 1:  return 1.2;     // 久坐，几乎不运动
    case 2:  return 1.375;   // 轻度活动（每周 1~3 次）
    case 3:  return 1.55;    // 中度活动（每周 3~5 次）
    case 4:  return 1.725;   // 高度活动（每周 6~7 次）
    case 5:  return 2.0;     // 极高活动（体力劳动者/高强度训练）
    default: return 1.375;
    }
}

double UserProfile::getBMR() const
{
    // Mifflin-St Jeor 公式
    double bmr = 10.0 * weight + 6.25 * height - 5.0 * age;
    if (gender == 'M')
        bmr += 5.0;
    else
        bmr -= 161.0;
    return bmr;
}

double UserProfile::getTDEE() const
{
    return getBMR() * activityFactor();
}

double UserProfile::dailyCalorieDeficit() const
{
    if (goalType == "lose")
        return 7700.0 * weeklyLossTarget / 7.0;   // 减重：热量缺口
    if (goalType == "gain")
        return -7700.0 * weeklyLossTarget / 7.0;  // 增重：热量盈余（负缺口）
    return 0.0;                                    // 维持
}

double UserProfile::recommendedIntake() const
{
    return getTDEE() - dailyCalorieDeficit();
}

QJsonObject UserProfile::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["gender"] = QString(QChar(gender));
    o["age"] = age;
    o["height"] = height;
    o["weight"] = weight;
    o["targetWeight"] = targetWeight;
    o["activityLevel"] = activityLevel;
    o["goalType"] = goalType;
    o["weeklyLossTarget"] = weeklyLossTarget;

    auto toArr = [](const QStringList &l) {
        QJsonArray a;
        for (const QString &s : l)
            a.append(s);
        return a;
    };
    o["dislikedExerciseIds"] = toArr(dislikedExerciseIds);
    o["likedExerciseIds"] = toArr(likedExerciseIds);
    o["dislikedRecipeIds"] = toArr(dislikedRecipeIds);
    o["likedRecipeIds"] = toArr(likedRecipeIds);
    return o;
}

UserProfile UserProfile::fromJson(const QJsonObject &o)
{
    UserProfile u;
    u.id = o.value("id").toString();
    u.name = o.value("name").toString();
    QString g = o.value("gender").toString();
    u.gender = g.isEmpty() ? 'M' : g.at(0).toLatin1();
    u.age = o.value("age").toInt(25);
    u.height = o.value("height").toDouble(170.0);
    u.weight = o.value("weight").toDouble(70.0);
    u.targetWeight = o.value("targetWeight").toDouble(65.0);
    u.activityLevel = o.value("activityLevel").toInt(2);
    u.goalType = o.value("goalType").toString("lose");
    u.weeklyLossTarget = o.value("weeklyLossTarget").toDouble(0.5);

    auto fromArr = [](const QJsonValue &v) {
        QStringList l;
        for (const QJsonValue &x : v.toArray())
            l << x.toString();
        return l;
    };
    u.dislikedExerciseIds = fromArr(o.value("dislikedExerciseIds"));
    u.likedExerciseIds = fromArr(o.value("likedExerciseIds"));
    u.dislikedRecipeIds = fromArr(o.value("dislikedRecipeIds"));
    u.likedRecipeIds = fromArr(o.value("likedRecipeIds"));
    return u;
}
