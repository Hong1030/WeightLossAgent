#pragma once

#include "DataStore.h"
#include "AgentCore.h"
#include "WeeklyPlan.h"
#include "WeeklyPlanManager.h"

#include <QMainWindow>
#include <QList>
#include <QHash>
#include <QVector>

class QListWidget;
class QTableWidget;
class QLabel;
class QTabWidget;
class QTextBrowser;
class QScrollArea;
class QGroupBox;
class QVBoxLayout;
class QComboBox;

/**
 * @brief 主窗口
 *
 * 左侧折叠式用户管理 + 右侧 Tab 分页：主页 / 身体指标 / 运动处方 / 食谱推荐 / 周计划。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 用户管理
    void onNewUser();
    void onEditUser();
    void onDeleteUser();
    void onUserChanged(int index);

    // 数据管理
    void onManageData();
    void onOpenData();
    void onSaveData();
    void onAbout();

    // 推荐
    void onRegenerateExercise();
    void onRegenerateMeals();
    void onRegenerateAll();

    // 周计划
    void onGenerateWeeklyPlan();
    void onExportPlan();

    // 反馈
    void onExerciseFeedback();
    void onRecipeFeedback();

private:
    // 数据与业务
    DataStore store_;
    AgentCore agent_;
    WeeklyPlanManager planManager_;
    QHash<QString, WeeklyPlan> weeklyPlanCache_;  // 按用户隔离的周计划缓存
    QList<AgentCore::ExercisePlanItem> cachedExercisePlan_;  // 当前运动处方（主页与详情页共享）
    AgentCore::MealPlan cachedMealPlan_;                      // 当前食谱（主页与详情页共享）

    // 左侧用户管理（折叠式下拉框）
    QComboBox *userSelector_ = nullptr;
    QLabel *lblUserCard_ = nullptr;   // 当前用户信息卡片（橘黄）

    // 右侧 Tabs
    QTabWidget *tabs_ = nullptr;

    // Tab0 主页
    QLabel *lblHomeUser_ = nullptr;      // 用户概览
    QLabel *lblHomeBmi_ = nullptr;       // BMI
    QLabel *lblHomeExercise_ = nullptr;  // 今日运动
    QLabel *lblHomeMeals_ = nullptr;     // 今日食谱
    QLabel *lblHomeWeek_ = nullptr;      // 周计划状态

    // Tab1 身体指标
    QLabel *lblUserInfo_ = nullptr;       // 个人信息标题区
    QVector<QLabel *> infoValueLabels_;   // 6 个个人信息小卡的值标签
    QLabel *lblBmiValue_ = nullptr;       // BMI 数值
    QLabel *lblBmiCategory_ = nullptr;    // BMI 评价标签
    QLabel *lblBmrValue_ = nullptr;       // BMR
    QLabel *lblTdeeValue_ = nullptr;      // TDEE
    QLabel *lblIntakeValue_ = nullptr;    // 推荐摄入
    QLabel *lblExTargetValue_ = nullptr;  // 运动目标

    // Tab2 运动处方
    QTableWidget *exerciseTable_ = nullptr;
    QLabel *lblExerciseTotal_ = nullptr;

    // Tab3 食谱推荐
    QList<QTableWidget *> mealTables_;    // 早/午/晚/加餐 四张表
    QList<QGroupBox *> mealGroups_;       // 四个餐次分组框（动态配色）
    QLabel *lblMealTotal_ = nullptr;

    // Tab4 周计划
    QScrollArea *weekScroll_ = nullptr;
    QWidget *weekContainer_ = nullptr;
    QVBoxLayout *weekLayout_ = nullptr;
    QList<QGroupBox *> weekGroups_;       // 每天一个 QGroupBox
    QLabel *lblEmptyWeek_ = nullptr;
    QLabel *lblWeekSummary_ = nullptr;

    // 内部方法
    void setupUi();
    void setupMenuBar();

    void refreshUserSelector();
    void refreshUserCard();
    void refreshHome();
    void refreshOverview();
    void refreshExercisePlan();
    void refreshMealPlan();
    void refreshWeeklyPlan();
    void refreshAll();

    UserProfile *currentUser();
    const UserProfile *currentUser() const;

    QString dataFilePath() const;
    void loadDataOrDefault();
    bool saveData();

    QString buildDayDetailText(const DailyPlan &day) const;
    void applyExerciseFeedback(const QString &id, const QString &kind);
    void applyRecipeFeedback(const QString &id, const QString &kind);
};
