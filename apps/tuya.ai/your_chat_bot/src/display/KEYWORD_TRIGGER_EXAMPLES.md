# 关键词触发表盘切换示例

## 概述

现在用户可以通过简单的关键词来触发表盘样式切换，无需复杂的语音命令。

## 支持的关键词

### 1. 动漫风格触发词
- **"动漫"** - 直接触发动漫风格
- **"anime"** - 英文触发词
- **"卡通"** - 卡通风格触发
- **"可爱"** - 可爱风格触发

### 2. 时尚风格触发词
- **"时尚"** - 直接触发时尚风格
- **"fashion"** - 英文触发词
- **"优雅"** - 优雅风格触发
- **"奢华"** - 奢华风格触发

### 3. 科技风格触发词
- **"科技"** - 直接触发科技风格
- **"tech"** - 英文触发词
- **"未来"** - 未来风格触发
- **"数字"** - 数字风格触发

### 4. 森林风格触发词
- **"森林"** - 直接触发森林风格
- **"forest"** - 英文触发词
- **"自然"** - 自然风格触发
- **"绿色"** - 绿色风格触发

### 5. 通用切换触发词
- **"切换表盘"** - 切换到下一个样式
- **"换表盘"** - 切换到下一个样式
- **"下一个表盘"** - 切换到下一个样式

## 使用示例

### 基本关键词触发
```
用户输入: "动漫"
系统响应: "Detected anime keywords, switching to anime style"
系统响应: "Watch style changed to: Anime Style"

用户输入: "科技"
系统响应: "Detected tech keywords, switching to tech style"
系统响应: "Watch style changed to: Tech Style"

用户输入: "森林"
系统响应: "Detected forest keywords, switching to forest style"
系统响应: "Watch style changed to: Forest Style"
```

### 英文关键词触发
```
用户输入: "anime"
系统响应: "Detected anime keywords, switching to anime style"
系统响应: "Watch style changed to: Anime Style"

用户输入: "tech"
系统响应: "Detected tech keywords, switching to tech style"
系统响应: "Watch style changed to: Tech Style"
```

### 描述性关键词触发
```
用户输入: "可爱"
系统响应: "Detected anime keywords, switching to anime style"
系统响应: "Watch style changed to: Anime Style"

用户输入: "优雅"
系统响应: "Detected fashion keywords, switching to fashion style"
系统响应: "Watch style changed to: Fashion Style"

用户输入: "未来"
系统响应: "Detected tech keywords, switching to tech style"
系统响应: "Watch style changed to: Tech Style"

用户输入: "自然"
系统响应: "Detected forest keywords, switching to forest style"
系统响应: "Watch style changed to: Forest Style"
```

### 通用切换触发
```
用户输入: "切换表盘"
系统响应: "Detected style change keywords, switching to next style"
系统响应: "Watch style changed to: Fashion Style"

用户输入: "换表盘"
系统响应: "Detected style change keywords, switching to next style"
系统响应: "Watch style changed to: Tech Style"
```

## 技术实现

### 关键词检测逻辑
```c
/* Check for anime style keywords */
if (strstr(text, "动漫") != NULL || strstr(text, "anime") != NULL || 
    strstr(text, "卡通") != NULL || strstr(text, "可爱") != NULL) {
    printf("Detected anime keywords, switching to anime style\n");
    switch_to_style(0); // Switch to anime style
    printf("Watch style changed to: %s\n", get_current_style_name());
    return;
}
```

### 优先级处理
1. **关键词检测** - 最高优先级
2. **历史事件检测** - 次优先级
3. **其他功能** - 最低优先级

### 即时切换
- 检测到关键词后立即切换表盘样式
- 切换完成后直接返回，不进行后续处理
- 提供清晰的日志输出

## 优势

### 1. 简单直观
- 用户只需说出关键词即可切换
- 无需复杂的语音命令
- 支持中英文关键词

### 2. 响应迅速
- 关键词检测优先级最高
- 即时切换，无需等待
- 清晰的反馈信息

### 3. 灵活多样
- 支持多种触发词
- 支持描述性关键词
- 支持通用切换命令

### 4. 易于扩展
- 可以轻松添加新的关键词
- 可以添加新的表盘样式
- 代码结构清晰

## 使用场景

### 场景1: 快速切换
```
用户: "动漫"
系统: 立即切换到动漫风格表盘
```

### 场景2: 描述性切换
```
用户: "我想要一个可爱的表盘"
系统: 检测到"可爱"关键词，切换到动漫风格
```

### 场景3: 英文切换
```
用户: "tech"
系统: 检测到"tech"关键词，切换到科技风格
```

### 场景4: 循环切换
```
用户: "切换表盘"
系统: 切换到下一个样式（动漫→时尚→科技→森林→动漫）
```

## 注意事项

1. **关键词匹配**: 使用`strstr`函数进行子字符串匹配
2. **优先级**: 关键词检测优先于历史事件检测
3. **即时返回**: 检测到关键词后立即返回，不进行后续处理
4. **日志记录**: 所有切换操作都有详细的日志记录

## 总结

通过关键词触发的方式，用户可以：
- 快速切换表盘样式
- 使用简单直观的关键词
- 享受即时响应的体验
- 支持多种语言和描述方式

这种实现方式让表盘切换变得更加简单和直观，用户只需要说出相关的关键词就能立即切换到对应的表盘样式。
