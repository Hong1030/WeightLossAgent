#include "MainWindow.h"

#include <QApplication>

// ---------------------------------------------------------------------------
// 全局主题样式：淡绿背景 + 深绿菜单栏 + 橙黄 Tab 栏 + 模块化配色
// ---------------------------------------------------------------------------
static const char *kAppStyle = R"(
* {
    font-family: "Microsoft YaHei UI", "Microsoft YaHei", "Segoe UI Emoji", "Segoe UI", sans-serif;
    font-size: 13px;
}

/* ---------- 全局淡绿色背景 ---------- */
QMainWindow, QDialog {
    background-color: #e8f5e9;
}
QWidget#centralWidget {
    background-color: #e8f5e9;
}

/* ---------- 顶部菜单栏：深绿色 ---------- */
QMenuBar {
    background-color: #1b5e20;
    color: #ffffff;
    border-bottom: 2px solid #144a18;
}
QMenuBar::item {
    padding: 7px 14px;
    border-radius: 4px;
    background: transparent;
}
QMenuBar::item:selected {
    background-color: #2e7d32;
}
QMenuBar::item:pressed {
    background-color: #144a18;
}
QMenu {
    background-color: #ffffff;
    border: 1px solid #c8e6c9;
    border-radius: 8px;
    padding: 4px;
}
QMenu::item {
    padding: 7px 26px;
    border-radius: 6px;
    color: #2e7d32;
}
QMenu::item:selected {
    background-color: #e8f5e9;
    color: #1b5e20;
}

/* ---------- Tab 栏：深绿色（与顶部菜单栏一致） ---------- */
QTabBar {
    background-color: #1b5e20;
}
QTabBar::tab {
    background-color: #2e7d32;
    color: #ffffff;
    padding: 11px 22px;
    margin-right: 3px;
    font-weight: 700;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
}
QTabBar::tab:selected {
    background-color: #43a047;
    color: #ffffff;
}
QTabBar::tab:hover:!selected {
    background-color: #388e3c;
}
QTabWidget::pane {
    background-color: #e8f5e9;
    border: 1px solid #a5d6a7;
    border-radius: 12px;
    top: -1px;
}

/* ---------- 卡片面板 ---------- */
QWidget#cardPanel {
    background-color: #ffffff;
    border: 1px solid #a5d6a7;
    border-radius: 14px;
}

/* ---------- 标题 ---------- */
QLabel#sectionTitle {
    font-size: 17px;
    font-weight: 700;
    color: #1b5e20;
}
QLabel#sectionSubtitle {
    font-size: 12px;
    color: #6b8e6f;
}

/* ---------- 下拉框（用户切换） ---------- */
QComboBox#userSelector {
    background-color: #ffffff;
    border: 1px solid #a5d6a7;
    border-radius: 8px;
    padding: 7px 12px;
    font-size: 14px;
    font-weight: 700;
    color: #1b5e20;
}
QComboBox#userSelector::drop-down {
    border: none;
    width: 26px;
}
QComboBox#userSelector QAbstractItemView {
    background-color: #ffffff;
    border: 1px solid #a5d6a7;
    border-radius: 8px;
    selection-background-color: #e8f5e9;
    selection-color: #1b5e20;
    outline: 0;
}

/* ---------- 用户信息卡片：淡绿色 ---------- */
QFrame#userCard {
    background-color: #e8f5e9;
    border: 2px solid #a5d6a7;
    border-radius: 12px;
}
QLabel#userCardTitle {
    font-size: 18px;
    font-weight: 700;
    color: #5d4037;
}
QLabel#userCardText {
    font-size: 13px;
    color: #6d4c41;
    font-weight: 600;
}

/* ---------- 身体指标：个人信息模块（淡绿）+ 指数模块（灰） ---------- */
QFrame#infoCard {
    background-color: #e8f5e9;
    border: 2px solid #a5d6a7;
    border-radius: 12px;
}
QFrame#infoMiniCard {
    background-color: #e8f5e9;
    border: 1px solid #a5d6a7;
    border-radius: 10px;
}
QLabel#infoValue {
    font-size: 16px;
    font-weight: 700;
    color: #1b5e20;
}
QLabel#infoLabel {
    font-size: 12px;
    color: #2e7d32;
    font-weight: 600;
}
QFrame#metricCard {
    background-color: #f5f5f5;
    border: 1px solid #bdbdbd;
    border-radius: 12px;
}
QLabel#metricLabel {
    font-size: 13px;
    color: #424242;
    font-weight: 600;
}
QLabel#metricValue {
    font-size: 24px;
    font-weight: 700;
    color: #212121;
}
QLabel#bmiValue {
    font-size: 44px;
    font-weight: 700;
}
QLabel#bmiTag {
    border-radius: 16px;
    padding: 7px 18px;
    font-size: 15px;
    font-weight: 700;
}

/* ---------- 主页卡片：浅绿色 ---------- */
QFrame#homeCard {
    background-color: #f1f8e9;
    border: 2px solid #a5d6a7;
    border-radius: 12px;
}
QLabel#homeCardTitle {
    font-size: 15px;
    font-weight: 700;
    color: #1b5e20;
}
QLabel#homeCardText {
    font-size: 13px;
    color: #2e7d32;
    font-weight: 600;
}

/* ---------- 按钮 ---------- */
QPushButton {
    background-color: #ffffff;
    border: 1px solid #a5d6a7;
    border-radius: 9px;
    padding: 8px 16px;
    color: #1b5e20;
    font-weight: 600;
}
QPushButton:hover {
    background-color: #e8f5e9;
    border-color: #43a047;
    color: #1b5e20;
}
QPushButton:pressed {
    background-color: #c8e6c9;
}
QPushButton#accentBtn {
    background-color: #43a047;
    border: 1px solid #2e7d32;
    color: #ffffff;
    font-weight: 700;
}
QPushButton#accentBtn:hover {
    background-color: #2e7d32;
    border-color: #1b5e20;
    color: #ffffff;
}
QPushButton#dangerBtn {
    background-color: #ffffff;
    border: 1px solid #ef9a9a;
    color: #c62828;
    font-weight: 700;
}
QPushButton#dangerBtn:hover {
    background-color: #ffebee;
    border-color: #e53935;
    color: #b71c1c;
}

/* ---------- 反馈按钮：绿色=喜欢，红色=不喜欢，纯文字醒目 ---------- */
QPushButton#fbLike {
    background-color: #43a047;
    border: 1px solid #2e7d32;
    border-radius: 8px;
    padding: 9px 22px;
    color: #ffffff;
    font-size: 16px;
    font-weight: 700;
}
QPushButton#fbLike:hover {
    background-color: #2e7d32;
}
QPushButton#fbDislike {
    background-color: #e53935;
    border: 1px solid #b71c1c;
    border-radius: 8px;
    padding: 9px 22px;
    color: #ffffff;
    font-size: 16px;
    font-weight: 700;
}
QPushButton#fbDislike:hover {
    background-color: #c62828;
}

/* ---------- 表格 ---------- */
QTableWidget {
    background-color: #ffffff;
    border: 1px solid #c8e6c9;
    border-radius: 10px;
    gridline-color: #e8f5e9;
    alternate-background-color: #f1f8e9;
}
QTableWidget::item {
    padding: 6px 8px;
    color: #212121;
}
QTableWidget::item:selected {
    background-color: #c8e6c9;
    color: #1b5e20;
}
QHeaderView::section {
    background-color: #a5d6a7;
    color: #1b5e20;
    font-weight: 700;
    border: none;
    border-bottom: 1px solid #81c784;
    padding: 9px 8px;
}

/* ---------- 分组框（食谱四餐，动态配色） ---------- */
QGroupBox {
    background-color: #ffffff;
    border: 2px solid #a5d6a7;
    border-radius: 12px;
    margin-top: 14px;
    padding-top: 8px;
    font-weight: 700;
    color: #1b5e20;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 14px;
    padding: 0 8px;
    font-weight: 700;
}

/* ---------- 汇总标签 ---------- */
QLabel#summaryLabel {
    background-color: #c8e6c9;
    border: 1px solid #a5d6a7;
    border-radius: 10px;
    padding: 10px 14px;
    color: #1b5e20;
    font-weight: 700;
}

/* ---------- 滚动区域 ---------- */
QScrollArea {
    border: none;
    background: transparent;
}
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: #a5d6a7;
    border-radius: 5px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover {
    background: #81c784;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: transparent;
}

/* ---------- 输入框（对话框） ---------- */
QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {
    background-color: #ffffff;
    border: 1px solid #a5d6a7;
    border-radius: 8px;
    padding: 6px 10px;
    color: #212121;
    selection-background-color: #43a047;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {
    border-color: #43a047;
}
QComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox QAbstractItemView {
    background-color: #ffffff;
    border: 1px solid #c8e6c9;
    border-radius: 8px;
    selection-background-color: #e8f5e9;
    selection-color: #1b5e20;
    outline: 0;
}
)";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("面向减重的运动处方与食谱推荐智能体"));
    app.setOrganizationName(QStringLiteral("CourseDesign"));
    app.setStyleSheet(QString::fromUtf8(kAppStyle));

    MainWindow w;
    w.show();
    return app.exec();
}
