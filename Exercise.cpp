#include "Exercise.h"

double Exercise::caloriesPerMinute(double weightKg) const
{
    return metValue * 3.5 * weightKg / 200.0;
}

QJsonObject Exercise::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["metValue"] = metValue;
    o["category"] = category;
    o["description"] = description;
    return o;
}

Exercise Exercise::fromJson(const QJsonObject &o)
{
    Exercise e;
    e.id = o.value("id").toString();
    e.name = o.value("name").toString();
    e.metValue = o.value("metValue").toDouble(4.0);
    e.category = o.value("category").toString(QStringLiteral("有氧"));
    e.description = o.value("description").toString();
    return e;
}
