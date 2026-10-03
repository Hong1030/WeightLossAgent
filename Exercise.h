#pragma once

#include <QString>
#include <QJsonObject>

class Exercise
{
public:
    QString id;                 ///< 唯一标识
    QString name;               ///< 运动名称
    double  metValue = 4.0;     ///< 代谢当量 MET
    QString category = "有氧";  ///< 类别：有氧 / 力量 / 柔韧
    QString description;        ///< 描述

    double caloriesPerMinute(double weightKg) const;

    QJsonObject toJson() const;
    static Exercise fromJson(const QJsonObject &o);
};
