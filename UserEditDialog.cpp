#include "UserEditDialog.h"

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

UserEditDialog::UserEditDialog(const UserProfile *existing, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(existing ? QStringLiteral("编辑用户") : QStringLiteral("新建用户"));
    if (existing)
        base_ = *existing;

    nameEdit_ = new QLineEdit(base_.name.isEmpty() ? QStringLiteral("新用户") : base_.name);
    nameEdit_->setMaxLength(32);

    genderCombo_ = new QComboBox;
    genderCombo_->addItem(QStringLiteral("男"), QStringLiteral("M"));
    genderCombo_->addItem(QStringLiteral("女"), QStringLiteral("F"));
    genderCombo_->setCurrentIndex(base_.gender == 'F' ? 1 : 0);

    ageSpin_ = new QSpinBox;
    ageSpin_->setRange(10, 100);
    ageSpin_->setValue(base_.age >= 10 ? base_.age : 25);

    heightSpin_ = new QDoubleSpinBox;
    heightSpin_->setRange(100, 250);
    heightSpin_->setDecimals(1);
    heightSpin_->setSuffix(QStringLiteral(" cm"));
    heightSpin_->setValue(base_.height > 0 ? base_.height : 170);

    weightSpin_ = new QDoubleSpinBox;
    weightSpin_->setRange(30, 300);
    weightSpin_->setDecimals(1);
    weightSpin_->setSuffix(QStringLiteral(" kg"));
    weightSpin_->setValue(base_.weight > 0 ? base_.weight : 70);

    targetWeightSpin_ = new QDoubleSpinBox;
    targetWeightSpin_->setRange(30, 300);
    targetWeightSpin_->setDecimals(1);
    targetWeightSpin_->setSuffix(QStringLiteral(" kg"));
    targetWeightSpin_->setValue(base_.targetWeight > 0 ? base_.targetWeight : 65);

    activityCombo_ = new QComboBox;
    activityCombo_->addItem(QStringLiteral("1 - 久坐，几乎不运动"), 1);
    activityCombo_->addItem(QStringLiteral("2 - 轻度活动（每周1~3次）"), 2);
    activityCombo_->addItem(QStringLiteral("3 - 中度活动（每周3~5次）"), 3);
    activityCombo_->addItem(QStringLiteral("4 - 高度活动（每周6~7次）"), 4);
    activityCombo_->addItem(QStringLiteral("5 - 极高活动（体力劳动）"), 5);
    int actIdx = activityCombo_->findData(base_.activityLevel);
    activityCombo_->setCurrentIndex(actIdx >= 0 ? actIdx : 1);

    goalCombo_ = new QComboBox;
    goalCombo_->addItem(QStringLiteral("减重"), QStringLiteral("lose"));
    goalCombo_->addItem(QStringLiteral("维持体重"), QStringLiteral("maintain"));
    goalCombo_->addItem(QStringLiteral("增重"), QStringLiteral("gain"));
    int goalIdx = goalCombo_->findData(base_.goalType);
    goalCombo_->setCurrentIndex(goalIdx >= 0 ? goalIdx : 0);

    weeklyCombo_ = new QComboBox;
    weeklyCombo_->addItem(QStringLiteral("每周减 0.5 kg"), 0.5);
    weeklyCombo_->addItem(QStringLiteral("每周减 1.0 kg"), 1.0);
    weeklyCombo_->addItem(QStringLiteral("每周减 1.5 kg"), 1.5);
    int wIdx = weeklyCombo_->findData(base_.weeklyLossTarget);
    if (wIdx < 0)
        wIdx = 0;
    weeklyCombo_->setCurrentIndex(wIdx);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("姓名"), nameEdit_);
    form->addRow(QStringLiteral("性别"), genderCombo_);
    form->addRow(QStringLiteral("年龄"), ageSpin_);
    form->addRow(QStringLiteral("身高"), heightSpin_);
    form->addRow(QStringLiteral("体重"), weightSpin_);
    form->addRow(QStringLiteral("目标体重"), targetWeightSpin_);
    form->addRow(QStringLiteral("日常活动量"), activityCombo_);
    form->addRow(QStringLiteral("目标类型"), goalCombo_);
    form->addRow(QStringLiteral("每周目标"), weeklyCombo_);

    auto *hint = new QLabel(QStringLiteral(
        "提示：减重按约 7700 kcal ≈ 1 kg 体重折算每日热量缺口。"));
    hint->setStyleSheet("color:gray;");

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(hint);
    layout->addWidget(buttons);
}

UserProfile UserEditDialog::resultUser() const
{
    UserProfile u = base_;
    u.name = nameEdit_->text().trimmed();
    u.gender = genderCombo_->currentData().toString().at(0).toLatin1();
    u.age = ageSpin_->value();
    u.height = heightSpin_->value();
    u.weight = weightSpin_->value();
    u.targetWeight = targetWeightSpin_->value();
    u.activityLevel = activityCombo_->currentData().toInt();
    u.goalType = goalCombo_->currentData().toString();
    u.weeklyLossTarget = weeklyCombo_->currentData().toDouble();
    if (u.goalType != "lose")
        u.weeklyLossTarget = 0.0;
    return u;
}
