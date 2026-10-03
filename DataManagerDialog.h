#pragma once

#include "DataStore.h"

#include <QDialog>

class QTableWidget;
class QLabel;
class QTableWidgetItem;

/**
 * @brief 运动库 / 食谱库管理对话框（含增删改、搜索筛选、CSV 导入导出）
 */
class DataManagerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DataManagerDialog(DataStore *store, QWidget *parent = nullptr);

private slots:
    // 运动库
    void onAddExercise();
    void onEditExercise();
    void onDeleteExercise();
    void onImportExercises();
    void onExportExercises();
    void onExerciseSearch(const QString &kw);
    void onExerciseFilter(int index);

    // 食谱库
    void onAddRecipe();
    void onEditRecipe();
    void onDeleteRecipe();
    void onImportRecipes();
    void onExportRecipes();
    void onRecipeSearch(const QString &kw);
    void onRecipeFilter(int index);

    // 表格单元格直接编辑
    void onExerciseItemChanged(QTableWidgetItem *item);
    void onRecipeItemChanged(QTableWidgetItem *item);

private:
    DataStore *store_;

    QTableWidget *exerciseTable_;
    QTableWidget *recipeTable_;
    QLabel *exCountLabel_;
    QLabel *recCountLabel_;
    QString exerciseSearch_;
    QString exerciseCategory_;   // 空 = 全部
    QString recipeSearch_;
    QString recipeMealType_;     // 空 = 全部

    void refreshExerciseTable();
    void refreshRecipeTable();
    void refreshCounts();

    int selectedExerciseRow() const;
    int selectedRecipeRow() const;
};
