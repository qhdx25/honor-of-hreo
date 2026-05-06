# 游戏运行逻辑图

下面两张图都可以直接复制到支持 Mermaid 的编辑器里导出，或者在支持 Mermaid 的 Markdown 预览中截图后放进 Word。

## 预览提示

- `Ctrl+Shift+V`：打开 Markdown 预览
- `Ctrl+K V`：在侧边打开 Markdown 预览
- 如果 Mermaid 没有显示，通常是 VS Code 版本太旧，更新后即可正常预览

![游戏地图预览](./res/wangzhe_gorge_playfield.png)

## 1. 游戏运行时逻辑图

```mermaid
flowchart TD
    A["main.cpp\n创建 QApplication 和 MainWindow"] --> B["MainWindow::MainWindow()\n初始化资源、对象、计时器、技能图标"]
    B --> C["startGame()\n点击开始后重置状态并启动计时器"]
    C --> D["m_gameTimer(timeout=16ms)"]
    C --> E["m_enemyTimer(timeout=1800ms)"]

    D --> F["updateBullets()\n主循环入口"]
    F --> F1["updateHeroMovement()\n读取 WASD 并更新英雄位置"]
    F --> F2["updateSkillCooldowns()\n更新技能冷却和按钮状态"]
    F --> F3["updateSkill2Effects()/updateFlashState()/updateDragonEffects()"]
    F --> F4["updateMedicinePack()\n药包刷新逻辑"]
    F --> F5["宠物 / 防御塔 / 水晶自动攻击"]
    F --> F6["更新玩家子弹、敌方子弹、宠物子弹"]
    F --> F7["碰撞检测\n扣血 / 击杀 / 经验增长"]
    F --> F8["updateEnemies()\n敌人追击与攻击"]
    F --> F9["判断胜利或失败"]
    F --> G["update()\n请求重绘"]

    E --> H["spawnEnemy()\n随机生成普通敌人"]

    G --> I["paintEvent()\n绘制地图、英雄、敌人、子弹、建筑、特效、UI"]

    J["键盘事件\nkeyPressEvent()/keyReleaseEvent()"] --> F1
    K["鼠标左键\nmousePressEvent()"] --> L["创建普通子弹"]
    M["技能图标拖拽\nSkillIconWidget"] --> N["beginSkillAim()/updateSkillAim()/releaseSkill()"]
    N --> O["castSkill1/2/3/6/7/Flash/Treatment"]
    O --> F6

    F9 --> P["startVictorySequence()"]
    F9 --> Q["startDefeatSequence()"]
    P --> R["returnToMainMenu()"]
    Q --> R
```

## 2. 各 cpp 文件之间关系图

```mermaid
flowchart LR
    A["main.cpp"] --> B["mainwindow.cpp"]

    B --> C["hero.cpp"]
    B --> D["enemy.cpp"]
    B --> E["bullet.cpp"]
    B --> F["skill2bullet.cpp"]
    B --> G["boomerangbullet.cpp"]
    B --> H["dragontornadobullet.cpp"]
    B --> I["enemybullet.cpp"]
    B --> J["enemybullet2.cpp"]
    B --> K["bullet3.cpp"]
    B --> L["bullet4.cpp"]
    B --> M["bullet5.cpp ~ bullet16.cpp"]
    B --> N["crystal.cpp"]
    B --> O["tower.cpp"]
    B --> P["pet.cpp"]
    B --> Q["medicinepack.cpp"]
    B --> R["skilliconwidget.cpp"]
    B --> S["dragonenemy.cpp"]
    B --> T["boss2enemy.cpp"]

    S --> D
    T --> D
    F --> E
    G --> E
    H --> E
    I --> E
    J --> E
    K --> E
    L --> E
    M --> E

    B --> U["游戏总控制器"]
    C --> V["英雄数据与升级"]
    D --> W["敌人属性 / 追击 / 攻击"]
    E --> X["基础弹道逻辑"]
    N --> Y["敌方水晶"]
    O --> Z["防御塔"]
    P --> AA["召唤宠物"]
    Q --> AB["药包系统"]
    R --> AC["技能图标与拖拽施法"]
```

## 3. 建议放进 Word 的说明文字

可以在图下方写：

“从程序结构上看，`mainwindow.cpp` 是整个项目的核心调度中心，它负责连接输入、计时器、渲染和战斗逻辑。`hero.cpp`、`enemy.cpp`、`bullet.cpp` 等文件分别负责不同对象的属性与行为；而 `skill2bullet.cpp`、`boomerangbullet.cpp`、`dragontornadobullet.cpp` 等文件则在基础子弹类的基础上扩展出不同技能效果。整个程序形成了较清晰的主控层、对象层和特效层结构。”
