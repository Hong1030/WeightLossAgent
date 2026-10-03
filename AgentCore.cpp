#include "AgentCore.h"

#include <algorithm>
#include <chrono>
#include <cmath>

AgentCore::AgentCore()
{
    // 用硬件熵源 + 时间戳混合播种
    std::random_device rd;
    auto seed = static_cast<unsigned long>(rd());
    seed ^= static_cast<unsigned long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    rng_.seed(seed);
}

int AgentCore::randInt(int lo, int hi) const
{
    if (lo >= hi)
        return lo;
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng_);
}

double AgentCore::randDouble(double lo, double hi) const
{
    std::uniform_real_distribution<double> dist(lo, hi);
    return dist(rng_);
}

double AgentCore::MealPlan::totalCalories() const
{
    double t = 0;
    for (const Recipe &r : breakfast) t += r.totalCalories;
    for (const Recipe &r : lunch)     t += r.totalCalories;
    for (const Recipe &r : dinner)    t += r.totalCalories;
    for (const Recipe &r : snacks)    t += r.totalCalories;
    return t;
}

int AgentCore::MealPlan::itemCount() const
{
    return breakfast.size() + lunch.size() + dinner.size() + snacks.size();
}

// ---------------------------------------------------------------------------
// 能量需求计算
// ---------------------------------------------------------------------------

AgentCore::CalorieNeed AgentCore::calculateCalorieNeeds(const UserProfile &user) const
{
    CalorieNeed n;
    n.bmr = user.getBMR();
    n.tdee = user.getTDEE();
    n.deficit = user.dailyCalorieDeficit();
    n.recommendedIntake = n.tdee - n.deficit;

    // 运动消耗目标 = 日缺口 - 饮食控制贡献
    // 采用 饮食:运动 = 7:3 的策略，即缺口中的 30% 由运动承担。
    n.exerciseTarget = std::max(0.0, n.deficit * 0.30);
    return n;
}

// ---------------------------------------------------------------------------
// 运动处方生成
// ---------------------------------------------------------------------------

QList<AgentCore::ExercisePlanItem> AgentCore::buildOneCombination(
    const QList<Exercise> &pool, double weightKg, double target) const
{
    QList<ExercisePlanItem> result;
    double total = 0.0;
    const int maxItems = 4;

    QList<Exercise> shuffled = pool;
    std::shuffle(shuffled.begin(), shuffled.end(), rng_);

    for (const Exercise &ex : shuffled) {
        if (result.size() >= maxItems || total >= target)
            break;

        double perMin = ex.caloriesPerMinute(weightKg);
        if (perMin <= 0.0)
            continue;

        int dur = randInt(10, 45);              // 随机时长 10~45 分钟
        double burn = perMin * dur;

        // 若加上后超过上限（target*1.1），缩短时长，使总消耗贴近目标
        if (total + burn > target * 1.10) {
            double allowHigh = target * 1.10 - total;
            int maxDur = static_cast<int>(allowHigh / perMin);
            if (maxDur >= 5) {
                dur = randInt(5, maxDur);
            } else {
                // 剩余空间不足 5 分钟，按最短补足时长处理
                dur = static_cast<int>(std::ceil((target - total) / perMin));
                if (dur < 3) dur = 3;
            }
            burn = perMin * dur;
        }

        if (dur <= 0)
            continue;

        ExercisePlanItem item;
        item.exerciseId = ex.id;
        item.exerciseName = ex.name;
        item.metValue = ex.metValue;
        item.durationMinutes = dur;
        item.caloriesBurned = burn;
        result.append(item);
        total += burn;
    }
    return result;
}

QList<AgentCore::ExercisePlanItem> AgentCore::generateExercisePrescription(
    const UserProfile &user, double targetCalories,
    const QList<Exercise> &exerciseDB) const
{
    QList<ExercisePlanItem> best;
    if (exerciseDB.isEmpty() || targetCalories <= 0.0)
        return best;

    // 构建候选池：排除"不喜欢"，"喜欢"的加权重（重复加入提高概率）
    QList<Exercise> pool;
    for (const Exercise &e : exerciseDB) {
        if (user.dislikedExerciseIds.contains(e.id))
            continue;
        pool.append(e);
        if (user.likedExerciseIds.contains(e.id))
            pool.append(e);
    }
    if (pool.isEmpty())
        pool = exerciseDB;   // 全部被排除则退回全部

    double bestScore = -1e18;
    const int attempts = 300;

    for (int i = 0; i < attempts; ++i) {
        QList<ExercisePlanItem> cand = buildOneCombination(pool, user.weight, targetCalories);
        if (cand.isEmpty())
            continue;

        double total = 0;
        for (const ExercisePlanItem &it : cand)
            total += it.caloriesBurned;

        double score;
        if (total >= targetCalories && total <= targetCalories * 1.10) {
            // 满足要求：越接近目标越高分
            score = 10000.0 - (total - targetCalories) / targetCalories * 100.0;
        } else {
            // 未落在区间：越接近目标越高分
            score = 5000.0 - std::abs(total - targetCalories) / targetCalories * 100.0;
        }

        if (score > bestScore) {
            bestScore = score;
            best = cand;
        }
        // 已非常接近目标即可提前结束
        double finalTotal = 0;
        for (const ExercisePlanItem &it : best)
            finalTotal += it.caloriesBurned;
        if (finalTotal >= targetCalories && finalTotal <= targetCalories * 1.10) {
            if ((finalTotal - targetCalories) / targetCalories < 0.02)
                break;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// 食谱推荐
// ---------------------------------------------------------------------------

QList<Recipe> AgentCore::fillOneMeal(const QList<Recipe> &pool, double target) const
{
    QList<Recipe> result;
    double total = 0.0;
    const int maxItems = 3;

    QList<Recipe> shuffled = pool;
    std::shuffle(shuffled.begin(), shuffled.end(), rng_);

    for (const Recipe &r : shuffled) {
        if (result.size() >= maxItems)
            break;
        if (total >= target * 1.03)
            break;
        // 已有菜品时避免明显超出上限
        if (!result.isEmpty() && total + r.totalCalories > target * 1.10)
            continue;
        result.append(r);
        total += r.totalCalories;
    }
    return result;
}

AgentCore::MealPlan AgentCore::generateMealPlan(
    double targetCalories, const QList<Recipe> &recipeDB,
    const QStringList &dislikeIds, const QStringList &likeIds) const
{
    MealPlan best;
    if (recipeDB.isEmpty() || targetCalories <= 0.0)
        return best;

    // 按餐次分组，并应用偏好权重
    QList<Recipe> breakfastPool, lunchPool, dinnerPool, snackPool;
    for (const Recipe &r : recipeDB) {
        if (dislikeIds.contains(r.id))
            continue;
        QList<Recipe> *dest = nullptr;
        if (r.mealType == "breakfast")      dest = &breakfastPool;
        else if (r.mealType == "lunch")     dest = &lunchPool;
        else if (r.mealType == "dinner")    dest = &dinnerPool;
        else if (r.mealType == "snack")     dest = &snackPool;
        if (!dest)
            continue;
        dest->append(r);
        if (likeIds.contains(r.id))
            dest->append(r);   // 喜欢的加权重
    }

    // 若某餐候选为空则退回该餐次全部（不排除）
    auto allByMeal = [&](const QString &mt) {
        QList<Recipe> r;
        for (const Recipe &x : recipeDB)
            if (x.mealType == mt)
                r.append(x);
        return r;
    };
    if (breakfastPool.isEmpty()) breakfastPool = allByMeal("breakfast");
    if (lunchPool.isEmpty())     lunchPool = allByMeal("lunch");
    if (dinnerPool.isEmpty())    dinnerPool = allByMeal("dinner");

    // 热量分配比例：早 30% / 午 40% / 晚 30%；若有加餐则整体略作调整
    double bRatio = 0.30, lRatio = 0.40, dRatio = 0.30;
    bool withSnack = !snackPool.isEmpty() && randInt(0, 2) == 0; // 1/3 概率加餐
    double sRatio = 0.0;
    if (withSnack) {
        bRatio = 0.28; lRatio = 0.34; dRatio = 0.28; sRatio = 0.10;
    }

    double bestDiff = 1e18;
    const int attempts = 150;

    for (int i = 0; i < attempts; ++i) {
        MealPlan mp;
        mp.breakfast = fillOneMeal(breakfastPool, targetCalories * bRatio);
        mp.lunch     = fillOneMeal(lunchPool,     targetCalories * lRatio);
        mp.dinner    = fillOneMeal(dinnerPool,    targetCalories * dRatio);
        if (withSnack)
            mp.snacks = fillOneMeal(snackPool, targetCalories * sRatio);

        double diff = std::abs(mp.totalCalories() - targetCalories);
        if (diff < bestDiff) {
            bestDiff = diff;
            best = mp;
        }
        if (bestDiff <= targetCalories * 0.02)
            break;   // 已足够接近（2% 以内）
    }
    return best;
}
