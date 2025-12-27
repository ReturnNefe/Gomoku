# Gomoku - 五子棋

简洁的五子棋游戏，有 UI 和控制台两个版本。

## 📦 包含内容

- **gomoku_ui.exe** - 图形界面版本
- **gomoku_console.exe** - 控制台版本

## 🎮 怎么玩

### UI 版本

1. 运行 `gomoku_ui.exe`
2. 在棋盘上点击放置黑棋
3. AI 会自动下白棋
4. 先连成 5 个棋子的一方获胜
5. 点击 "Restart" 按钮重新开始

### 控制台版本

1. 运行 `gomoku_console.exe`
2. 输入坐标放置棋子（例如：A1、B5）
3. AI 会随之下棋
4. 先连成 5 个棋子的一方获胜
5. 输入 "quit" 退出游戏

## 🎯 游戏规则

- 棋盘：15×15 的网格
- 你操作黑棋，AI 操作白棋
- 首先连成 5 个棋子（横、竖、斜）即为胜利
- 支持平局

## 🤖 AI 特性

- 使用 Alpha-Beta 剪枝的 Minimax 算法
- 智能评估棋盘局势
- 既能进攻也能防守

## 📋 系统要求

- Windows 操作系统
- 不需要额外安装任何依赖

---

Created by Tianjian Chen (Acton Chen)