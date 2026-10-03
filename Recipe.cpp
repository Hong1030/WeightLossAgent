#include "Recipe.h"

#include <QJsonArray>

QJsonObject Recipe::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["ingredients"] = ingredients;
    o["totalCalories"] = totalCalories;
    o["mealType"] = mealType;

    QJsonArray tags;
    for (const QString &t : nutritionTags)
        tags.append(t);
    o["nutritionTags"] = tags;
    return o;
}

Recipe Recipe::fromJson(const QJsonObject &o)
{
    Recipe r;
    r.id = o.value("id").toString();
    r.name = o.value("name").toString();
    r.ingredients = o.value("ingredients").toString();
    r.totalCalories = o.value("totalCalories").toDouble(300.0);
    r.mealType = o.value("mealType").toString(QStringLiteral("lunch"));

    r.nutritionTags.clear();
    for (const QJsonValue &v : o.value("nutritionTags").toArray())
        r.nutritionTags << v.toString();
    return r;
}
