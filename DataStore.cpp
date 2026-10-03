#include "DataStore.h"
#include "InitialData.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

// 兼容 Qt5 / Qt6 的流编码设置
static void setStreamUtf8(QTextStream &ts)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ts.setEncoding(QStringConverter::Utf8);
#else
    ts.setCodec("UTF-8");
#endif
}

// ---------------------------------------------------------------------------
// JSON 持久化
// ---------------------------------------------------------------------------

bool DataStore::saveToFile(const QString &path, QString *errMsg) const
{
    QJsonObject root;
    root["currentUserId"] = currentUserId;

    QJsonArray userArr, exArr, recArr;
    for (const UserProfile &u : users)
        userArr.append(u.toJson());
    for (const Exercise &e : exercises)
        exArr.append(e.toJson());
    for (const Recipe &r : recipes)
        recArr.append(r.toJson());

    root["users"] = userArr;
    root["exercises"] = exArr;
    root["recipes"] = recArr;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入文件：%1").arg(path);
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

bool DataStore::loadFromFile(const QString &path, QString *errMsg)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法打开文件：%1").arg(path);
        return false;
    }
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    f.close();
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errMsg) *errMsg = QStringLiteral("JSON 解析失败：%1").arg(pe.errorString());
        return false;
    }

    QJsonObject root = doc.object();
    users.clear();
    exercises.clear();
    recipes.clear();

    for (const QJsonValue &v : root.value("users").toArray())
        users.append(UserProfile::fromJson(v.toObject()));
    for (const QJsonValue &v : root.value("exercises").toArray())
        exercises.append(Exercise::fromJson(v.toObject()));
    for (const QJsonValue &v : root.value("recipes").toArray())
        recipes.append(Recipe::fromJson(v.toObject()));

    currentUserId = root.value("currentUserId").toString();
    if (currentUserId.isEmpty() && !users.isEmpty())
        currentUserId = users.first().id;
    return true;
}

// ---------------------------------------------------------------------------
// 查找
// ---------------------------------------------------------------------------

UserProfile *DataStore::findUser(const QString &id)
{
    for (UserProfile &u : users)
        if (u.id == id)
            return &u;
    return nullptr;
}

const UserProfile *DataStore::findUser(const QString &id) const
{
    for (const UserProfile &u : users)
        if (u.id == id)
            return &u;
    return nullptr;
}

Exercise *DataStore::findExercise(const QString &id)
{
    for (Exercise &e : exercises)
        if (e.id == id)
            return &e;
    return nullptr;
}

const Exercise *DataStore::findExercise(const QString &id) const
{
    for (const Exercise &e : exercises)
        if (e.id == id)
            return &e;
    return nullptr;
}

Recipe *DataStore::findRecipe(const QString &id)
{
    for (Recipe &r : recipes)
        if (r.id == id)
            return &r;
    return nullptr;
}

const Recipe *DataStore::findRecipe(const QString &id) const
{
    for (const Recipe &r : recipes)
        if (r.id == id)
            return &r;
    return nullptr;
}

// ---------------------------------------------------------------------------
// ID 生成
// ---------------------------------------------------------------------------

QString DataStore::nextUserId() const
{
    int maxN = 0;
    for (const UserProfile &u : users) {
        QString n = u.id;
        if (n.startsWith("u"))
            n.remove(0, 1);
        bool ok = false;
        int v = n.toInt(&ok);
        if (ok && v > maxN)
            maxN = v;
    }
    return QStringLiteral("u%1").arg(maxN + 1);
}

QString DataStore::nextExerciseId() const
{
    int maxN = 0;
    for (const Exercise &e : exercises) {
        QString n = e.id;
        if (n.startsWith("e"))
            n.remove(0, 1);
        bool ok = false;
        int v = n.toInt(&ok);
        if (ok && v > maxN)
            maxN = v;
    }
    return QStringLiteral("e%1").arg(maxN + 1);
}

QString DataStore::nextRecipeId() const
{
    int maxN = 0;
    for (const Recipe &r : recipes) {
        QString n = r.id;
        if (n.startsWith("r"))
            n.remove(0, 1);
        bool ok = false;
        int v = n.toInt(&ok);
        if (ok && v > maxN)
            maxN = v;
    }
    return QStringLiteral("r%1").arg(maxN + 1);
}

// ---------------------------------------------------------------------------
// CSV 导入/导出（简单 CSV：逗号分隔，字段内不含逗号换行）
// ---------------------------------------------------------------------------

bool DataStore::importExercisesCsv(const QString &path, QString *errMsg)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法打开 CSV：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);
    bool firstLine = true;
    int added = 0;
    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (firstLine) { // 跳过表头
            firstLine = false;
            if (line.contains("name") || line.contains("名称"))
                continue;
        }
        QStringList cols = line.split(',');
        if (cols.size() < 2)
            continue;
        Exercise e;
        e.id = nextExerciseId();
        e.name = cols.value(0).trimmed();
        e.metValue = cols.value(1).trimmed().toDouble();
        e.category = cols.value(2).trimmed().isEmpty() ? QStringLiteral("有氧") : cols.value(2).trimmed();
        e.description = cols.value(3).trimmed();
        if (e.name.isEmpty() || e.metValue <= 0)
            continue;
        exercises.append(e);
        ++added;
    }
    f.close();
    if (errMsg) *errMsg = QStringLiteral("成功导入 %1 项运动").arg(added);
    return added > 0;
}

bool DataStore::importRecipesCsv(const QString &path, QString *errMsg)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法打开 CSV：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);
    bool firstLine = true;
    int added = 0;
    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (firstLine) {
            firstLine = false;
            if (line.contains("name") || line.contains("名称"))
                continue;
        }
        QStringList cols = line.split(',');
        if (cols.size() < 3)
            continue;
        Recipe r;
        r.id = nextRecipeId();
        r.name = cols.value(0).trimmed();
        r.ingredients = cols.value(1).trimmed();
        r.totalCalories = cols.value(2).trimmed().toDouble();
        r.mealType = cols.value(3).trimmed().isEmpty() ? QStringLiteral("lunch") : cols.value(3).trimmed();
        if (r.name.isEmpty() || r.totalCalories <= 0)
            continue;
        recipes.append(r);
        ++added;
    }
    f.close();
    if (errMsg) *errMsg = QStringLiteral("成功导入 %1 道菜品").arg(added);
    return added > 0;
}

bool DataStore::exportExercisesCsv(const QString &path, QString *errMsg) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入 CSV：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);
    ts << "名称,MET值,类别,描述\n";
    for (const Exercise &e : exercises) {
        ts << e.name << ',' << e.metValue << ',' << e.category << ',' << e.description << '\n';
    }
    f.close();
    return true;
}

bool DataStore::exportRecipesCsv(const QString &path, QString *errMsg) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errMsg) *errMsg = QStringLiteral("无法写入 CSV：%1").arg(path);
        return false;
    }
    QTextStream ts(&f);
    setStreamUtf8(ts);
    ts << "名称,食材,总热量(kcal),餐次,营养标签\n";
    for (const Recipe &r : recipes) {
        ts << r.name << ',' << r.ingredients << ',' << r.totalCalories << ','
           << r.mealType << ',' << r.nutritionTags.join(';') << '\n';
    }
    f.close();
    return true;
}

// ---------------------------------------------------------------------------
// 统计辅助
// ---------------------------------------------------------------------------

int DataStore::countByMealType(const QString &mealType) const
{
    int c = 0;
    for (const Recipe &r : recipes)
        if (r.mealType == mealType)
            ++c;
    return c;
}

int DataStore::countByCategory(const QString &category) const
{
    int c = 0;
    for (const Exercise &e : exercises)
        if (e.category == category)
            ++c;
    return c;
}

void DataStore::fillInitialData()
{
    *this = buildInitialData();
}
