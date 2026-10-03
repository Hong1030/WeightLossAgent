#include "MainWindow.h"
#include "UserEditDialog.h"
#include "DataManagerDialog.h"

#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QMenuBar>
#include <QMenu>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QApplication>
#include <QFont>
#include <QSignalBlocker>

namespace {
const QStringList kMealNames = {"早餐", "午餐", "晚餐", "加餐"};

// 四餐配色：背景 / 边框（深于内部） / 标题文字
struct MealColor { const char *bg; const char *border; const char *title; };
const MealColor kMealColors[4] = {
    {"#e3f2fd", "#64b5f6", "#1565c0"},  // 早餐 淡蓝
    {"#fff3e0", "#ffb74d", "#e65100"},  // 午餐 淡橙
    {"#f3e5f5", "#ba68c8", "#6a1b9a"},  // 晚餐 淡紫
    {"#fff9c4", "#fdd835", "#827717"},  // 加餐 淡黄
};

const QStringList kInfoLabels = {"姓名", "性别", "年龄", "身高", "体重", "目标体重"};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("面向减重的运动处方与食谱推荐智能体"));
    resize(1180, 780);

    setupMenuBar();
    setupUi();

    loadDataOrDefault();
    refreshAll();
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));
    fileMenu->addAction(QStringLiteral("打开数据..."), this, &MainWindow::onOpenData);
    fileMenu->addAction(QStringLiteral("保存数据"), this, &MainWindow::onSaveData);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("退出"), this, &QWidget::close);

    auto *dataMenu = menuBar()->addMenu(QStringLiteral("数据(&D)"));
    dataMenu->addAction(QStringLiteral("运动库/食谱库管理..."), this, &MainWindow::onManageData);

    auto *helpMenu = menuBar()->addMenu(QStringLiteral("帮助(&H)"));
    helpMenu->addAction(QStringLiteral("关于"), this, &MainWindow::onAbout);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget;
    central->setObjectName("centralWidget");
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(14);

    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(8);

    // ================= 左侧：折叠式用户管理 =================
    auto *leftWidget = new QWidget;
    leftWidget->setObjectName("cardPanel");
    leftWidget->setMinimumWidth(250);
    leftWidget->setMaximumWidth(300);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(16, 18, 16, 18);
    leftLayout->setSpacing(12);

    auto *userTitle = new QLabel(QStringLiteral("用户管理"));
    userTitle->setObjectName("sectionTitle");
    auto *userSub = new QLabel(QStringLiteral("点击姓名切换用户"));
    userSub->setObjectName("sectionSubtitle");

    userSelector_ = new QComboBox;
    userSelector_->setObjectName("userSelector");

    // 当前用户信息卡片（橘黄）
    auto *userCard = new QFrame;
    userCard->setObjectName("userCard");
    auto *userCardLay = new QVBoxLayout(userCard);
    userCardLay->setContentsMargins(16, 14, 16, 14);
    userCardLay->setSpacing(2);
    lblUserCard_ = new QLabel;
    lblUserCard_->setTextFormat(Qt::RichText);
    lblUserCard_->setWordWrap(true);
    lblUserCard_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    userCardLay->addWidget(lblUserCard_);

    auto *btnNew = new QPushButton(QStringLiteral("＋ 新建用户"));
    btnNew->setObjectName("accentBtn");
    auto *btnEdit = new QPushButton(QStringLiteral("编辑用户"));
    auto *btnDel = new QPushButton(QStringLiteral("删除用户"));
    btnDel->setObjectName("dangerBtn");

    leftLayout->addWidget(userTitle);
    leftLayout->addWidget(userSub);
    leftLayout->addWidget(userSelector_);
    leftLayout->addWidget(userCard);   // 高度自适应内容，不留空白
    leftLayout->addStretch(1);         // 按钮靠底部
    leftLayout->addWidget(btnNew);
    leftLayout->addWidget(btnEdit);
    leftLayout->addWidget(btnDel);

    // ================= 右侧：Tabs =================
    tabs_ = new QTabWidget;
    tabs_->setDocumentMode(true);

    // ---------- Tab0 主页 ----------
    auto makeHomeCard = [](const QString &title, QLabel **outText) -> QFrame * {
        auto *card = new QFrame;
        card->setObjectName("homeCard");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 12, 16, 14);
        cl->setSpacing(6);
        auto *t = new QLabel(title);
        t->setObjectName("homeCardTitle");
        auto *txt = new QLabel(QStringLiteral("--"));
        txt->setObjectName("homeCardText");
        txt->setTextFormat(Qt::RichText);
        txt->setWordWrap(true);
        txt->setTextInteractionFlags(Qt::TextSelectableByMouse);
        cl->addWidget(t);
        cl->addWidget(txt);
        *outText = txt;
        return card;
    };

    auto *homeTab = new QWidget;
    auto *homeLay = new QVBoxLayout(homeTab);
    homeLay->setContentsMargins(16, 16, 16, 16);
    homeLay->setSpacing(12);

    QFrame *homeUserCard, *homeBmiCard, *homeExCard, *homeMealCard, *homeWeekCard;
    homeUserCard = makeHomeCard(QStringLiteral("用户概览"), &lblHomeUser_);
    homeBmiCard = makeHomeCard(QStringLiteral("身体指标"), &lblHomeBmi_);
    homeExCard = makeHomeCard(QStringLiteral("今日运动处方"), &lblHomeExercise_);
    homeMealCard = makeHomeCard(QStringLiteral("今日食谱"), &lblHomeMeals_);
    homeWeekCard = makeHomeCard(QStringLiteral("周计划"), &lblHomeWeek_);

    homeLay->addWidget(homeUserCard);
    homeLay->addWidget(homeBmiCard);
    homeLay->addWidget(homeExCard);
    homeLay->addWidget(homeMealCard);
    homeLay->addWidget(homeWeekCard);
    homeLay->addStretch(1);

    auto *homeScroll = new QScrollArea;
    homeScroll->setWidgetResizable(true);
    homeScroll->setFrameShape(QFrame::NoFrame);
    homeScroll->setWidget(homeTab);
    tabs_->addTab(homeScroll, QStringLiteral("主页"));

    // ---------- Tab1 身体指标 ----------
    auto *overviewTab = new QWidget;
    auto *overviewLay = new QVBoxLayout(overviewTab);
    overviewLay->setContentsMargins(16, 16, 16, 16);
    overviewLay->setSpacing(12);

    // 个人信息模块（橘黄，6 个小卡）
    auto *infoCard = new QFrame;
    infoCard->setObjectName("infoCard");
    auto *infoCardLay = new QVBoxLayout(infoCard);
    infoCardLay->setContentsMargins(18, 14, 18, 16);
    infoCardLay->setSpacing(10);
    auto *infoTitle = new QLabel(QStringLiteral("个人信息"));
    infoTitle->setObjectName("sectionTitle");
    infoCardLay->addWidget(infoTitle);
    auto *infoGrid = new QGridLayout;
    infoGrid->setSpacing(10);
    for (int i = 0; i < kInfoLabels.size(); ++i) {
        auto *miniCard = new QFrame;
        miniCard->setObjectName("infoMiniCard");   // 淡绿小卡
        auto *ml = new QVBoxLayout(miniCard);
        ml->setContentsMargins(14, 10, 14, 10);
        ml->setSpacing(2);
        auto *lab = new QLabel(kInfoLabels.at(i));
        lab->setObjectName("infoLabel");
        auto *val = new QLabel(QStringLiteral("--"));
        val->setObjectName("infoValue");
        ml->addWidget(lab);
        ml->addWidget(val);
        infoGrid->addWidget(miniCard, i / 3, i % 3);
        infoValueLabels_.append(val);
    }
    infoCardLay->addLayout(infoGrid);
    overviewLay->addWidget(infoCard);

    // BMI 卡（灰）
    auto *bmiCard = new QFrame;
    bmiCard->setObjectName("metricCard");
    auto *bmiLay = new QHBoxLayout(bmiCard);
    bmiLay->setContentsMargins(20, 16, 20, 16);
    bmiLay->setSpacing(16);
    auto *bmiLeft = new QVBoxLayout;
    bmiLeft->setSpacing(2);
    auto *bmiLabel = new QLabel(QStringLiteral("BMI 身体质量指数"));
    bmiLabel->setObjectName("metricLabel");
    lblBmiValue_ = new QLabel(QStringLiteral("--"));
    lblBmiValue_->setObjectName("bmiValue");
    bmiLeft->addWidget(bmiLabel);
    bmiLeft->addWidget(lblBmiValue_);
    lblBmiCategory_ = new QLabel(QStringLiteral("--"));
    lblBmiCategory_->setObjectName("bmiTag");
    lblBmiCategory_->setAlignment(Qt::AlignCenter);
    auto *bmiHint = new QLabel(QStringLiteral("偏瘦 <18.5 · 正常 18.5~24 · 超重 24~28 · 肥胖 ≥28"));
    bmiHint->setObjectName("metricLabel");
    auto *bmiRight = new QVBoxLayout;
    bmiRight->setSpacing(8);
    bmiRight->addWidget(lblBmiCategory_, 0, Qt::AlignVCenter);
    bmiRight->addWidget(bmiHint);
    bmiLay->addLayout(bmiLeft);
    bmiLay->addStretch(1);
    bmiLay->addLayout(bmiRight);
    overviewLay->addWidget(bmiCard);

    // 四个指数卡（灰）
    auto makeMetric = [](const QString &label) {
        auto *card = new QFrame;
        card->setObjectName("metricCard");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 16, 18, 16);
        cl->setSpacing(4);
        auto *title = new QLabel(label);
        title->setObjectName("metricLabel");
        auto *value = new QLabel(QStringLiteral("--"));
        value->setObjectName("metricValue");
        cl->addWidget(title);
        cl->addWidget(value);
        return std::pair<QFrame *, QLabel *>(card, value);
    };
    auto bmrPair = makeMetric(QStringLiteral("基础代谢率 BMR"));
    lblBmrValue_ = bmrPair.second;
    auto tdeePair = makeMetric(QStringLiteral("总能量消耗 TDEE"));
    lblTdeeValue_ = tdeePair.second;
    auto intakePair = makeMetric(QStringLiteral("每日推荐摄入热量"));
    lblIntakeValue_ = intakePair.second;
    auto exPair = makeMetric(QStringLiteral("每日运动消耗目标"));
    lblExTargetValue_ = exPair.second;

    auto *metricsGrid = new QGridLayout;
    metricsGrid->setSpacing(12);
    metricsGrid->addWidget(bmrPair.first, 0, 0);
    metricsGrid->addWidget(tdeePair.first, 0, 1);
    metricsGrid->addWidget(intakePair.first, 1, 0);
    metricsGrid->addWidget(exPair.first, 1, 1);
    overviewLay->addLayout(metricsGrid);
    overviewLay->addStretch(1);

    auto *overviewScroll = new QScrollArea;
    overviewScroll->setWidgetResizable(true);
    overviewScroll->setFrameShape(QFrame::NoFrame);
    overviewScroll->setWidget(overviewTab);
    tabs_->addTab(overviewScroll, QStringLiteral("身体指标"));

    // ---------- Tab2 运动处方 ----------
    auto *exTab = new QWidget;
    exerciseTable_ = new QTableWidget;
    exerciseTable_->setColumnCount(5);
    exerciseTable_->setHorizontalHeaderLabels(
        {QStringLiteral("运动名称"), QStringLiteral("MET"), QStringLiteral("时长(分钟)"),
         QStringLiteral("消耗(kcal)"), QStringLiteral("反馈")});
    exerciseTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    exerciseTable_->verticalHeader()->setVisible(false);
    exerciseTable_->setSelectionMode(QAbstractItemView::NoSelection);
    exerciseTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    exerciseTable_->setFocusPolicy(Qt::NoFocus);
    exerciseTable_->setAlternatingRowColors(true);
    exerciseTable_->setShowGrid(false);

    lblExerciseTotal_ = new QLabel;
    lblExerciseTotal_->setObjectName("summaryLabel");
    auto *btnRegenEx = new QPushButton(QStringLiteral("重新生成运动处方"));
    btnRegenEx->setObjectName("accentBtn");

    auto *exLay = new QVBoxLayout(exTab);
    exLay->setContentsMargins(16, 16, 16, 16);
    exLay->setSpacing(12);
    exLay->addWidget(exerciseTable_, 1);
    exLay->addWidget(lblExerciseTotal_);
    exLay->addWidget(btnRegenEx, 0, Qt::AlignRight);
    tabs_->addTab(exTab, QStringLiteral("运动处方"));

    // ---------- Tab3 食谱推荐 ----------
    auto *mealTab = new QWidget;
    auto *mealLay = new QVBoxLayout(mealTab);
    mealLay->setContentsMargins(16, 16, 16, 16);
    mealLay->setSpacing(10);
    for (int i = 0; i < 4; ++i) {
        auto *group = new QGroupBox(kMealNames.at(i));
        group->setStyleSheet(QStringLiteral(
            "QGroupBox { background-color: %1; border: 2px solid %2; border-radius: 12px;"
            " margin-top: 14px; padding-top: 8px; font-weight: 700; color: %3; font-size: 17px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 8px; color: %3;"
            " font-size: 17px; font-weight: 700; }")
            .arg(QString::fromLatin1(kMealColors[i].bg))
            .arg(QString::fromLatin1(kMealColors[i].border))
            .arg(QString::fromLatin1(kMealColors[i].title)));
        auto *gl = new QVBoxLayout(group);
        gl->setContentsMargins(10, 10, 10, 10);
        auto *t = new QTableWidget;
        t->setColumnCount(4);
        t->setHorizontalHeaderLabels(
            {QStringLiteral("菜品"), QStringLiteral("食材"), QStringLiteral("热量(kcal)"), QStringLiteral("反馈")});
        t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        t->verticalHeader()->setVisible(false);
        t->setSelectionMode(QAbstractItemView::NoSelection);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setFocusPolicy(Qt::NoFocus);
        t->setMinimumHeight(96);
        t->setShowGrid(false);
        // 表格背景与模块外框颜色一致，文字放大加粗
        t->setStyleSheet(QStringLiteral(
            "QTableWidget { background-color: %1; border: none; gridline-color: transparent; font-size: 15px; }"
            "QTableWidget::item { color: #212121; padding: 6px 8px; font-weight: 700; }"
            "QTableWidget::item:selected { background-color: %2; }"
            "QHeaderView::section { background-color: %1; color: %3; border: none;"
            " border-bottom: 1px solid %2; padding: 8px; font-weight: 700; font-size: 15px; }")
            .arg(QString::fromLatin1(kMealColors[i].bg))
            .arg(QString::fromLatin1(kMealColors[i].border))
            .arg(QString::fromLatin1(kMealColors[i].title)));
        gl->addWidget(t);
        mealLay->addWidget(group);
        mealTables_.append(t);
        mealGroups_.append(group);
    }
    lblMealTotal_ = new QLabel;
    lblMealTotal_->setObjectName("summaryLabel");
    auto *btnRegenMeal = new QPushButton(QStringLiteral("重新生成一日食谱"));
    btnRegenMeal->setObjectName("accentBtn");
    mealLay->addWidget(lblMealTotal_);
    mealLay->addWidget(btnRegenMeal, 0, Qt::AlignRight);
    tabs_->addTab(mealTab, QStringLiteral("食谱推荐"));

    // ---------- Tab4 周计划 ----------
    auto *weekTab = new QWidget;
    weekScroll_ = new QScrollArea;
    weekScroll_->setWidgetResizable(true);
    weekScroll_->setFrameShape(QFrame::NoFrame);
    weekContainer_ = new QWidget;
    weekLayout_ = new QVBoxLayout(weekContainer_);
    weekLayout_->setContentsMargins(4, 4, 4, 4);
    weekLayout_->setSpacing(12);

    lblEmptyWeek_ = new QLabel(QStringLiteral("尚未生成周计划，点击下方「生成一周计划」。"));
    lblEmptyWeek_->setObjectName("metricLabel");
    lblEmptyWeek_->setAlignment(Qt::AlignCenter);
    lblEmptyWeek_->setMinimumHeight(120);
    weekLayout_->addWidget(lblEmptyWeek_);
    weekLayout_->addStretch(1);
    weekScroll_->setWidget(weekContainer_);

    lblWeekSummary_ = new QLabel;
    lblWeekSummary_->setObjectName("summaryLabel");

    auto *btnGenWeek = new QPushButton(QStringLiteral("生成一周计划"));
    btnGenWeek->setObjectName("accentBtn");
    auto *btnExport = new QPushButton(QStringLiteral("导出周计划"));

    auto *weekBtnRow = new QHBoxLayout;
    weekBtnRow->addWidget(lblWeekSummary_, 1);
    weekBtnRow->addWidget(btnGenWeek);
    weekBtnRow->addWidget(btnExport);

    auto *weekLay = new QVBoxLayout(weekTab);
    weekLay->setContentsMargins(16, 16, 16, 16);
    weekLay->setSpacing(12);
    weekLay->addWidget(weekScroll_, 1);
    weekLay->addLayout(weekBtnRow);
    tabs_->addTab(weekTab, QStringLiteral("周计划"));

    splitter->addWidget(leftWidget);
    splitter->addWidget(tabs_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({280, 880});

    rootLayout->addWidget(splitter);
    setCentralWidget(central);

    // ---------------- 信号连接 ----------------
    connect(userSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserChanged);
    connect(btnNew, &QPushButton::clicked, this, &MainWindow::onNewUser);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditUser);
    connect(btnDel, &QPushButton::clicked, this, &MainWindow::onDeleteUser);

    connect(btnRegenEx, &QPushButton::clicked, this, &MainWindow::onRegenerateExercise);
    connect(btnRegenMeal, &QPushButton::clicked, this, &MainWindow::onRegenerateMeals);

    connect(btnGenWeek, &QPushButton::clicked, this, &MainWindow::onGenerateWeeklyPlan);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportPlan);
}

// ---------------------------------------------------------------------------
// 数据持久化
// ---------------------------------------------------------------------------

QString MainWindow::dataFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/data.json");
}

void MainWindow::loadDataOrDefault()
{
    QString path = dataFilePath();
    QString err;
    if (QFile::exists(path) && store_.loadFromFile(path, &err)) {
        // 成功加载
    } else {
        store_.fillInitialData();
        store_.saveToFile(path);
    }
}

bool MainWindow::saveData()
{
    return store_.saveToFile(dataFilePath());
}

UserProfile *MainWindow::currentUser()
{
    return store_.findUser(store_.currentUserId);
}

const UserProfile *MainWindow::currentUser() const
{
    return store_.findUser(store_.currentUserId);
}

// ---------------------------------------------------------------------------
// 用户管理（折叠式下拉框）
// ---------------------------------------------------------------------------

void MainWindow::refreshUserSelector()
{
    QSignalBlocker blocker(userSelector_);
    userSelector_->clear();
    for (const UserProfile &u : store_.users)
        userSelector_->addItem(u.name, u.id);
    int idx = userSelector_->findData(store_.currentUserId);
    if (idx >= 0)
        userSelector_->setCurrentIndex(idx);
}

void MainWindow::refreshUserCard()
{
    const UserProfile *u = currentUser();
    if (!u) {
        lblUserCard_->setText(QString());
        return;
    }
    QString gender = (u->gender == 'M') ? QStringLiteral("男") : QStringLiteral("女");
    QString goal;
    if (u->goalType == "lose")      goal = QStringLiteral("减重");
    else if (u->goalType == "gain") goal = QStringLiteral("增重");
    else                            goal = QStringLiteral("维持体重");

    lblUserCard_->setText(QStringLiteral(
        "<div style='font-size:23px;font-weight:700;color:#1b5e20;'>%1</div>"
        "<div style='font-size:15px;color:#2e7d32;font-weight:600;margin-top:14px;'>%2 · %3 岁</div>"
        "<div style='font-size:15px;color:#2e7d32;font-weight:600;margin-top:10px;'>%4 cm · %5 kg</div>"
        "<div style='font-size:15px;color:#2e7d32;font-weight:600;margin-top:10px;'>目标 %6 kg · %7</div>"
        "<div style='font-size:19px;font-weight:700;color:#2e7d32;margin-top:14px;'>BMI %8（%9）</div>")
        .arg(u->name).arg(gender).arg(u->age)
        .arg(u->height).arg(u->weight).arg(u->targetWeight).arg(goal)
        .arg(QString::number(u->calculateBMI(), 'f', 1)).arg(u->bmiCategory()));
}

void MainWindow::onUserChanged(int index)
{
    if (index < 0)
        return;
    QString id = userSelector_->itemData(index).toString();
    if (id == store_.currentUserId)
        return;
    store_.currentUserId = id;
    refreshAll();
}

void MainWindow::onNewUser()
{
    UserEditDialog dlg(nullptr, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    UserProfile u = dlg.resultUser();
    u.id = store_.nextUserId();
    store_.users.append(u);
    store_.currentUserId = u.id;
    saveData();
    refreshUserSelector();
    refreshAll();
}

void MainWindow::onEditUser()
{
    UserProfile *u = currentUser();
    if (!u)
        return;
    UserEditDialog dlg(u, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    UserProfile nu = dlg.resultUser();
    nu.id = u->id;
    *u = nu;
    saveData();
    refreshUserSelector();
    refreshAll();
}

void MainWindow::onDeleteUser()
{
    UserProfile *u = currentUser();
    if (!u)
        return;
    if (store_.users.size() <= 1) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("至少保留一个用户"));
        return;
    }
    if (QMessageBox::question(this, QStringLiteral("确认"),
            QStringLiteral("确定删除用户 %1 吗？").arg(u->name),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    for (int i = 0; i < store_.users.size(); ++i) {
        if (store_.users.at(i).id == u->id) {
            store_.users.removeAt(i);
            break;
        }
    }
    store_.currentUserId = store_.users.first().id;
    saveData();
    refreshUserSelector();
    refreshAll();
}

// ---------------------------------------------------------------------------
// 数据管理
// ---------------------------------------------------------------------------

void MainWindow::onManageData()
{
    DataManagerDialog dlg(&store_, this);
    dlg.exec();
    saveData();
    refreshAll();
}

void MainWindow::onOpenData()
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("打开数据文件"),
        QString(), QStringLiteral("JSON 文件 (*.json)"));
    if (path.isEmpty())
        return;
    QString err;
    if (!store_.loadFromFile(path, &err)) {
        QMessageBox::warning(this, QStringLiteral("加载失败"), err);
        return;
    }
    refreshUserSelector();
    refreshAll();
}

void MainWindow::onSaveData()
{
    if (saveData())
        QMessageBox::information(this, QStringLiteral("保存成功"),
            QStringLiteral("数据已保存到：\n%1").arg(dataFilePath()));
    else
        QMessageBox::warning(this, QStringLiteral("保存失败"), QStringLiteral("无法写入文件"));
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, QStringLiteral("关于"),
        QStringLiteral("<b>面向减重的运动处方与食谱推荐智能体</b><br><br>"
            "基于 C++ + Qt 的个性化减重辅助系统。<br>"
            "实现：BMR/TDEE 计算、运动处方生成、食谱推荐、周计划管理与用户反馈。"));
}

// ---------------------------------------------------------------------------
// 主页（整合四个页面信息）
// ---------------------------------------------------------------------------

void MainWindow::refreshHome()
{
    const UserProfile *u = currentUser();
    if (!u) {
        lblHomeUser_->setText(QStringLiteral("请在左侧选择用户"));
        lblHomeBmi_->setText(QString());
        lblHomeExercise_->setText(QString());
        lblHomeMeals_->setText(QString());
        lblHomeWeek_->setText(QString());
        return;
    }

    QString gender = (u->gender == 'M') ? QStringLiteral("男") : QStringLiteral("女");
    QString goal;
    if (u->goalType == "lose")      goal = QStringLiteral("减重");
    else if (u->goalType == "gain") goal = QStringLiteral("增重");
    else                            goal = QStringLiteral("维持体重");

    lblHomeUser_->setText(QStringLiteral(
        "<span style='font-size:18px;font-weight:700;color:#3e2723;'>%1</span>　"
        "<span style='font-size:13px;font-weight:600;color:#5d4037;'>%2 · %3岁 · %4cm · %5kg</span><br>"
        "<span style='font-size:13px;font-weight:600;color:#5d4037;'>目标体重 %6kg · %7</span>")
        .arg(u->name).arg(gender).arg(u->age).arg(u->height).arg(u->weight)
        .arg(u->targetWeight).arg(goal));

    AgentCore::CalorieNeed need = agent_.calculateCalorieNeeds(*u);
    double bmi = u->calculateBMI();
    QString cat = u->bmiCategory();
    QString mainColor;
    if (bmi < 18.5)      mainColor = "#1565c0";
    else if (bmi < 24.0) mainColor = "#2e7d32";
    else if (bmi < 28.0) mainColor = "#e65100";
    else                 mainColor = "#c62828";

    lblHomeBmi_->setText(QStringLiteral(
        "<span style='font-size:26px;font-weight:700;color:%1;'>BMI %2</span>　"
        "<span style='font-size:14px;font-weight:700;color:%1;'>%3</span><br>"
        "<span style='font-size:13px;font-weight:600;color:#212121;'>BMR %4 · TDEE %5 kcal/天</span><br>"
        "<span style='font-size:13px;font-weight:600;color:#212121;'>推荐摄入 %6 · 运动目标 %7 kcal/天</span>")
        .arg(mainColor).arg(QString::number(bmi, 'f', 1)).arg(cat)
        .arg(QString::number(need.bmr, 'f', 0)).arg(QString::number(need.tdee, 'f', 0))
        .arg(QString::number(need.recommendedIntake, 'f', 0)).arg(QString::number(need.exerciseTarget, 'f', 0)));

    // 今日运动（用缓存）
    QStringList exItems;
    double exTotal = 0;
    for (const AgentCore::ExercisePlanItem &it : cachedExercisePlan_) {
        exTotal += it.caloriesBurned;
        exItems << QStringLiteral("%1 %2分钟").arg(it.exerciseName).arg(it.durationMinutes);
    }
    lblHomeExercise_->setText(exItems.isEmpty()
        ? QStringLiteral("暂无运动处方")
        : QStringLiteral("<span style='font-size:13px;font-weight:600;color:#2e7d32;'>%1</span><br>"
                         "<span style='font-size:14px;font-weight:700;color:#1b5e20;'>合计消耗 %2 kcal</span>")
              .arg(exItems.join(QStringLiteral("　|　"))).arg(QString::number(exTotal, 'f', 0)));

    // 今日食谱（用缓存）
    double mealTotal = cachedMealPlan_.totalCalories();
    auto mealText = [](const QString &label, const QList<Recipe> &list) {
        if (list.isEmpty())
            return QString();
        QStringList names;
        for (const Recipe &r : list)
            names << r.name;
        return QStringLiteral("<span style='color:#555;'>%1</span> %2").arg(label).arg(names.join(QStringLiteral("、")));
    };
    QStringList mealLines;
    QString b = mealText(QStringLiteral("早餐"), cachedMealPlan_.breakfast);
    QString l = mealText(QStringLiteral("午餐"), cachedMealPlan_.lunch);
    QString d = mealText(QStringLiteral("晚餐"), cachedMealPlan_.dinner);
    QString s = mealText(QStringLiteral("加餐"), cachedMealPlan_.snacks);
    if (!b.isEmpty()) mealLines << b;
    if (!l.isEmpty()) mealLines << l;
    if (!d.isEmpty()) mealLines << d;
    if (!s.isEmpty()) mealLines << s;
    lblHomeMeals_->setText(mealLines.isEmpty()
        ? QStringLiteral("暂无食谱")
        : QStringLiteral("<span style='font-size:13px;font-weight:600;color:#2e7d32;'>%1</span><br>"
                         "<span style='font-size:14px;font-weight:700;color:#1b5e20;'>全天摄入 %2 kcal</span>")
              .arg(mealLines.join(QStringLiteral("<br>"))).arg(QString::number(mealTotal, 'f', 0)));

    // 周计划状态
    auto it = weeklyPlanCache_.find(u->id);
    if (it != weeklyPlanCache_.end() && !it.value().days.isEmpty()) {
        lblHomeWeek_->setText(QStringLiteral(
            "<span style='font-size:13px;font-weight:600;color:#2e7d32;'>已生成 %1 天计划</span><br>"
            "<span style='font-size:14px;font-weight:700;color:#1b5e20;'>周消耗 %2 · 周摄入 %3 kcal</span>")
            .arg(it.value().days.size())
            .arg(QString::number(it.value().totalCaloriesOut(), 'f', 0))
            .arg(QString::number(it.value().totalCaloriesIn(), 'f', 0)));
    } else {
        lblHomeWeek_->setText(QStringLiteral("<span style='font-size:13px;font-weight:600;color:#888;'>尚未生成，可在「周计划」页一键生成</span>"));
    }
}

// ---------------------------------------------------------------------------
// 身体指标
// ---------------------------------------------------------------------------

void MainWindow::refreshOverview()
{
    const UserProfile *u = currentUser();
    if (!u) {
        for (QLabel *l : infoValueLabels_)
            l->setText(QStringLiteral("--"));
        lblBmiValue_->setText(QStringLiteral("--"));
        lblBmiCategory_->setText(QStringLiteral("--"));
        lblBmrValue_->setText(QStringLiteral("--"));
        lblTdeeValue_->setText(QStringLiteral("--"));
        lblIntakeValue_->setText(QStringLiteral("--"));
        lblExTargetValue_->setText(QStringLiteral("--"));
        return;
    }

    AgentCore::CalorieNeed need = agent_.calculateCalorieNeeds(*u);
    QString gender = (u->gender == 'M') ? QStringLiteral("男") : QStringLiteral("女");
    QString goal;
    if (u->goalType == "lose")      goal = QStringLiteral("减重");
    else if (u->goalType == "gain") goal = QStringLiteral("增重");
    else                            goal = QStringLiteral("维持体重");

    // 6 个个人信息小卡
    if (infoValueLabels_.size() >= 6) {
        infoValueLabels_[0]->setText(u->name);
        infoValueLabels_[1]->setText(gender);
        infoValueLabels_[2]->setText(QStringLiteral("%1 岁").arg(u->age));
        infoValueLabels_[3]->setText(QStringLiteral("%1 cm").arg(u->height));
        infoValueLabels_[4]->setText(QStringLiteral("%1 kg").arg(u->weight));
        infoValueLabels_[5]->setText(QStringLiteral("%1 kg").arg(u->targetWeight));
    }

    // BMI
    double bmi = u->calculateBMI();
    QString cat = u->bmiCategory();
    QString mainColor, softBg;
    if (bmi < 18.5)      { mainColor = "#1565c0"; softBg = "#bbdefb"; }
    else if (bmi < 24.0) { mainColor = "#2e7d32"; softBg = "#c8e6c9"; }
    else if (bmi < 28.0) { mainColor = "#e65100"; softBg = "#ffe0b2"; }
    else                 { mainColor = "#c62828"; softBg = "#ffcdd2"; }
    lblBmiValue_->setText(QString::number(bmi, 'f', 1));
    lblBmiValue_->setStyleSheet(QStringLiteral("font-size:44px;font-weight:700;color:%1;").arg(mainColor));
    lblBmiCategory_->setText(cat);
    lblBmiCategory_->setStyleSheet(QStringLiteral(
        "background-color:%1;color:%2;border-radius:16px;padding:7px 18px;font-size:15px;font-weight:700;")
        .arg(softBg).arg(mainColor));

    auto fmt = [](const QString &v) {
        return QStringLiteral("<span style='font-size:24px;font-weight:700;color:#212121;'>%1</span>"
                              "<span style='font-size:13px;color:#616161;'> kcal/天</span>").arg(v);
    };
    for (QLabel *l : {lblBmrValue_, lblTdeeValue_, lblIntakeValue_, lblExTargetValue_})
        l->setTextFormat(Qt::RichText);
    lblBmrValue_->setText(fmt(QString::number(need.bmr, 'f', 0)));
    lblTdeeValue_->setText(fmt(QString::number(need.tdee, 'f', 0)));
    lblIntakeValue_->setText(fmt(QString::number(need.recommendedIntake, 'f', 0)));
    lblExTargetValue_->setText(fmt(QString::number(need.exerciseTarget, 'f', 0)));
}

// ---------------------------------------------------------------------------
// 运动处方
// ---------------------------------------------------------------------------

void MainWindow::refreshExercisePlan()
{
    exerciseTable_->setRowCount(0);
    cachedExercisePlan_.clear();
    const UserProfile *u = currentUser();
    if (!u) {
        lblExerciseTotal_->setText(QString());
        return;
    }
    AgentCore::CalorieNeed need = agent_.calculateCalorieNeeds(*u);
    cachedExercisePlan_ = agent_.generateExercisePrescription(*u, need.exerciseTarget, store_.exercises);

    double total = 0;
    exerciseTable_->setRowCount(cachedExercisePlan_.size());
    for (int i = 0; i < cachedExercisePlan_.size(); ++i) {
        const AgentCore::ExercisePlanItem &it = cachedExercisePlan_.at(i);
        total += it.caloriesBurned;

        auto *nameItem = new QTableWidgetItem(it.exerciseName);
        nameItem->setData(Qt::UserRole, it.exerciseId);
        exerciseTable_->setItem(i, 0, nameItem);
        exerciseTable_->setItem(i, 1, new QTableWidgetItem(QString::number(it.metValue, 'f', 1)));
        exerciseTable_->setItem(i, 2, new QTableWidgetItem(QString::number(it.durationMinutes)));
        exerciseTable_->setItem(i, 3, new QTableWidgetItem(QString::number(it.caloriesBurned, 'f', 1)));

        auto *w = new QWidget;
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(2, 2, 2, 2);
        h->setSpacing(6);
        auto *like = new QPushButton(QStringLiteral("喜欢"));
        auto *dislike = new QPushButton(QStringLiteral("不喜欢"));
        like->setObjectName("fbLike");
        dislike->setObjectName("fbDislike");
        like->setProperty("exId", it.exerciseId);
        dislike->setProperty("exId", it.exerciseId);
        like->setProperty("kind", "like");
        dislike->setProperty("kind", "dislike");
        connect(like, &QPushButton::clicked, this, &MainWindow::onExerciseFeedback);
        connect(dislike, &QPushButton::clicked, this, &MainWindow::onExerciseFeedback);
        h->addWidget(like);
        h->addWidget(dislike);
        exerciseTable_->setCellWidget(i, 4, w);
        exerciseTable_->resizeRowToContents(i);
    }

    QString color = (total >= need.exerciseTarget) ? "#2e7d32" : "#c62828";
    lblExerciseTotal_->setText(QStringLiteral(
        "<b>运动消耗合计：<span style='color:%1;'>%2 kcal</span>（目标 ≥ %3 kcal）</b>")
        .arg(color).arg(QString::number(total, 'f', 0)).arg(QString::number(need.exerciseTarget, 'f', 0)));
}

// ---------------------------------------------------------------------------
// 食谱推荐
// ---------------------------------------------------------------------------

void MainWindow::refreshMealPlan()
{
    for (QTableWidget *t : mealTables_)
        t->setRowCount(0);

    const UserProfile *u = currentUser();
    if (!u) {
        lblMealTotal_->setText(QString());
        return;
    }
    AgentCore::CalorieNeed need = agent_.calculateCalorieNeeds(*u);
    cachedMealPlan_ = agent_.generateMealPlan(
        need.recommendedIntake, store_.recipes,
        u->dislikedRecipeIds, u->likedRecipeIds);

    auto fillTable = [&](QTableWidget *table, const QList<Recipe> &list) {
        table->setRowCount(list.size());
        for (int i = 0; i < list.size(); ++i) {
            const Recipe &r = list.at(i);
            auto *nameItem = new QTableWidgetItem(r.name);
            nameItem->setData(Qt::UserRole, r.id);
            table->setItem(i, 0, nameItem);
            table->setItem(i, 1, new QTableWidgetItem(r.ingredients));
            table->setItem(i, 2, new QTableWidgetItem(QString::number(r.totalCalories, 'f', 0)));

            auto *w = new QWidget;
            auto *h = new QHBoxLayout(w);
            h->setContentsMargins(2, 2, 2, 2);
            h->setSpacing(6);
            auto *like = new QPushButton(QStringLiteral("喜欢"));
            auto *dislike = new QPushButton(QStringLiteral("不喜欢"));
            like->setObjectName("fbLike");
            dislike->setObjectName("fbDislike");
            like->setProperty("recId", r.id);
            dislike->setProperty("recId", r.id);
            like->setProperty("kind", "like");
            dislike->setProperty("kind", "dislike");
            connect(like, &QPushButton::clicked, this, &MainWindow::onRecipeFeedback);
            connect(dislike, &QPushButton::clicked, this, &MainWindow::onRecipeFeedback);
            h->addWidget(like);
            h->addWidget(dislike);
            table->setCellWidget(i, 3, w);
            table->resizeRowToContents(i);
        }
    };
    fillTable(mealTables_[0], cachedMealPlan_.breakfast);
    fillTable(mealTables_[1], cachedMealPlan_.lunch);
    fillTable(mealTables_[2], cachedMealPlan_.dinner);
    fillTable(mealTables_[3], cachedMealPlan_.snacks);

    double total = cachedMealPlan_.totalCalories();
    double target = need.recommendedIntake;
    double low = target * 0.9;
    double high = target * 1.1;
    QString color = (total >= low && total <= high) ? "#2e7d32" : "#e65100";
    lblMealTotal_->setText(QStringLiteral(
        "<b>全天摄入合计：<span style='color:%1;'>%2 kcal</span>（推荐 %3 kcal，范围 ±10%）</b>")
        .arg(color).arg(QString::number(total, 'f', 0)).arg(QString::number(target, 'f', 0)));
}

// ---------------------------------------------------------------------------
// 周计划
// ---------------------------------------------------------------------------

QString MainWindow::buildDayDetailText(const DailyPlan &day) const
{
    QString s;
    s += QStringLiteral("<p style='margin:0 0 4px 0;color:#1b5e20;font-weight:700;'>运动处方 · 消耗 %1 kcal</p>")
             .arg(QString::number(day.totalCaloriesOut(), 'f', 0));
    if (day.exercises.isEmpty()) {
        s += QStringLiteral("<p style='margin:0 0 8px 0;color:#888;'>（无）</p>");
    } else {
        QStringList items;
        for (const PlannedExercise &e : day.exercises)
            items << QStringLiteral("%1 · %2 分钟 · ≈%3 kcal")
                        .arg(e.name).arg(e.durationMinutes).arg(QString::number(e.caloriesBurned, 'f', 0));
        s += QStringLiteral("<p style='margin:0 0 8px 0;color:#2e7d32;font-weight:600;'>%1</p>")
                 .arg(items.join(QStringLiteral("　|　")));
    }

    auto mealLine = [](const QString &label, const QList<PlannedRecipe> &list) {
        if (list.isEmpty())
            return QString();
        QStringList names;
        for (const PlannedRecipe &r : list)
            names << QStringLiteral("%1(%2kcal)").arg(r.name).arg(QString::number(r.totalCalories, 'f', 0));
        return QStringLiteral("<p style='margin:3px 0;color:#2e7d32;font-weight:600;'><span style='color:#555;'>%1</span>　%2</p>")
                   .arg(label).arg(names.join(QStringLiteral("、")));
    };
    s += mealLine(QStringLiteral("早餐"), day.breakfast);
    s += mealLine(QStringLiteral("午餐"), day.lunch);
    s += mealLine(QStringLiteral("晚餐"), day.dinner);
    s += mealLine(QStringLiteral("加餐"), day.snacks);

    s += QStringLiteral("<p style='margin:8px 0 0 0;color:#555;font-size:12px;font-weight:600;'>全天摄入 %1 kcal − 消耗 %2 kcal = 净热量 %3 kcal</p>")
             .arg(QString::number(day.totalCaloriesIn(), 'f', 0))
             .arg(QString::number(day.totalCaloriesOut(), 'f', 0))
             .arg(QString::number(day.netCalories(), 'f', 0));
    return s;
}

void MainWindow::refreshWeeklyPlan()
{
    for (QGroupBox *g : weekGroups_) {
        weekLayout_->removeWidget(g);
        g->deleteLater();
    }
    weekGroups_.clear();

    auto it = weeklyPlanCache_.find(store_.currentUserId);
    if (it == weeklyPlanCache_.end() || it.value().days.isEmpty()) {
        lblEmptyWeek_->setVisible(true);
        lblWeekSummary_->setText(QString());
        return;
    }

    const WeeklyPlan &plan = it.value();
    lblEmptyWeek_->setVisible(false);
    for (const DailyPlan &d : plan.days) {
        auto *group = new QGroupBox(QStringLiteral("%1 · 消耗 %2 / 摄入 %3 kcal")
            .arg(d.date.toString(QStringLiteral("MM月dd日 dddd")))
            .arg(QString::number(d.totalCaloriesOut(), 'f', 0))
            .arg(QString::number(d.totalCaloriesIn(), 'f', 0)));
        auto *gl = new QVBoxLayout(group);
        gl->setContentsMargins(16, 14, 16, 14);
        auto *content = new QLabel(buildDayDetailText(d));
        content->setTextFormat(Qt::RichText);
        content->setWordWrap(true);
        content->setTextInteractionFlags(Qt::TextSelectableByMouse);
        gl->addWidget(content);
        weekLayout_->insertWidget(weekLayout_->count() - 1, group);
        weekGroups_.append(group);
    }

    lblWeekSummary_->setText(QStringLiteral(
        "<b>一周合计：</b>运动消耗 %1 kcal　/　饮食摄入 %2 kcal")
        .arg(QString::number(plan.totalCaloriesOut(), 'f', 0))
        .arg(QString::number(plan.totalCaloriesIn(), 'f', 0)));
}

void MainWindow::refreshAll()
{
    refreshUserSelector();
    refreshUserCard();
    refreshOverview();
    refreshExercisePlan();
    refreshMealPlan();
    refreshWeeklyPlan();
    refreshHome();
}

// ---------------------------------------------------------------------------
// 推荐重生成 / 周计划
// ---------------------------------------------------------------------------

void MainWindow::onRegenerateExercise()
{
    refreshExercisePlan();
    refreshHome();
}

void MainWindow::onRegenerateMeals()
{
    refreshMealPlan();
    refreshHome();
}

void MainWindow::onRegenerateAll()
{
    refreshExercisePlan();
    refreshMealPlan();
    refreshHome();
}

void MainWindow::onGenerateWeeklyPlan()
{
    const UserProfile *u = currentUser();
    if (!u)
        return;
    weeklyPlanCache_[u->id] = planManager_.generateWeeklyPlan(
        *u, agent_, store_.exercises, store_.recipes, QDate::currentDate());
    refreshWeeklyPlan();
    refreshHome();
}

void MainWindow::onExportPlan()
{
    auto it = weeklyPlanCache_.find(store_.currentUserId);
    if (it == weeklyPlanCache_.end() || it.value().days.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先生成周计划"));
        return;
    }
    QString base = QFileDialog::getSaveFileName(this, QStringLiteral("导出周计划"),
        QStringLiteral("weekly_plan.txt"),
        QStringLiteral("文本文件 (*.txt);;CSV 文件 (*.csv)"));
    if (base.isEmpty())
        return;
    QString err;
    bool ok = base.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)
        ? planManager_.exportToCsv(it.value(), base, &err)
        : planManager_.exportToText(it.value(), base, &err);
    if (ok)
        QMessageBox::information(this, QStringLiteral("导出成功"), QStringLiteral("已导出到：%1").arg(base));
    else
        QMessageBox::warning(this, QStringLiteral("导出失败"), err);
}

// ---------------------------------------------------------------------------
// 反馈
// ---------------------------------------------------------------------------

void MainWindow::applyExerciseFeedback(const QString &id, const QString &kind)
{
    UserProfile *u = currentUser();
    if (!u)
        return;
    if (kind == "like") {
        u->dislikedExerciseIds.removeAll(id);
        if (!u->likedExerciseIds.contains(id))
            u->likedExerciseIds.append(id);
    } else {
        u->likedExerciseIds.removeAll(id);
        if (!u->dislikedExerciseIds.contains(id))
            u->dislikedExerciseIds.append(id);
    }
    saveData();
    refreshExercisePlan();
}

void MainWindow::applyRecipeFeedback(const QString &id, const QString &kind)
{
    UserProfile *u = currentUser();
    if (!u)
        return;
    if (kind == "like") {
        u->dislikedRecipeIds.removeAll(id);
        if (!u->likedRecipeIds.contains(id))
            u->likedRecipeIds.append(id);
    } else {
        u->likedRecipeIds.removeAll(id);
        if (!u->dislikedRecipeIds.contains(id))
            u->dislikedRecipeIds.append(id);
    }
    saveData();
    refreshMealPlan();
}

void MainWindow::onExerciseFeedback()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    applyExerciseFeedback(btn->property("exId").toString(), btn->property("kind").toString());
}

void MainWindow::onRecipeFeedback()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    applyRecipeFeedback(btn->property("recId").toString(), btn->property("kind").toString());
}

// ---------------------------------------------------------------------------
// 关闭时自动保存
// ---------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    event->accept();
}
