#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <graphics.h>
#include <windows.h>
#include <easyx.h>
#include <math.h>
#include <stdarg.h>
#include<mmsystem.h> /*音乐的库*/
#pragma comment(lib,"winmm.lib") /*链接起来*/

//背景大小宏
#define WIN_width 800
#define WIN_height 800
#define R1 200
#define R2 250
#define R3 300
#define R4 350
#define R5 400
#define R6 450
#define R7 500
#define STA 175
#define STO 450
float delta_time = 0.0f;
static clock_t last_time;

//图片引用申明
IMAGE imgbg;//定义一个图片变量
IMAGE imghouse_one;
IMAGE imghouse_two;
IMAGE imgjail;
IMAGE imghospital;
IMAGE whitebg;//定义一个白色背景图片变量
IMAGE imgchance;
IMAGE character1;
IMAGE character2;
IMAGE character3;
IMAGE character4;
IMAGE c1ground;
IMAGE c2ground;
IMAGE c3ground;
IMAGE c4ground;

// consts
const int LAND = 1000;
const int HOUSE = 1000;
const int BUILDING = 2000;
const int RENT0 = 200;
const int RENT1 = 500;
const int RENT2 = 900;
const char* chance[5] =
{ "买彩票中奖 +1000元", "赌博失败 -2000元", "无事发生",
 "你今天开心 给各玩家500元", "你今天过生日 各玩家给你500元" };
const int x_axis[26] = { 75,185,280,370,465,565,675,675,675,675,675,675,675,675,565,465,370,280,185,75,75,75,75,75,75,75 };
const int y_axis[26] = { 58,58,58,58,58,58,58,160,245,325,410,490,575,675,675,675,675,675,675,675,575,490,410,325,245,160 };

// structs
typedef enum
{
    PLAYER_1 = 0,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4,
} Player;

typedef struct
{
    int Ingame;
    int Injail;
    int Inhospital;
    int money;
    int house[22];  // 记录玩家拥有的房产位置（0-25，-1表示没有）
    int houseCount; // 拥有房产数量
    int position;
    int jailCount;
    int hospitalCount;
    int isAI; // AI 标志（新增）
} PlayerStates;

// functions declaration
void Init();
void Timer_init();
void Timer_Update();
void Delay_ms(int x);
int Randomint(int range);
int Dice();
void Bankruptcy(Player current);
void ClearPlayer(Player current);
int Judge(Player now);
Player NextPlayer(Player current);
int Move(int i, int before, int step);
int SpecialArea(int now);
void ChanceEvent(Player current);
void BuyLand(Player current, int pos);
void BuildHouse(Player current, int pos);
void PayRent(Player current, int pos, int level);
int SellHouse(Player player);
void ShowStatus(Player player);
void ShowMap();
void ShowMoney();
int CanUpgrade(int pos);
void Pause();
void House_init();//加载房子图片
void Map_init();
void draw_house_one(int x, int y);//绘制低级房子
void draw_house_two(int x, int y);//绘制高级房子
void DrawBoardObjects();
void DrawAllCharacters(int exclusion);
void Player_init();
void Background_init();//加载背景图片
void update_background();//更新背景图片
void draw_jail(int x, int y);//绘制监狱图片
void draw_hospital(int x, int y);//绘制医院图片
void clear_house(int x, int y);//卖房函数
void draw_chance(int x, int y);//绘制机会图片
void Character_init();
void move_photo(IMAGE* photo, int& x, int& y, int target_x, int target_y, int speed);
IMAGE* Character(int i);
int a_step(IMAGE* photo, int& x, int& y, int target_x, int target_y, int speed, int playerindex);
int Move(int i, int before, int step);
void WriteWord(int x, int y, const char* word, ...);
void cleartextxy(int x, int y, int width, int height);
void draw_c1ground(int x, int y);//绘制角色1空地图片
void draw_c2ground(int x, int y);//绘制角色2空地图片
void draw_c3ground(int x, int y);//绘制角色3空地图片
void draw_c4ground(int x, int y);//绘制角色4空地图片

// arrays & numbers
PlayerStates states[4];
int map[26][2]; // map[i][0]: 拥有者 (-1:无主) ; map[i][1]: 建筑等级 (0:土地,1:一级,2:二级)
int flag = 1;
int num_player;
int game_round = 0;
int useFixedSeed = 0; // 是否使用固定种子（用于调试）

int main()
{
    // 选择随机种子模式
    printf("请选择随机种子模式：\n");
    printf("1. 使用当前时间作为种子（正常游戏）\n");
    printf("2. 使用固定种子（12345，用于调试）\n");
    printf("请选择(1-2): ");
    int seedChoice;
    scanf("%d", &seedChoice);
    if (seedChoice == 2) {
        srand(12345);  // 固定种子，方便调试
        useFixedSeed = 1;
    }
    else {
        srand((unsigned)time(NULL)); // 随机种子
    }

    // 输入玩家数量（1-4人）
    do {
        printf("请输入玩家数量(1-4): ");
        scanf("%d", &num_player);
        if (num_player < 1 || num_player > 4) {
            printf("玩家数量必须在1到4之间！\n");
        }
    } while (num_player < 1 || num_player > 4);
   
    // 初始化
    Init();
    Player current = PLAYER_1;
    WriteWord(195, 195, "\n===== 游戏开始！ =====\n");
    Delay_ms(2000);
    cleartextxy(STA, 195, STO, 50);

    // 主循环体
    while (flag)
    {
        game_round++;
        WriteWord(195, 195, "\n===== 第 %d 轮 =====\n", game_round);
        ShowMap(); // 显示当前地图状态
        ShowMoney(); // 显示当前玩家财富状态

        // 寻找可行动的玩家
        int skipped = 0;
        while (!Judge(current)) {
            WriteWord(195, 300, "玩家 %d 无法行动，跳过\n", current + 1);
            Delay_ms(2000);
            cleartextxy(STA, 300, STO, 50);
            current = NextPlayer(current);
            skipped++;
            if (skipped >= num_player) {
                printf("所有玩家都无法行动\n");
                Delay_ms(2000);
                cleartextxy(STA, 300, STO, 50);
                break;
            }
        }
        if (!Judge(current)) {
            current = NextPlayer(current);
            continue;
        }

        WriteWord(195, 300, "\n--- 玩家 %d 的回合 ---\n", current + 1);
        Delay_ms(2000);
        cleartextxy(STA, 300, STO, 50);
        ShowStatus(current);

        // 回合菜单
        int choice;
        do {
            WriteWord(195, R2, "\n请选择操作：\n");
            WriteWord(195, R3, "1. 掷骰子前进\n");
            WriteWord(195, R4, "2. 查看自己状态\n");
            WriteWord(195, R5, "3. 查看地图\n");
            WriteWord(195, R6, "4. 结束回合\n");
            WriteWord(195, R7, "请选择(1-4): ");
            
            if (!states[current].isAI) {
                if (scanf("%d", &choice) != 1) {
                    WriteWord(195, 300, "请输入有效数字！\n");
                    while (getchar() != '\n'); // 清理无效输入
                    continue;
                }
                while (getchar() != '\n'); // 清理换行符

                if (choice < 1 || choice > 4) {
                    WriteWord(195, 300, "请输入1-4之间的数字！\n");
                    continue;
                }
            }
            else {
                choice = 1; // 若为 AI 自动选择 1
                WriteWord(195, 300, "%d\n", choice);
                Delay_ms(1000);
                cleartextxy(STA, 300, STO, 50);
            }

            switch (choice) {
            case 1: {
                // 掷骰子
                WriteWord(195, 550, "按Enter键掷骰子...");
                if (states[current].isAI) {
                    Delay_ms(2000);
                }
                else {
                    getchar(); // 等待回车
                }
                cleartextxy(STA, 200, STO, 400);
                int dice = Dice();
                WriteWord(195, 350, "你掷出了 %d 点！\n", dice);
                FlushBatchDraw();
                Delay_ms(2000);
                cleartextxy(STA, 200, STO, 350);
                int beforePos = states[current].position;

                if ((beforePos + dice) >= 26) //起点奖励
                { 
                    if ((beforePos + dice) == 26)
                    {
                        WriteWord(195, 600, "到达起点, 奖励2000元!\n");
                        states[current].money += 2000;   //真正加钱
                    }
                    else
                    {
                        WriteWord(195, 600, "经过起点, 奖励1000元!\n");
                        states[current].money += 1000;   //真正加钱
                    }
                    Delay_ms(2000);
                    cleartextxy(STA, 600, STO, 50);
                }

                // 执行动画移动
                int newPos = Move(current, beforePos, dice);
                states[current].position = newPos;

                // 移动并更新位置
                WriteWord(195, 300, "你移动到了 %d 号地块\n", newPos);
                Delay_ms(2000);
                cleartextxy(STA, 300, STO, 50);

                // 处理地块事件
                int areaType;
                areaType = SpecialArea(newPos);
                switch (areaType)
                {
                case 0: // 特殊格
                    if (newPos == 6) {
                        WriteWord(195, 300, "住进医院，暂停一轮\n");
                        Delay_ms(2000);
                        cleartextxy(STA, 300, STO, 50);
                        states[current].Inhospital = 1;
                        states[current].hospitalCount = 1;
                    }
                    else if (newPos == 19) {
                        WriteWord(195, 300, "被关进监狱，暂停三轮\n");
                        Delay_ms(2000);
                        cleartextxy(STA, 300, STO, 50);
                        states[current].Injail = 1;
                        states[current].jailCount = 3;
                    }
                    else if (newPos == 13) {
                        WriteWord(195, 300, "你遇到了机会事件！\n");
                        Delay_ms(2000);
                        cleartextxy(STA, 300, STO, 50);
                        ChanceEvent(current);
                    }

                    break;
                case 1: // 空地
                    WriteWord(195, 300, "空地，价格 %d 元。\n", LAND);

                    if (states[current].money >= LAND) {
                        WriteWord(195, 350, "你是否购买？(1:是 0:否): ");

                        int buyChoice;
                        if (states[current].isAI) {
                            buyChoice = 1;
                            WriteWord(195, 350, "%d\n", buyChoice);
                            Delay_ms(500);
                        }
                        else {
                            scanf("%d", &buyChoice);
                        }
                        if (buyChoice == 1) {
                            BuyLand(current, newPos);
                        }
                    }
                    else {
                        WriteWord(195, 400, "你的金钱不足，无法购买\n");
                    }
                    Delay_ms(2000);
                    cleartextxy(STA, 300, STO, 150);
                    break;

                case 2: // 土地或一级房子
                    if (map[newPos][0] == current) {
                        WriteWord(195, 300, "这是你自己的地块。\n");
                        Delay_ms(2000);
                        cleartextxy(STA, 300, STO, 50);
                        if (CanUpgrade(newPos))
                        {
                            if (map[newPos][1] == 0)
                            {
                                WriteWord(195, 300, "是否建造一级房%d 元 ？(1:是 0:否): ", HOUSE);
                                int buildChoice;
                                if (states[current].isAI) {
                                    buildChoice = 1;
                                    WriteWord(195, 300, "%d\n", buildChoice);
                                    Delay_ms(500);
                                }
                                else {
                                    scanf("%d", &buildChoice);
                                }
                                if (buildChoice == 1)
                                {
                                    BuildHouse(current, newPos);
                                }
                            }
                            else if (map[newPos][1] == 1)
                            {
                                WriteWord(195, 300, "是否升级二级房 %d 元？ (1:是 0:否): ", BUILDING);
                                int upgradeChoice;
                                if (states[current].isAI) {
                                    upgradeChoice = 1;
                                    WriteWord(195, 300, "%d\n", upgradeChoice);
                                    Delay_ms(500);
                                }
                                else {
                                    scanf("%d", &upgradeChoice);
                                }
                                if (upgradeChoice == 1)
                                {
                                    if (states[current].money >= BUILDING)
                                    {
                                        states[current].money -= BUILDING;
                                        map[newPos][1] = 2;
                                        WriteWord(195, 350, "升级成功！\n");
                                    }
                                    else
                                    {
                                        WriteWord(195, 350, "金钱不足，升级失败\n");
                                    }
                                }
                            }
                        }
                        else {
                            WriteWord(195, 300, "必须一级一级升级\n");
                        }
                    }
                    else {
                        int owner = map[newPos][0];
                        if (!map[newPos][1]) {
                            WriteWord(195, 300, "玩家 %d 的地块，支付 %d 元\n", owner + 1, RENT0);
                            PayRent(current, newPos, 0);
                        }
                        else {
                            WriteWord(195, 300, "玩家 %d 的一级房，支付 %d 元\n", owner + 1, RENT1);
                            PayRent(current, newPos, 1);
                        }
                    }
                    Delay_ms(2000);
                    cleartextxy(STA, 300, STO, 100);
                    break;

                case 3: // 二级房子
                    if (map[newPos][0] == current) {
                        WriteWord(195, 300, "这是你自己的二级房子\n");
                    }
                    else {
                        int owner = map[newPos][0];
                        WriteWord(195, 300, "玩家 %d 的二级房，支付 %d 元\n", owner + 1, RENT2);
                        PayRent(current, newPos, 2);
                    }
                    Delay_ms(2000);
                    cleartextxy(STA, 300, STO, 100);
                    break;
                }
                // 检查玩家是否破产
                if (states[current].money < 0) {
                    WriteWord(195, 300, "玩家 %d 资金为负！请变卖房产偿还债务\n", current + 1);
                    Bankruptcy(current);
                   
                    // 循环结束后判断是否破产
                    ClearPlayer(current);
                }
                choice = 4; // 移动后自动结束回合
                break;
            }

            case 2:
                ShowStatus(current);
                break;

            case 3:
                ShowMap();
                break;

            case 4:
                WriteWord(195, 300, "玩家 %d 结束回合\n", current + 1);
                Delay_ms(2000);
                cleartextxy(STA, 300, STO, 100);
                break;

            default:
                printf("无效选择，请重新输入\n");

            }
        } while (choice != 4);

        // 检查游戏是否结束（只剩一个玩家）
        int aliveCount = 0;
        int winner = -1;
        for (int i = 0; i < 4; i++) {
            if (states[i].Ingame) {
                aliveCount++;
                winner = i;
            }
        }
        if (aliveCount <= 1) {
            cleartextxy(STA, 195, STO, 50);
            WriteWord(195, 195, "\n===== 游戏结束！ =====\n");
            if (aliveCount == 0) {//补充
                WriteWord(195, 300, "所有玩家均已破产，游戏无胜利者！\n");
            }
            else {
                WriteWord(195, 300, "玩家 %d 获胜！\n", winner + 1);
                WriteWord(195, 350, "游戏共进行了 %d 轮\n", game_round);
            }
            flag = 0;
            Delay_ms(2000);
            cleartextxy(STA, 195, STO, 200);
            break;
        }

        // 切换到下一个玩家
        current = NextPlayer(current);
        WriteWord(195, 195, "\n按Enter键继续下一回合...");
        Delay_ms(2000);
        cleartextxy(STA, 195, STO, 50);
        if (!states[current].isAI) getchar(); getchar(); // AI
    }
    return 0;
}

void Init()
{
    Player_init();
    Background_init();
    update_background();
    House_init();
    Character_init();
    Timer_init();
    mciSendStringA(R"(open "res\Richman.mp3" alias music)", nullptr, 0, nullptr);
    mciSendStringA("play music repeat", nullptr, 0, nullptr);
    Map_init();
    DrawAllCharacters(-1);
}

void Timer_init()
{
    last_time = clock();
}

void Timer_Update()
{
    clock_t current_time = clock();
    delta_time = (float)(current_time - last_time) / CLOCKS_PER_SEC;
    last_time = current_time;
}

void Delay_ms(int x)
{
    Sleep(x);
    Timer_Update();
}

int Randomint(int range)
{
    return rand() % range + 1;
}

int Dice()
{
    return Randomint(6);
}

void Bankruptcy(Player current)
{
    while (states[current].money < 0 && states[current].houseCount > 0) {
        WriteWord(195, 350, "当前债务: %d 元\n", -states[current].money);
        int soldPos = SellHouse(current);
        if (soldPos == -1) {
            break; // 用户取消卖房
        }

        // 从玩家房产列表中移除
        for (int i = 0; i < states[current].houseCount; i++) {
            if (states[current].house[i] == soldPos) {
                // 移动后面的元素覆盖当前位置
                for (int j = i; j < states[current].houseCount - 1; j++) {
                    states[current].house[j] = states[current].house[j + 1];
                }
                states[current].houseCount--;
                break;
            }
        }
        Delay_ms(2000);
        cleartextxy(STA, 300, STO, 100);
        // 重置地块
        map[soldPos][0] = -1;
        map[soldPos][1] = 0;
        clear_house(x_axis[soldPos], y_axis[soldPos]);//清空该地块图片
    }
}

void ClearPlayer(Player current)
{
    if (states[current].money < 0) {
        WriteWord(195, 300, "玩家 %d 破产出局！\n", current + 1);
        states[current].Ingame = 0;
        // 将该玩家的地块全部释放
        for (int i = 0; i < 26; i++) {
            if (map[i][0] == current) {
                map[i][0] = -1;
                map[i][1] = 0;
                clear_house(x_axis[i], y_axis[i]);//清空该地块图片
            }
        }
        Delay_ms(2000);
        cleartextxy(STA, 300, STO, 100);
        //补充：清除玩家图形
        int pos = states[current].position;//获取玩家位置
        putimage(x_axis[pos], y_axis[pos], &whitebg);
    }
}

int Judge(Player now)
{
    // 更新医院状态
    if (states[now].Inhospital) {
        states[now].hospitalCount--;
        if (states[now].hospitalCount == 0) {
            states[now].Inhospital = 0;
            WriteWord(195, 600, "玩家 %d 出院了\n", now + 1);
            Delay_ms(2000);
            cleartextxy(STA, 600, STO, 50);
        }
    }
    // 更新监狱状态
    if (states[now].Injail) {
        states[now].jailCount--;
        if (states[now].jailCount == 0) {
            states[now].Injail = 0;
            WriteWord(195, 600, "玩家 %d 出狱了\n", now + 1);
            Delay_ms(2000);
            cleartextxy(STA, 600, STO, 50);
        }
    }

    if (states[now].Ingame && !states[now].Inhospital && !states[now].Injail)
        return 1;
    return 0;
}

Player NextPlayer(Player current) //AI
{
    return (Player)((current + 1) % 4);
}

int SpecialArea(int now)
{
    if (map[now][0] == -2) // 特殊地块
        return 0;
    if (map[now][0] == -1) // 空地
        return 1;
    if (map[now][1] == 2)  // 二级房子
        return 3;
    return 2;              // 土地或一级房子
}

int CanUpgrade(int pos)
{
    // 检查是否可以升级（必须按顺序：土地→一级→二级）
    if (map[pos][1] == 0) {
        // 土地可以建一级房子
        return 1;
    }
    else if (map[pos][1] == 1) {
        // 一级房子可以升级为二级
        return 1;
    }
    return 0; // 已经是二级，不能再升级
}

void ChanceEvent(Player current)
{
    int index = rand() % 5;
    WriteWord(195, 550, "机会: %s\n", chance[index]);

    switch (index) {
    case 0: // 中奖
        states[current].money += 1000;
        WriteWord(195, 600, "获得1000元\n");
        Delay_ms(2000);
        cleartextxy(STA, 550, STO, 100);
        break;

    case 1: // 赌博失败
        states[current].money -= 2000;
        WriteWord(195, 600, "失去2000元\n");
        Delay_ms(2000);
        cleartextxy(STA, 550, STO, 100);
        break;

    case 2: // 无事发生
        WriteWord(195, 600, "无事发生\n");
        Delay_ms(2000);
        cleartextxy(STA, 550, STO, 100);
        break;

    case 3: // 给其他玩家钱
    {
        int totalNeeded = 0;
        // 先计算需要多少钱（包含 AI 补位，遍历 4 个槽）
        for (int i = 0; i < 4; i++) {
            if (i != current && states[i].Ingame) {
                totalNeeded += 500;
            }
        }
        int bankrupt = 0;
        if (states[current].money < totalNeeded) bankrupt = 1;
        for (int i = 0; i < 4; i++) {
            if (i != current && states[i].Ingame) {
                states[current].money -= 500;
                states[i].money += 500;
                WriteWord(195, 600, "给玩家 %d 500元\n", i + 1);
            }
        }
        if(bankrupt) {
            WriteWord(195, 300, "玩家 %d 资金为负！请变卖房产偿还债务\n", current + 1);
            Bankruptcy(current);
            ClearPlayer(current);
        }
    }
    Delay_ms(2000);
    cleartextxy(STA, 550, STO, 100);
    break;

    case 4: // 收其他玩家钱
    {
        int canExecute[4] = { 1, 1, 1, 1 };
        int now = current;
        // 先检查所有其他玩家是否有足够钱（包含 AI 补位）
        for (int i = 0; i < 4; i++) {
            if (i != current && states[i].Ingame && states[i].money < 500) 
                canExecute[i] = 0;
        }
        for (int i = 0; i < 4; i++) {
            if (i != current && states[i].Ingame) {
                states[i].money -= 500;
                states[current].money += 500;
                WriteWord(195, 600, "收到玩家 %d 500元\n", i + 1);
                Delay_ms(2000);
                cleartextxy(STA, 550, STO, 100);
            }
        }
        for (int i = 0; i < 4; i++) {
            if (!canExecute[i]) {
                WriteWord(195, 300, "玩家 %d 资金为负！请变卖房产偿还债务\n", i+1);
                Bankruptcy(Player(i));
                ClearPlayer(Player(i));
            }
        }
    }
    break;
    }
}

void BuyLand(Player current, int pos)
{
    if (states[current].money >= LAND) {
        states[current].money -= LAND;
        map[pos][0] = current;
        map[pos][1] = 0;
        // 添加到玩家房产列表
        states[current].house[states[current].houseCount] = pos;
        states[current].houseCount++;
        WriteWord(195, 600, "购买成功！\n");
        FlushBatchDraw();
        Delay_ms(1000);

        DrawBoardObjects();
        DrawAllCharacters(-1);
    }
    else {
        WriteWord(195, 600, "金钱不足，购买失败\n");
        FlushBatchDraw();
        Delay_ms(2000);
    }
    cleartextxy(STA, 550, STO, 100);
}

void BuildHouse(Player current, int pos)
{
    if (states[current].money >= HOUSE) {
        states[current].money -= HOUSE;
        map[pos][1] = 1;
        WriteWord(195, 600, "建造一级房子成功！\n");
    }
    else {
        WriteWord(195, 600, "金钱不足，建造失败\n");
    }
    Delay_ms(2000);
    cleartextxy(STA, 550, STO, 100);
    DrawBoardObjects();
    DrawAllCharacters(-1);
}

void PayRent(Player current, int pos, int level)
{
    int owner = map[pos][0];

    int rent;
    switch (level) {
    case 0: rent = RENT0; break;
    case 1: rent = RENT1; break;
    case 2: rent = RENT2; break;
    }

    if (states[current].money >= rent) {
        states[current].money -= rent;
        states[owner].money += rent;
        WriteWord(195, 600, "支付 %d 元给玩家 %d\n", rent, owner + 1);
    }
    else {
        WriteWord(195, 450, "金钱不足，无法支付 %d 元\n", rent);
        WriteWord(195, 500, "请变卖房产筹集资金...\n");

        int debt = rent - states[current].money;
        WriteWord(195, 550, "需要筹集 %d 元\n", debt);
        Bankruptcy(current);

        // 再次检查资金是否足够
        if (states[current].money >= rent) {
            states[current].money -= rent;
            states[owner].money += rent;
            WriteWord(195, 600, "支付租金 %d 元给玩家 %d\n", rent, owner + 1);
        }
        else {
            WriteWord(195, 600, "仍然无法支付，破产处理\n");
            states[current].money -= rent; // 变为负数
        }
    }
    Delay_ms(2000);
    cleartextxy(STA, 450, STO, 200);
    DrawBoardObjects();
    DrawAllCharacters(-1);
}

int SellHouse(Player player)
{
    if (states[player].houseCount == 0) {
        printf("没有房产可卖\n");
        return -1;  // 返回-1表示没有房产或取消
    }

    printf("\n你的房产列表：\n");
    for (int i = 0; i < states[player].houseCount; i++) {
        int pos = states[player].house[i];
        int value = 0;
        if (map[pos][1] == 0) {
            value = LAND * 7 / 10; // 土地打7折
        }
        else if (map[pos][1] == 1) {
            value = (LAND + HOUSE) * 7 / 10; // 土地+一级房子打7折
        }
        else {
            value = (LAND + HOUSE + BUILDING) * 7 / 10; // 全套打7折
        }
        printf("%d. 地块 %d (等级:%d) 价值:%d元\n",
            i + 1, pos, map[pos][1], value);
    }

    printf("选择要卖的房产编号(0取消): ");
    int choice;
    if (states[player].isAI)
    {
        choice = Randomint(states[player].houseCount); // AI 随机选择要卖的房产（1..count）
        printf("%d\n", choice);
    }
    else
    {
        scanf("%d", &choice);
    }

    if (choice < 1 || choice > states[player].houseCount) {
        printf("取消卖房\n");
        return -1;
    }

    int idx = choice - 1;
    int pos = states[player].house[idx];

    // 计算卖价（打7折）
    int sellPrice = 0;
    if (map[pos][1] == 0) {
        sellPrice = LAND * 7 / 10;
    }
    else if (map[pos][1] == 1) {
        sellPrice = (LAND + HOUSE) * 7 / 10;
    }
    else {
        sellPrice = (LAND + HOUSE + BUILDING) * 7 / 10;
    }

    // 卖房获得资金
    states[player].money += sellPrice;
    printf("卖房获得 %d 元\n", sellPrice);

    // 只返回卖出的地块位置，不处理状态更新
    printf("当前资金: %d 元\n", states[player].money);
    DrawBoardObjects();
    DrawAllCharacters(-1);
    return pos;  // 返回卖出的地块位置
}

void ShowStatus(Player player)
{
    printf("\n玩家 %d 状态：\n", player + 1);
    printf("资金: %d 元\n", states[player].money);
    printf("位置: %d 号地块\n", states[player].position);
    printf("状态: ");
    if (states[player].Inhospital) {
        printf("住院中（剩余%d轮）\n", states[player].hospitalCount);
    }
    else if (states[player].Injail) {
        printf("坐牢中（剩余%d轮）\n", states[player].jailCount);
    }
    else {
        printf("正常\n");
    }
    printf("房产数量: %d\n", states[player].houseCount);
    if (states[player].houseCount > 0) {
        printf("房产列表: ");
        for (int i = 0; i < states[player].houseCount; i++) {
            int pos = states[player].house[i];
            printf("%d号地(等级%d) ", pos, map[pos][1]);
        }
        printf("\n");
    }
}

void ShowMap()
{
    printf("\n===== 地图状态 =====\n");
    printf("地块\t拥有者\t等级\t类型\n");
    printf("----\t------\t----\t----\n");

    for (int i = 0; i < 26; i++) {
        printf("%2d\t", i);

        if (map[i][0] == -2) {
            if (i == 0) printf("起点\t-\t特殊\n");
            else if (i == 6) printf("医院\t-\t特殊\n");
            else if (i == 13) printf("机会\t-\t特殊\n");
            else if (i == 19) printf("监狱\t-\t特殊\n");
        }
        else if (map[i][0] == -1) {
            printf("无主\t%d\t空地\n", map[i][1]);
        }
        else {
            printf("玩家%d\t%d\t", map[i][0] + 1, map[i][1]);
            if (map[i][1] == 0) printf("土地\n");
            else if (map[i][1] == 1) printf("一级房\n");
            else printf("二级房\n");
        }
    }

    printf("=====================\n");
}

void ShowMoney()
{
    printf("\n===== 玩家财富状态 =====\n");
    printf("Player1: %d 元\n", states[0].money);
    printf("Player2: %d 元\n", states[1].money);
    printf("Player3: %d 元\n", states[2].money);
    printf("Player4: %d 元\n", states[3].money);

    printf("=====================\n");
}

void DrawBoardObjects()
{
    for (int i = 0; i < 26; ++i)
    {
        // 特殊格
        if (map[i][0] == -2)
        {
            if (i == 6)      draw_hospital(x_axis[i], y_axis[i]);
            else if (i == 19) draw_jail(x_axis[i], y_axis[i]);
            else if (i == 13) draw_chance(x_axis[i], y_axis[i]);
            // i == 0 起点，不需要单独图片就可以不画
            continue;
        }

        // 普通格：看建筑等级
        if (map[i][0] == -1)
        {
            // 无房产，清空成白底（可选）
            clear_house(x_axis[i], y_axis[i]);
        }
        else
        {
            // 有房产：根据等级画房子
            if (map[i][1] == 0)
            {
                switch (map[i][0])
                {
                case 0: {
                    draw_c1ground(x_axis[i], y_axis[i]);
                    break;
                }
                case 1: {
                    draw_c2ground(x_axis[i], y_axis[i]);
                    break;
                }
                case 2: {
                    draw_c3ground(x_axis[i], y_axis[i]);
                    break;
                }
                case 3: {
                    draw_c4ground(x_axis[i], y_axis[i]);
                    break;
                }
                }
            }
            else if (map[i][1] == 1)
            {
                draw_house_one(x_axis[i], y_axis[i]);
            }
            else if (map[i][1] == 2)
            {
                draw_house_two(x_axis[i], y_axis[i]);
            }
        }
    }
}

void DrawAllCharacters(int exclusion = -1)
{
    // 改为遍历 4 个槽（包含 AI 补位）
    for (int i = 0; i < 4; ++i)
    {
        if (!states[i].Ingame) continue;     // 已破产玩家不画
        if (i == exclusion) continue;      // 排除指定玩家
        int pos = states[i].position;        // 逻辑位置 0-25
        int x = x_axis[pos];
        int y = y_axis[pos];

        IMAGE* photo = Character(i);
        putimage(x, y, photo);
    }
}

void Pause()
{
    printf("按Enter键继续...");
    getchar(); getchar();
}

void House_init()//加载房子图片
{
    //将图片放到内存变量里,再从变量里调用
    loadimage(&imghouse_one, "res/house_one.png", 60, 56);
    loadimage(&imghouse_two, "res/house_two.png", 50, 60);
    loadimage(&imgjail, "res/jail.png", 55, 57);
    loadimage(&imghospital, "res/hospital.png", 55, 55);
    loadimage(&imgchance, "res/chance.png", 55, 55);
    //
    //
    //看到一个图片少加载了，顺便加上
    //
    //
    loadimage(&whitebg, "res/whitebg.png", 60, 60);//加载白色背景图片
    //新增
    loadimage(&c1ground, "res/c1ground.png", 60, 60);//加载玩家一空地图片
    loadimage(&c2ground, "res/c2ground.png", 60, 60);//加载玩家二空地图片
    loadimage(&c3ground, "res/c3ground.png", 60, 60);//加载玩家三空地图片
    loadimage(&c4ground, "res/c4ground.png", 60, 60);//加载玩家四空地图片

}

void Map_init() {
    // 初始化地图
    for (int i = 0; i < 26; i++) {
        map[i][0] = -1;
        map[i][1] = 0;
    }
    // 标记特殊地块（不能购买）
    // 0:起点, 6:医院, 13:机会, 19:监狱
    map[0][0] = -2;  // -2表示特殊地块
    map[6][0] = -2;
    map[13][0] = -2;
    map[19][0] = -2;
    draw_hospital(x_axis[6], y_axis[6]);// 绘制医院图片
    draw_jail(x_axis[19], y_axis[19]);// 绘制监狱图片
    draw_chance(x_axis[13], y_axis[13]);// 绘制机会图片
}

void draw_house_one(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &imghouse_one);//建造低级房子
}

void draw_house_two(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &imghouse_two);//建造高级房子
}

void draw_jail(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &imgjail);//建造监狱
}

void clear_house(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
}

void draw_hospital(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &imghospital);//建造医院
}

void draw_chance(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &imgchance);//建造机会格
}

void draw_c1ground(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &c1ground);//建造玩家一空地
}
void draw_c2ground(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &c2ground);//建造玩家二空地
}
void draw_c3ground(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &c3ground);//建造玩家三空地
}
void draw_c4ground(int x, int y)
{
    putimage(x, y, &whitebg);//清空本格子区域
    putimage(x, y, &c4ground);//建造玩家四空地
}

void Player_init()
{
    // 初始化玩家状态
    for (int i = 0; i < 4; i++) {
        // 默认把真实玩家标记为 Ingame = 1，超出索引的槽位先设为 0
        states[i].Ingame = (i < num_player) ? 1 : 0;
        states[i].Injail = 0;
        states[i].Inhospital = 0;
        states[i].money = 5000; // 初始资金调整为5000
        states[i].position = 0;
        states[i].jailCount = 0;
        states[i].hospitalCount = 0;
        states[i].houseCount = 0;
        states[i].isAI = 0; // 先清 0
        for (int j = 0; j < 22; j++) states[i].house[j] = -1;
    }

    // AI 初始化：若 num_player < 4，则用 AI 补位
    if (num_player != 4)
    {
        for (int i = 0; i < num_player; i++) states[i].isAI = 0;
        for (int i = num_player; i < 4; i++) {
            states[i].isAI = 1;
            states[i].Ingame = 1; // AI 补位后也设为在场
        }
    }
    else {
        for (int i = 0; i < 4; i++) states[i].isAI = 0;
    }
}

void Background_init()//加载背景图片
{
    //初始化窗口
    initgraph(WIN_width, WIN_height);
    //加载游戏背景图片
    loadimage(&imgbg, "res/background.png", 800, 800);
    //图片加载到内存变量imgbg里
}

void update_background()//更新背景图片
{
    putimage(0, 0, &imgbg);//把图片变量里的图片画到窗口上
}
void move_photo(IMAGE* photo, int& x, int& y, int target_x, int target_y, int speed)
{
    //移动图片的函数
    double dx = target_x - x;
    double dy = target_y - y;
    double distance = sqrt(dx * dx + dy * dy);

    if (distance < 1.0)
    {
        x = target_x;
        y = target_y;
    }
    else {
        double ex = dx / distance;
        double ey = dy / distance;
        double step = speed * delta_time;
        if (step > distance) step = distance;
        x += (int)(ex * step);
        y += (int)(ey * step);
    }

    putimage(x, y, photo);
}

IMAGE* Character(int i)
{
    switch (i)
    {
    case 0: return &character1;
    case 1: return &character2;
    case 2: return &character3;
    case 3: return &character4;
    default: return &character1;
    }
}

int Move(int i, int before, int step)
{
    int result = 0;
    IMAGE* photo = Character(i);
    int idx = 0;
    while (idx < step)
    {
        int current_pos = (before + idx) % 26;
        int next_pos = (before + idx + 1) % 26;
        int x = x_axis[(before + idx) % 26];
        int y = y_axis[(before + idx) % 26];
        int target_x = x_axis[(before + idx + 1) % 26];
        int target_y = y_axis[(before + idx + 1) % 26];
        //判断是否经过起点(位置0)

        a_step(photo, x, y, target_x, target_y, 150, i);
        idx++;
    }
    result = (before + step) % 26;
    return result;
}

int a_step(IMAGE* photo, int& x, int& y, int target_x, int target_y, int speed, int playerindex)
{
    BeginBatchDraw(); // 开始批绘制（与 EndBatchDraw 配对）
    while (!(x == target_x && y == target_y))
    {
        Timer_Update();

        // 清屏并绘制背景
        cleardevice();
        update_background();

        DrawBoardObjects();
        DrawAllCharacters(playerindex);
        // 绘制角色移动
        move_photo(photo, x, y, target_x, target_y, speed);

        // 一次性刷新到屏幕
        FlushBatchDraw();

        Sleep(16); // 控制帧率 ~60FPS
    }
    EndBatchDraw();
    return 1;
}

void Character_init()
{
    //加载角色图片
    loadimage(&character1, "res/character1.jpg");
    loadimage(&character2, "res/character2.jpg");
    loadimage(&character3, "res/character3.jpg");
    loadimage(&character4, "res/character4.jpg");
}

void WriteWord(int x, int y, const char* word, ...) {
    char temp[256];
    va_list args;
    va_start(args, word);
    vsnprintf(temp, sizeof(temp), word, args);
    va_end(args);

    settextcolor(RED);
    setbkmode(TRANSPARENT);
    settextstyle(25, 0, "宋体");

    outtextxy(x, y, temp); // 使用格式化后的字符串
    printf("%s", temp);
}

void cleartextxy(int x, int y, int width, int height)
{
    setfillcolor(RGB(167, 229, 240)); // 背景色
    solidrectangle(x, y, x + width, y + height);
}