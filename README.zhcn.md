# Gomoku (五子棋)

一款基于 C++ 实现的五子棋对弈程序，支持控制台和图形界面双版本。

![游戏界面](screenshots/ui.jpg)

## 构建 & 运行

### 前置要求
- MinGW-w64 (g++)
- Raylib 库（GUI 版本需要）

### 编译
```powershell
# 编译所有版本
.\build.ps1 -Target all

# 仅编译控制台版本
.\build.ps1 -Target console

# 仅编译 GUI 版本
.\build.ps1 -Target raylib
```

### 运行
```powershell
# 控制台版本
.\bin\gomoku_console.exe

# GUI 版本
.\bin\gomoku_ui.exe
```

## 如何使用

### 控制台版本
- 输入格式：`A1` - `O15`（列字母 + 行数字）
- 输入 `quit` 退出游戏

### GUI 版本
- 鼠标点击棋盘落子
- 点击 "Restart" 按钮重新开始
- 黑棋（玩家）/白棋（AI）

## 技术栈

### 核心
- **C++17**：使用现代 C++ 特性
- **Raylib**：轻量级图形库，提供优雅的 UI 渲染
- **多线程异步**：使用 `std::async` 实现非阻塞 AI 计算

### 构建工具
- **PowerShell 构建脚本**：自动化编译流程
- **MinGW-w64 (g++)**：静态链接编译，生成独立可执行文件

## 核心算法

### 1. Minimax 搜索 + Alpha-Beta 剪枝
- **搜索深度**：5 层（可配置）
- **剪枝优化**：减少约 60-70% 的搜索节点
- **时间复杂度**：O(b^d) → O(b^(d/2)) 剪枝后

```cpp
// 核心搜索逻辑（ai.h）
int minimax(Board &board, Role role, int depth, Point lastMove, int alpha, int beta)
```

### 2. 启发式评估函数
- **模式识别**：识别五连、活四、冲四、活三等 7 种棋型
- **评分系统**：
  - 五连：1e6 分
  - 活四：1e5 分
  - 冲四：1e4 分
  - 活三：8e3 分
- **攻防平衡**：防御权重 1.2（可配置的 K-Value）

```cpp
// 评估公式
Score = Σ(My Patterns) - 1.2 × Σ(Opponent Patterns)
```

### 3. 候选位置剪枝
- **搜索范围限制**：只搜索已有棋子周围 2 格范围（225→~50 候选点）
- **启发式排序**：优先搜索攻防价值高的位置，提升剪枝效率

```cpp
// 候选点排序（board.h）
vector<Point> getSortedCandidates(Role role)
```

### 4. 零拷贝线性视图 (LineView)
- **迭代器模式**：支持 range-based for 循环
- **按需计算**：不分配额外内存，直接访问棋盘数据
- **高效遍历**：分析四个方向（横/竖/斜）的棋型

## 项目特点

### 设计
1. **Morandi 配色方案**：现代 UI 设计
2. **模块化架构**：
   - `types.h` - 基础类型定义
   - `board.h` - 棋盘逻辑
   - `ai.h` - AI 决策
   - 独立 UI 层（console.cpp / game.cpp）

### 性能
1. **异步 AI 计算**：主线程不阻塞，流畅的 UI 响应（60 FPS）
2. **剪枝策略**：
   - Alpha-Beta 剪枝减少搜索树
   - 候选点过滤（225→50）
   - 启发式排序提升剪枝命中率
3. **静态链接编译**：无需额外 DLL，单文件发布

### 体验
1. **实时动画**：
   - AI 思考指示器（旋转加载动画）
   - 游戏结束横幅滑动效果
   - 悬停预览半透明棋子
2. **高 DPI 支持**：适配高分辨率屏幕
3. **双版本**
   - 控制台版：轻量级，适合快速对弈
   - Raylib 版：精美 UI，完整游戏体

## 项目结构
```
Gomoku/
├── src/
│   ├── headers/
│   │   ├── types.h      # 基础类型定义
│   │   ├── board.h      # 棋盘逻辑 + 棋局评估
│   │   └── ai.h         # Minimax + Alpha-Beta 剪枝
│   ├── console.cpp      # 控制台版本
│   └── game.cpp         # 图形界面版本
├── build.ps1            # 自动化构建脚本
```

## 算法复杂度分析

| 阶段 | 时间复杂度 | 说明 |
|------|-----------|------|
| 候选点生成 | O(n²) | n=15 |
| 候选点排序 | O(m log m) | m≈50 |
| Minimax 搜索 | O(b^d) | b≈50, d=5 |
| Alpha-Beta 剪枝后 | O(b^(d/2)) | 实际 b≈20-30 |
| 单步评估 | O(n) | 4 方向 × 9 格扫描 |

## License

MIT License