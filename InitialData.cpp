#include "InitialData.h"

#include <QStringList>

DataStore buildInitialData()
{
    DataStore ds;

    // ------------------------------------------------------------------
    // 1. 初始用户（10 个）
    // ------------------------------------------------------------------
    auto addUser = [&](const QString &id, const QString &name, char g, int age,
                       double h, double w, double tw, int act,
                       const QString &goal, double weekly) {
        UserProfile u;
        u.id = id;
        u.name = name;
        u.gender = g;
        u.age = age;
        u.height = h;
        u.weight = w;
        u.targetWeight = tw;
        u.activityLevel = act;
        u.goalType = goal;
        u.weeklyLossTarget = weekly;
        ds.users.append(u);
    };
    addUser("u1",  "张三", 'M', 30, 175, 82, 72, 3, "lose", 0.5);
    addUser("u2",  "李四", 'F', 25, 160, 60, 52, 2, "lose", 0.5);
    addUser("u3",  "王五", 'M', 35, 170, 90, 75, 2, "lose", 1.0);
    addUser("u4",  "赵六", 'F', 28, 165, 55, 50, 3, "maintain", 0.0);
    addUser("u5",  "钱七", 'M', 40, 180, 95, 80, 1, "lose", 1.5);
    addUser("u6",  "孙八", 'F', 22, 158, 48, 48, 4, "maintain", 0.0);
    addUser("u7",  "周九", 'M', 45, 172, 78, 70, 2, "lose", 0.5);
    addUser("u8",  "吴十", 'F', 33, 162, 68, 58, 2, "lose", 1.0);
    addUser("u9",  "郑十一", 'M', 27, 178, 70, 66, 3, "lose", 0.5);
    addUser("u10", "陈十二", 'F', 38, 168, 75, 60, 2, "lose", 1.0);

    // ------------------------------------------------------------------
    // 2. 初始运动库（30 项）
    // ------------------------------------------------------------------
    auto addExercise = [&](const QString &id, const QString &name, double met,
                           const QString &cat, const QString &desc) {
        Exercise e;
        e.id = id;
        e.name = name;
        e.metValue = met;
        e.category = cat;
        e.description = desc;
        ds.exercises.append(e);
    };
    // 有氧（21 项）
    addExercise("e1",  "慢走",            3.0,  "有氧", "低强度步行，适合热身与恢复");
    addExercise("e2",  "快走",            4.3,  "有氧", "中等强度快走，简单易坚持");
    addExercise("e3",  "慢跑",            7.0,  "有氧", "中等配速慢跑");
    addExercise("e4",  "跑步(8km/h)",     8.3,  "有氧", "8km/h 配速跑步");
    addExercise("e5",  "快跑(10km/h)",    11.0, "有氧", "高强度快跑，燃脂效率高");
    addExercise("e6",  "骑行(休闲)",      4.0,  "有氧", "休闲骑行");
    addExercise("e7",  "骑行(中等强度)",  6.8,  "有氧", "中等强度骑行");
    addExercise("e8",  "跳绳",            11.0, "有氧", "高效全身燃脂运动");
    addExercise("e9",  "游泳(自由泳)",    8.3,  "有氧", "自由泳，全身运动");
    addExercise("e10", "游泳(蛙泳)",      7.0,  "有氧", "蛙泳，对关节友好");
    addExercise("e11", "爬楼梯",          8.0,  "有氧", "爬楼梯，锻炼下肢");
    addExercise("e12", "登山",            7.5,  "有氧", "户外登山，心肺与力量兼顾");
    addExercise("e13", "椭圆机",          5.0,  "有氧", "低冲击有氧器械");
    addExercise("e14", "划船机",          7.0,  "有氧", "全身有氧+力量器械");
    addExercise("e15", "有氧健身操",      6.5,  "有氧", "跟随音乐的有氧操");
    addExercise("e16", "动感单车",        8.5,  "有氧", "室内高强度骑行课");
    addExercise("e17", "篮球",            6.5,  "有氧", "对抗性球类运动");
    addExercise("e18", "足球",            7.0,  "有氧", "跑动量大，消耗高");
    addExercise("e19", "羽毛球",          5.5,  "有氧", "灵活快速的球类运动");
    addExercise("e20", "乒乓球",          4.0,  "有氧", "技巧型球类运动");
    addExercise("e21", "网球",            7.3,  "有氧", "跑动与挥拍结合");
    // 力量（6 项）
    addExercise("e22", "深蹲",            5.0,  "力量", "下肢力量训练");
    addExercise("e23", "俯卧撑",          3.8,  "力量", "上肢与核心力量");
    addExercise("e24", "哑铃卧推",        3.5,  "力量", "胸大肌力量训练");
    addExercise("e25", "硬拉",            6.0,  "力量", "全身复合力量动作");
    addExercise("e26", "引体向上",        5.0,  "力量", "背部与上肢力量");
    addExercise("e27", "平板支撑",        3.8,  "力量", "核心稳定性训练");
    // 柔韧（3 项）
    addExercise("e28", "瑜伽",            2.5,  "柔韧", "柔韧与放松");
    addExercise("e29", "普拉提",          3.0,  "柔韧", "核心控制与体态训练");
    addExercise("e30", "全身拉伸",        2.3,  "柔韧", "训练后放松与拉伸");

    // ------------------------------------------------------------------
    // 3. 初始食谱库（40 道）
    // ------------------------------------------------------------------
    auto addRecipe = [&](const QString &id, const QString &name, const QString &ing,
                         double kcal, const QString &meal, const QStringList &tags) {
        Recipe r;
        r.id = id;
        r.name = name;
        r.ingredients = ing;
        r.totalCalories = kcal;
        r.mealType = meal;
        r.nutritionTags = tags;
        ds.recipes.append(r);
    };
    // 早餐（10 道）
    addRecipe("r1",  "水煮蛋+全麦吐司",   "鸡蛋2个、全麦吐司2片",        350, "breakfast", {"高蛋白"});
    addRecipe("r2",  "燕麦牛奶粥",         "燕麦50g、牛奶250ml",          300, "breakfast", {"高纤维"});
    addRecipe("r3",  "豆浆+素包子",        "豆浆300ml、素包子1个",        400, "breakfast", {"低脂"});
    addRecipe("r4",  "鸡蛋三明治",         "鸡蛋1个、吐司2片、生菜",      380, "breakfast", {"高蛋白"});
    addRecipe("r5",  "小米粥+水煮蛋",      "小米粥1碗、鸡蛋1个",          280, "breakfast", {"低脂"});
    addRecipe("r6",  "全麦吐司+花生酱",    "全麦吐司2片、花生酱15g",     320, "breakfast", {});
    addRecipe("r7",  "酸奶+水果燕麦杯",    "酸奶200g、燕麦30g、水果",     350, "breakfast", {"高纤维"});
    addRecipe("r8",  "蒸玉米+鸡蛋",        "玉米1根、鸡蛋1个",            250, "breakfast", {"高纤维"});
    addRecipe("r9",  "蔬菜鸡蛋饼",         "鸡蛋2个、面粉、蔬菜",         330, "breakfast", {"低脂"});
    addRecipe("r10", "牛奶+坚果麦片",      "牛奶250ml、麦片40g、坚果",    300, "breakfast", {"高纤维"});
    // 午餐（10 道）
    addRecipe("r11", "鸡胸肉蔬菜沙拉",     "鸡胸肉100g、时蔬、橄榄油",    400, "lunch", {"高蛋白", "低脂"});
    addRecipe("r12", "番茄炒蛋+米饭",      "番茄2个、鸡蛋2个、米饭1碗",   550, "lunch", {});
    addRecipe("r13", "清蒸鱼+糙米饭",      "鲈鱼150g、糙米饭1碗",         500, "lunch", {"高蛋白", "低脂"});
    addRecipe("r14", "牛肉炒西兰花+米饭",  "牛肉100g、西兰花、米饭",      560, "lunch", {"高蛋白"});
    addRecipe("r15", "宫保鸡丁+米饭",      "鸡胸肉、花生、米饭1碗",       600, "lunch", {"高蛋白"});
    addRecipe("r16", "香菇滑鸡+杂粮饭",    "鸡腿肉、香菇、杂粮饭",        520, "lunch", {"高蛋白"});
    addRecipe("r17", "虾仁蛋炒饭",         "虾仁、鸡蛋、米饭",            580, "lunch", {"高蛋白"});
    addRecipe("r18", "豆腐蔬菜煲",         "豆腐、时蔬、菌菇",            420, "lunch", {"低脂"});
    addRecipe("r19", "三文鱼+藜麦饭",      "三文鱼120g、藜麦饭",          520, "lunch", {"高蛋白", "低脂"});
    addRecipe("r20", "照烧鸡腿饭",         "鸡腿肉150g、米饭1碗",         650, "lunch", {"高蛋白"});
    // 晚餐（10 道）
    addRecipe("r21", "蔬菜沙拉+鸡胸肉",    "鸡胸肉100g、生菜、番茄",      350, "dinner", {"高蛋白", "低脂"});
    addRecipe("r22", "紫薯+清蒸虾",        "紫薯1个、虾150g",             380, "dinner", {"高蛋白", "低脂"});
    addRecipe("r23", "冬瓜瘦肉汤",         "冬瓜、瘦猪肉、姜",            320, "dinner", {"低脂"});
    addRecipe("r24", "凉拌黄瓜+酱牛肉",    "黄瓜、酱牛肉80g",             380, "dinner", {"高蛋白"});
    addRecipe("r25", "番茄豆腐汤+青菜",    "番茄、豆腐、青菜",            280, "dinner", {"低脂"});
    addRecipe("r26", "蒸蛋+清炒西兰花",    "鸡蛋2个、西兰花",             300, "dinner", {"低脂"});
    addRecipe("r27", "鱼汤+杂粮饭",        "鲫鱼汤、杂粮饭1碗",           420, "dinner", {"高蛋白"});
    addRecipe("r28", "烤鸡翅+时蔬",        "鸡翅2个、时蔬",               450, "dinner", {"高蛋白"});
    addRecipe("r29", "虾仁蒸豆腐",         "虾仁、嫩豆腐",                350, "dinner", {"高蛋白", "低脂"});
    addRecipe("r30", "蔬菜炒面",           "面条、时蔬、鸡蛋",            480, "dinner", {});
    // 加餐（10 道）
    addRecipe("r31", "苹果",               "苹果1个",                     80,  "snack", {"低脂"});
    addRecipe("r32", "香蕉",               "香蕉1根",                     105, "snack", {});
    addRecipe("r33", "无糖酸奶",           "无糖酸奶200g",                120, "snack", {"低脂"});
    addRecipe("r34", "混合坚果",           "混合坚果20g",                 160, "snack", {});
    addRecipe("r35", "全麦饼干",           "全麦饼干3片",                 140, "snack", {"高纤维"});
    addRecipe("r36", "蛋白棒",             "蛋白棒1根",                   180, "snack", {"高蛋白"});
    addRecipe("r37", "圣女果",             "圣女果150g",                  50,  "snack", {"低脂"});
    addRecipe("r38", "纯牛奶",             "纯牛奶250ml",                 120, "snack", {"高蛋白"});
    addRecipe("r39", "水煮蛋",             "鸡蛋1个",                     70,  "snack", {"高蛋白"});
    addRecipe("r40", "橙子",               "橙子1个",                     60,  "snack", {"低脂"});

    ds.currentUserId = "u1";
    return ds;
}
