#pragma once

#include "UserProfile.h"

#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;

/**
 * @brief 用户档案创建/编辑对话框
 */
class UserEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserEditDialog(const UserProfile *existing, QWidget *parent = nullptr);

    /** 获取对话框中的用户数据（id 保留原值，新建时由调用方赋值） */
    UserProfile resultUser() const;

private:
    QLineEdit      *nameEdit_;
    QComboBox      *genderCombo_;
    QSpinBox       *ageSpin_;
    QDoubleSpinBox *heightSpin_;
    QDoubleSpinBox *weightSpin_;
    QDoubleSpinBox *targetWeightSpin_;
    QComboBox      *activityCombo_;
    QComboBox      *goalCombo_;
    QComboBox      *weeklyCombo_;

    UserProfile base_;
};
