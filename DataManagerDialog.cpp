#include "DataManagerDialog.h"

#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>
#include <QSignalBlocker>

// ---------------------------------------------------------------------------
// 内部辅助：编辑单个运动 / 食谱的小对话框
// ---------------------------------------------------------------------------

static Exercise promptExercise(const Exercise *existing, const QString &defaultId, QWidget *parent)
{
    Exercise e = existing ? *existing : Exercise();
    if (e.id.isEmpty())
        e.id = defaultId;

    QDialog dlg(parent);
    dlg.setWindowTitle(existing ? QStringLiteral("编辑运动") : QStringLiteral("新增运动"));

    auto *name = new QLineEdit(e.name);
    auto *met = new QDoubleSpinBox;
    met->setRange(0.1, 30.0);
    met->setDecimals(1);
    met->setValue(e.metValue > 0 ? e.metValue : 4.0);
    auto *cat = new QComboBox;
    cat->addItems({QStringLiteral("有氧"), QStringLiteral("力量"), QStringLiteral("柔韧")});
    cat->setCurrentText(e.category.isEmpty() ? QStringLiteral("有氧") : e.category);
    auto *desc = new QLineEdit(e.description);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("名称"), name);
    form->addRow(QStringLiteral("MET 值"), met);
    form->addRow(QStringLiteral("类别"), cat);
    form->addRow(QStringLiteral("描述"), desc);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addLayout(form);
    lay->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) {
        e.id.clear();
        return e;
    }
    e.name = name->text().trimmed();
    e.metValue = met->value();
    e.category = cat->currentText();
    e.description = desc->text().trimmed();
    return e;
}

static Recipe promptRecipe(const Recipe *existing, const QString &defaultId, QWidget *parent)
{
    Recipe r = existing ? *existing : Recipe();
    if (r.id.isEmpty())
        r.id = defaultId;

    QDialog dlg(parent);
    dlg.setWindowTitle(existing ? QStringLiteral("编辑食谱") : QStringLiteral("新增食谱"));

    auto *name = new QLineEdit(r.name);
    auto *ing = new QLineEdit(r.ingredients);
    auto *kcal = new QDoubleSpinBox;
    kcal->setRange(1, 5000);
    kcal->setDecimals(0);
    kcal->setSuffix(QStringLiteral(" kcal"));
    kcal->setValue(r.totalCalories > 0 ? r.totalCalories : 300);
    auto *meal = new QComboBox;
    meal->addItem(QStringLiteral("早餐"), QStringLiteral("breakfast"));
    meal->addItem(QStringLiteral("午餐"), QStringLiteral("lunch"));
    meal->addItem(QStringLiteral("晚餐"), QStringLiteral("dinner"));
    meal->addItem(QStringLiteral("加餐"), QStringLiteral("snack"));
    int mi = meal->findData(r.mealType);
    meal->setCurrentIndex(mi >= 0 ? mi : 1);
    auto *tags = new QLineEdit(r.nutritionTags.join(QStringLiteral(",")));

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("名称"), name);
    form->addRow(QStringLiteral("食材"), ing);
    form->addRow(QStringLiteral("总热量"), kcal);
    form->addRow(QStringLiteral("餐次"), meal);
    form->addRow(QStringLiteral("营养标签(逗号分隔)"), tags);

    auto *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *lay = new QVBoxLayout(&dlg);
    lay->addLayout(form);
    lay->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) {
        r.id.clear();
        return r;
    }
    r.name = name->text().trimmed();
    r.ingredients = ing->text().trimmed();
    r.totalCalories = kcal->value();
    r.mealType = meal->currentData().toString();
    r.nutritionTags.clear();
    QString tagStr = tags->text().trimmed();
    if (!tagStr.isEmpty())
        r.nutritionTags = tagStr.split(',', Qt::SkipEmptyParts);
    return r;
}

// ---------------------------------------------------------------------------
// 构造
// ---------------------------------------------------------------------------

DataManagerDialog::DataManagerDialog(DataStore *store, QWidget *parent)
    : QDialog(parent), store_(store)
{
    setWindowTitle(QStringLiteral("运动库与食谱库管理"));
    resize(820, 600);

    auto *tabs = new QTabWidget;

    // ===================== 运动库 Tab =====================
    auto *exTab = new QWidget;
    exerciseTable_ = new QTableWidget;
    exerciseTable_->setColumnCount(4);
    exerciseTable_->setHorizontalHeaderLabels(
        {QStringLiteral("名称"), QStringLiteral("MET"), QStringLiteral("类别"), QStringLiteral("描述")});
    exerciseTable_->horizontalHeader()->setStretchLastSection(true);
    exerciseTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    exerciseTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    exerciseTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    exerciseTable_->setAlternatingRowColors(true);
    exerciseTable_->setShowGrid(false);

    auto *exSearch = new QLineEdit;
    exSearch->setPlaceholderText(QStringLiteral("搜索运动名称..."));
    auto *exFilter = new QComboBox;
    exFilter->addItem(QStringLiteral("全部类别"), QString());
    exFilter->addItem(QStringLiteral("有氧"), QStringLiteral("有氧"));
    exFilter->addItem(QStringLiteral("力量"), QStringLiteral("力量"));
    exFilter->addItem(QStringLiteral("柔韧"), QStringLiteral("柔韧"));

    auto *exAdd = new QPushButton(QStringLiteral("新增"));
    auto *exEdit = new QPushButton(QStringLiteral("编辑"));
    auto *exDel = new QPushButton(QStringLiteral("删除"));
    auto *exImport = new QPushButton(QStringLiteral("导入CSV"));
    auto *exExport = new QPushButton(QStringLiteral("导出CSV"));

    auto *exBtnRow = new QHBoxLayout;
    exBtnRow->addWidget(exSearch, 1);
    exBtnRow->addWidget(exFilter);
    exBtnRow->addWidget(exAdd);
    exBtnRow->addWidget(exEdit);
    exBtnRow->addWidget(exDel);
    exBtnRow->addWidget(exImport);
    exBtnRow->addWidget(exExport);

    exCountLabel_ = new QLabel;

    auto *exLay = new QVBoxLayout(exTab);
    exLay->addLayout(exBtnRow);
    exLay->addWidget(exerciseTable_);
    exLay->addWidget(exCountLabel_);

    // ===================== 食谱库 Tab =====================
    auto *recTab = new QWidget;
    recipeTable_ = new QTableWidget;
    recipeTable_->setColumnCount(5);
    recipeTable_->setHorizontalHeaderLabels(
        {QStringLiteral("名称"), QStringLiteral("食材"), QStringLiteral("热量(kcal)"),
         QStringLiteral("餐次"), QStringLiteral("营养标签")});
    recipeTable_->horizontalHeader()->setStretchLastSection(true);
    recipeTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    recipeTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    recipeTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    recipeTable_->setAlternatingRowColors(true);
    recipeTable_->setShowGrid(false);

    auto *recSearch = new QLineEdit;
    recSearch->setPlaceholderText(QStringLiteral("搜索菜品名称..."));
    auto *recFilter = new QComboBox;
    recFilter->addItem(QStringLiteral("全部餐次"), QString());
    recFilter->addItem(QStringLiteral("早餐"), QStringLiteral("breakfast"));
    recFilter->addItem(QStringLiteral("午餐"), QStringLiteral("lunch"));
    recFilter->addItem(QStringLiteral("晚餐"), QStringLiteral("dinner"));
    recFilter->addItem(QStringLiteral("加餐"), QStringLiteral("snack"));

    auto *recAdd = new QPushButton(QStringLiteral("新增"));
    auto *recEdit = new QPushButton(QStringLiteral("编辑"));
    auto *recDel = new QPushButton(QStringLiteral("删除"));
    auto *recImport = new QPushButton(QStringLiteral("导入CSV"));
    auto *recExport = new QPushButton(QStringLiteral("导出CSV"));

    auto *recBtnRow = new QHBoxLayout;
    recBtnRow->addWidget(recSearch, 1);
    recBtnRow->addWidget(recFilter);
    recBtnRow->addWidget(recAdd);
    recBtnRow->addWidget(recEdit);
    recBtnRow->addWidget(recDel);
    recBtnRow->addWidget(recImport);
    recBtnRow->addWidget(recExport);

    recCountLabel_ = new QLabel;

    auto *recLay = new QVBoxLayout(recTab);
    recLay->addLayout(recBtnRow);
    recLay->addWidget(recipeTable_);
    recLay->addWidget(recCountLabel_);

    tabs->addTab(exTab, QStringLiteral("运动库"));
    tabs->addTab(recTab, QStringLiteral("食谱库"));

    auto *closeBtn = new QPushButton(QStringLiteral("关闭"));
    auto *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(tabs);
    mainLay->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // 运动库信号
    connect(exAdd, &QPushButton::clicked, this, &DataManagerDialog::onAddExercise);
    connect(exEdit, &QPushButton::clicked, this, &DataManagerDialog::onEditExercise);
    connect(exDel, &QPushButton::clicked, this, &DataManagerDialog::onDeleteExercise);
    connect(exImport, &QPushButton::clicked, this, &DataManagerDialog::onImportExercises);
    connect(exExport, &QPushButton::clicked, this, &DataManagerDialog::onExportExercises);
    connect(exSearch, &QLineEdit::textChanged, this, &DataManagerDialog::onExerciseSearch);
    connect(exFilter, &QComboBox::currentIndexChanged, this, &DataManagerDialog::onExerciseFilter);
    connect(exerciseTable_, &QTableWidget::itemChanged, this, &DataManagerDialog::onExerciseItemChanged);

    // 食谱库信号
    connect(recAdd, &QPushButton::clicked, this, &DataManagerDialog::onAddRecipe);
    connect(recEdit, &QPushButton::clicked, this, &DataManagerDialog::onEditRecipe);
    connect(recDel, &QPushButton::clicked, this, &DataManagerDialog::onDeleteRecipe);
    connect(recImport, &QPushButton::clicked, this, &DataManagerDialog::onImportRecipes);
    connect(recExport, &QPushButton::clicked, this, &DataManagerDialog::onExportRecipes);
    connect(recSearch, &QLineEdit::textChanged, this, &DataManagerDialog::onRecipeSearch);
    connect(recFilter, &QComboBox::currentIndexChanged, this, &DataManagerDialog::onRecipeFilter);
    connect(recipeTable_, &QTableWidget::itemChanged, this, &DataManagerDialog::onRecipeItemChanged);

    refreshExerciseTable();
    refreshRecipeTable();
    refreshCounts();
}

// ---------------------------------------------------------------------------
// 运动库
// ---------------------------------------------------------------------------

int DataManagerDialog::selectedExerciseRow() const
{
    auto items = exerciseTable_->selectedItems();
    if (items.isEmpty())
        return -1;
    return items.first()->row();
}

void DataManagerDialog::refreshExerciseTable()
{
    QList<Exercise> filtered;
    for (const Exercise &e : store_->exercises) {
        if (!exerciseCategory_.isEmpty() && e.category != exerciseCategory_)
            continue;
        if (!exerciseSearch_.isEmpty() && !e.name.contains(exerciseSearch_, Qt::CaseInsensitive))
            continue;
        filtered.append(e);
    }

    QSignalBlocker blocker(exerciseTable_);
    exerciseTable_->setRowCount(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        const Exercise &e = filtered.at(i);
        auto *nameItem = new QTableWidgetItem(e.name);
        nameItem->setData(Qt::UserRole, e.id);
        exerciseTable_->setItem(i, 0, nameItem);
        exerciseTable_->setItem(i, 1, new QTableWidgetItem(QString::number(e.metValue, 'f', 1)));
        exerciseTable_->setItem(i, 2, new QTableWidgetItem(e.category));
        exerciseTable_->setItem(i, 3, new QTableWidgetItem(e.description));
    }
}

void DataManagerDialog::onExerciseItemChanged(QTableWidgetItem *item)
{
    if (!item)
        return;
    int row = item->row();
    int col = item->column();
    QTableWidgetItem *idItem = exerciseTable_->item(row, 0);
    if (!idItem)
        return;
    Exercise *target = store_->findExercise(idItem->data(Qt::UserRole).toString());
    if (!target)
        return;
    switch (col) {
    case 0: target->name = item->text().trimmed(); break;
    case 1: target->metValue = item->text().toDouble(); break;
    case 2: target->category = item->text().trimmed(); break;
    case 3: target->description = item->text().trimmed(); break;
    default: break;
    }
    refreshCounts();
}

void DataManagerDialog::onAddExercise()
{
    Exercise e = promptExercise(nullptr, store_->nextExerciseId(), this);
    if (e.id.isEmpty())
        return;
    if (e.name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("运动名称不能为空"));
        return;
    }
    store_->exercises.append(e);
    refreshExerciseTable();
    refreshCounts();
}

void DataManagerDialog::onEditExercise()
{
    int row = selectedExerciseRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要编辑的运动"));
        return;
    }
    QString id = exerciseTable_->item(row, 0)->data(Qt::UserRole).toString();
    Exercise *target = store_->findExercise(id);
    if (!target)
        return;
    Exercise e = promptExercise(target, id, this);
    if (e.id.isEmpty())
        return;
    *target = e;
    refreshExerciseTable();
}

void DataManagerDialog::onDeleteExercise()
{
    int row = selectedExerciseRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要删除的运动"));
        return;
    }
    QString id = exerciseTable_->item(row, 0)->data(Qt::UserRole).toString();
    if (QMessageBox::question(this, QStringLiteral("确认"),
            QStringLiteral("确定删除该运动吗？"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    for (int i = 0; i < store_->exercises.size(); ++i) {
        if (store_->exercises.at(i).id == id) {
            store_->exercises.removeAt(i);
            break;
        }
    }
    refreshExerciseTable();
    refreshCounts();
}

void DataManagerDialog::onImportExercises()
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("导入运动 CSV"),
        QString(), QStringLiteral("CSV 文件 (*.csv)"));
    if (path.isEmpty())
        return;
    QString msg;
    store_->importExercisesCsv(path, &msg);
    QMessageBox::information(this, QStringLiteral("导入结果"), msg);
    refreshExerciseTable();
    refreshCounts();
}

void DataManagerDialog::onExportExercises()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("导出运动 CSV"),
        QStringLiteral("exercises.csv"), QStringLiteral("CSV 文件 (*.csv)"));
    if (path.isEmpty())
        return;
    QString msg;
    if (store_->exportExercisesCsv(path, &msg))
        QMessageBox::information(this, QStringLiteral("导出成功"), QStringLiteral("已导出到：%1").arg(path));
    else
        QMessageBox::warning(this, QStringLiteral("导出失败"), msg);
}

void DataManagerDialog::onExerciseSearch(const QString &kw)
{
    exerciseSearch_ = kw;
    refreshExerciseTable();
}

void DataManagerDialog::onExerciseFilter(int index)
{
    Q_UNUSED(index);
    auto *combo = qobject_cast<QComboBox *>(sender());
    exerciseCategory_ = combo ? combo->currentData().toString() : QString();
    refreshExerciseTable();
}

// ---------------------------------------------------------------------------
// 食谱库
// ---------------------------------------------------------------------------

int DataManagerDialog::selectedRecipeRow() const
{
    auto items = recipeTable_->selectedItems();
    if (items.isEmpty())
        return -1;
    return items.first()->row();
}

void DataManagerDialog::refreshRecipeTable()
{
    QList<Recipe> filtered;
    for (const Recipe &r : store_->recipes) {
        if (!recipeMealType_.isEmpty() && r.mealType != recipeMealType_)
            continue;
        if (!recipeSearch_.isEmpty() && !r.name.contains(recipeSearch_, Qt::CaseInsensitive))
            continue;
        filtered.append(r);
    }

    QSignalBlocker blocker(recipeTable_);
    recipeTable_->setRowCount(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        const Recipe &r = filtered.at(i);
        auto *nameItem = new QTableWidgetItem(r.name);
        nameItem->setData(Qt::UserRole, r.id);
        recipeTable_->setItem(i, 0, nameItem);
        recipeTable_->setItem(i, 1, new QTableWidgetItem(r.ingredients));
        recipeTable_->setItem(i, 2, new QTableWidgetItem(QString::number(r.totalCalories, 'f', 0)));

        QString mealName;
        if (r.mealType == "breakfast")      mealName = QStringLiteral("早餐");
        else if (r.mealType == "lunch")     mealName = QStringLiteral("午餐");
        else if (r.mealType == "dinner")    mealName = QStringLiteral("晚餐");
        else if (r.mealType == "snack")     mealName = QStringLiteral("加餐");
        recipeTable_->setItem(i, 3, new QTableWidgetItem(mealName));
        recipeTable_->setItem(i, 4, new QTableWidgetItem(r.nutritionTags.join(QStringLiteral("、"))));
    }
}

void DataManagerDialog::onRecipeItemChanged(QTableWidgetItem *item)
{
    if (!item)
        return;
    int row = item->row();
    int col = item->column();
    QTableWidgetItem *idItem = recipeTable_->item(row, 0);
    if (!idItem)
        return;
    Recipe *target = store_->findRecipe(idItem->data(Qt::UserRole).toString());
    if (!target)
        return;
    switch (col) {
    case 0: target->name = item->text().trimmed(); break;
    case 1: target->ingredients = item->text().trimmed(); break;
    case 2: target->totalCalories = item->text().toDouble(); break;
    case 3: {
        QString t = item->text().trimmed();
        if (t == QStringLiteral("早餐"))      target->mealType = "breakfast";
        else if (t == QStringLiteral("午餐")) target->mealType = "lunch";
        else if (t == QStringLiteral("晚餐")) target->mealType = "dinner";
        else if (t == QStringLiteral("加餐")) target->mealType = "snack";
        break;
    }
    case 4:
        target->nutritionTags = item->text().split(QStringLiteral("、"), Qt::SkipEmptyParts);
        break;
    default: break;
    }
    refreshCounts();
}

void DataManagerDialog::onAddRecipe()
{
    Recipe r = promptRecipe(nullptr, store_->nextRecipeId(), this);
    if (r.id.isEmpty())
        return;
    if (r.name.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("菜品名称不能为空"));
        return;
    }
    store_->recipes.append(r);
    refreshRecipeTable();
    refreshCounts();
}

void DataManagerDialog::onEditRecipe()
{
    int row = selectedRecipeRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要编辑的菜品"));
        return;
    }
    QString id = recipeTable_->item(row, 0)->data(Qt::UserRole).toString();
    Recipe *target = store_->findRecipe(id);
    if (!target)
        return;
    Recipe r = promptRecipe(target, id, this);
    if (r.id.isEmpty())
        return;
    *target = r;
    refreshRecipeTable();
}

void DataManagerDialog::onDeleteRecipe()
{
    int row = selectedRecipeRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选中要删除的菜品"));
        return;
    }
    QString id = recipeTable_->item(row, 0)->data(Qt::UserRole).toString();
    if (QMessageBox::question(this, QStringLiteral("确认"),
            QStringLiteral("确定删除该菜品吗？"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    for (int i = 0; i < store_->recipes.size(); ++i) {
        if (store_->recipes.at(i).id == id) {
            store_->recipes.removeAt(i);
            break;
        }
    }
    refreshRecipeTable();
    refreshCounts();
}

void DataManagerDialog::onImportRecipes()
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("导入食谱 CSV"),
        QString(), QStringLiteral("CSV 文件 (*.csv)"));
    if (path.isEmpty())
        return;
    QString msg;
    store_->importRecipesCsv(path, &msg);
    QMessageBox::information(this, QStringLiteral("导入结果"), msg);
    refreshRecipeTable();
    refreshCounts();
}

void DataManagerDialog::onExportRecipes()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("导出食谱 CSV"),
        QStringLiteral("recipes.csv"), QStringLiteral("CSV 文件 (*.csv)"));
    if (path.isEmpty())
        return;
    QString msg;
    if (store_->exportRecipesCsv(path, &msg))
        QMessageBox::information(this, QStringLiteral("导出成功"), QStringLiteral("已导出到：%1").arg(path));
    else
        QMessageBox::warning(this, QStringLiteral("导出失败"), msg);
}

void DataManagerDialog::onRecipeSearch(const QString &kw)
{
    recipeSearch_ = kw;
    refreshRecipeTable();
}

void DataManagerDialog::onRecipeFilter(int index)
{
    Q_UNUSED(index);
    auto *combo = qobject_cast<QComboBox *>(sender());
    recipeMealType_ = combo ? combo->currentData().toString() : QString();
    refreshRecipeTable();
}

void DataManagerDialog::refreshCounts()
{
    exCountLabel_->setText(QStringLiteral("共 %1 项运动（有氧 %2 / 力量 %3 / 柔韧 %4）")
        .arg(store_->exercises.size())
        .arg(store_->countByCategory(QStringLiteral("有氧")))
        .arg(store_->countByCategory(QStringLiteral("力量")))
        .arg(store_->countByCategory(QStringLiteral("柔韧"))));

    recCountLabel_->setText(QStringLiteral("共 %1 道菜品（早 %2 / 午 %3 / 晚 %4 / 加餐 %5）")
        .arg(store_->recipes.size())
        .arg(store_->countByMealType(QStringLiteral("breakfast")))
        .arg(store_->countByMealType(QStringLiteral("lunch")))
        .arg(store_->countByMealType(QStringLiteral("dinner")))
        .arg(store_->countByMealType(QStringLiteral("snack"))));
}
