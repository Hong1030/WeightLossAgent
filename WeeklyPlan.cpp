#include "WeeklyPlan.h"

// ---------------------------- PlannedExercise ----------------------------

QJsonObject PlannedExercise::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["metValue"] = metValue;
    o["durationMinutes"] = durationMinutes;
    o["caloriesBurned"] = caloriesBurned;
    return o;
}

PlannedExercise PlannedExercise::fromJson(const QJsonObject &o)
{
    PlannedExercise p;
    p.id = o.value("id").toString();
    p.name = o.value("name").toString();
    p.metValue = o.value("metValue").toDouble(0);
    p.durationMinutes = o.value("durationMinutes").toInt(0);
    p.caloriesBurned = o.value("caloriesBurned").toDouble(0);
    return p;
}

// ---------------------------- PlannedRecipe ----------------------------

QJsonObject PlannedRecipe::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["ingredients"] = ingredients;
    o["totalCalories"] = totalCalories;
    o["mealType"] = mealType;
    return o;
}

PlannedRecipe PlannedRecipe::fromJson(const QJsonObject &o)
{
    PlannedRecipe p;
    p.id = o.value("id").toString();
    p.name = o.value("name").toString();
    p.ingredients = o.value("ingredients").toString();
    p.totalCalories = o.value("totalCalories").toDouble(0);
    p.mealType = o.value("mealType").toString();
    return p;
}

// ---------------------------- DailyPlan ----------------------------

double DailyPlan::totalCaloriesOut() const
{
    double t = 0;
    for (const PlannedExercise &e : exercises)
        t += e.caloriesBurned;
    return t;
}

double DailyPlan::totalCaloriesIn() const
{
    double t = 0;
    for (const PlannedRecipe &r : breakfast) t += r.totalCalories;
    for (const PlannedRecipe &r : lunch)     t += r.totalCalories;
    for (const PlannedRecipe &r : dinner)    t += r.totalCalories;
    for (const PlannedRecipe &r : snacks)    t += r.totalCalories;
    return t;
}

double DailyPlan::netCalories() const
{
    return totalCaloriesIn() - totalCaloriesOut();
}

QJsonObject DailyPlan::toJson() const
{
    QJsonObject o;
    o["date"] = date.toString(Qt::ISODate);
    o["completed"] = completed;

    QJsonArray ex, b, l, d, s;
    for (const PlannedExercise &e : exercises) ex.append(e.toJson());
    for (const PlannedRecipe &r : breakfast)   b.append(r.toJson());
    for (const PlannedRecipe &r : lunch)       l.append(r.toJson());
    for (const PlannedRecipe &r : dinner)      d.append(r.toJson());
    for (const PlannedRecipe &r : snacks)      s.append(r.toJson());

    o["exercises"] = ex;
    o["breakfast"] = b;
    o["lunch"] = l;
    o["dinner"] = d;
    o["snacks"] = s;
    return o;
}

DailyPlan DailyPlan::fromJson(const QJsonObject &o)
{
    DailyPlan p;
    p.date = QDate::fromString(o.value("date").toString(), Qt::ISODate);
    p.completed = o.value("completed").toBool(false);

    auto readEx = [](const QJsonValue &v, QList<PlannedExercise> &out) {
        for (const QJsonValue &x : v.toArray())
            out.append(PlannedExercise::fromJson(x.toObject()));
    };
    auto readRec = [](const QJsonValue &v, QList<PlannedRecipe> &out) {
        for (const QJsonValue &x : v.toArray())
            out.append(PlannedRecipe::fromJson(x.toObject()));
    };
    readEx(o.value("exercises"), p.exercises);
    readRec(o.value("breakfast"), p.breakfast);
    readRec(o.value("lunch"), p.lunch);
    readRec(o.value("dinner"), p.dinner);
    readRec(o.value("snacks"), p.snacks);
    return p;
}

// ---------------------------- WeeklyPlan ----------------------------

double WeeklyPlan::totalCaloriesIn() const
{
    double t = 0;
    for (const DailyPlan &d : days)
        t += d.totalCaloriesIn();
    return t;
}

double WeeklyPlan::totalCaloriesOut() const
{
    double t = 0;
    for (const DailyPlan &d : days)
        t += d.totalCaloriesOut();
    return t;
}

QJsonObject WeeklyPlan::toJson() const
{
    QJsonObject o;
    QJsonArray arr;
    for (const DailyPlan &d : days)
        arr.append(d.toJson());
    o["days"] = arr;
    return o;
}

WeeklyPlan WeeklyPlan::fromJson(const QJsonObject &o)
{
    WeeklyPlan w;
    for (const QJsonValue &v : o.value("days").toArray())
        w.days.append(DailyPlan::fromJson(v.toObject()));
    return w;
}
