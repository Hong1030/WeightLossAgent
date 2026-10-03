#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>

/**
 * @brief 食谱条目类
 *
 * 记录菜品名称、食材组成、总卡路里与适用餐次，可附加营养标签。
 */
class Recipe
{
public:
    QString id;                          ///< 唯一标识
    QString name;                        ///< 菜品名称
    QString ingredients;                 ///< 食材及用量描述
    double  totalCalories = 300.0;       ///< 总卡路里（kcal）
    QString mealType = "lunch";          ///< 餐次：breakfast/lunch/dinner/snack
    QStringList nutritionTags;           ///< 营养标签：高蛋白/低脂/高纤维等

    QJsonObject toJson() const;
    static Recipe fromJson(const QJsonObject &o);
};
