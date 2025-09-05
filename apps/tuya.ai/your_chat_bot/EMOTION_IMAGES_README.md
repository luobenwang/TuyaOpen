# 情绪图片显示功能

## 概述

本项目已经成功集成了情绪图片显示功能，将原来的文字情绪显示替换为更直观的图片显示。当AI助手表达不同情绪时，设备会显示对应的情绪图片而不是文字。

## 功能特性

### 支持的情绪类型
- **HAPPY** - 开心表情
- **SAD** - 悲伤表情  
- **ANGRY** - 愤怒表情
- **SURPRISE** - 惊讶表情
- **THINKING** - 思考表情
- **SLEEP** - 睡眠表情
- **LOVE** - 爱心表情

### 技术实现
- 图片格式：RGB565 (16位色彩)
- 图片尺寸：240x240像素
- 存储方式：C数组形式嵌入固件
- 显示方式：LVGL图片控件

## 文件结构

```
src/display/ui/emotion_images/
├── emotion_images.h              # 头文件声明
├── emotion_angry_image.c         # 愤怒表情图片数据
├── emotion_happy_image.c         # 开心表情图片数据
├── emotion_love_image.c          # 爱心表情图片数据
├── emotion_sad_image.c           # 悲伤表情图片数据
├── emotion_shock_image.c         # 惊讶表情图片数据
├── emotion_sleep_image.c         # 睡眠表情图片数据
└── emotion_think_image.c         # 思考表情图片数据
```

## 编译和烧录

### 1. 环境准备
确保已经激活了 `tos.py` 工具：
```bash
. ./export.sh
```

### 2. 配置项目
选择开发板配置：
```bash
tos.py config choice
# 选择对应的开发板配置
```

### 3. 编译项目
```bash
tos.py build
```

### 4. 烧录固件
```bash
tos.py flash -p /dev/cu.wchusbserial56D70348951
```

## 使用方法

### 自动触发
情绪图片会在以下情况下自动显示：
- AI助手回复时根据内容情绪自动切换
- 系统状态变化时显示对应情绪
- 用户交互时根据AI响应显示情绪

### 测试功能
可以使用提供的测试脚本验证图片显示：
```bash
python3 script/test_emotion_images.py
```

## 技术细节

### 图片转换
原始PNG图片通过 `script/png_to_rgb565.py` 脚本转换为RGB565格式的C数组：

```bash
python3 script/png_to_rgb565.py image src/display/ui/emotion_images
```

### 代码集成
- 修改了 `ui_chatbot.c` 文件，添加了图片显示功能
- 更新了 `CMakeLists.txt` 文件，包含新的图片源文件
- 保持了向后兼容性，如果图片加载失败会回退到文字显示

### 内存优化
- 图片数据经过优化，总大小约800KB
- 固件大小增加控制在合理范围内
- 如果ROM空间不足，可以减少图片数量

## 故障排除

### 图片不显示
1. 检查情绪名称是否正确（使用大写）
2. 确认图片文件已正确编译
3. 查看串口日志确认情绪事件

### 编译错误
1. 确保所有图片源文件存在
2. 检查CMakeLists.txt配置
3. 清理后重新编译：`tos.py clean -f && tos.py build`

### 固件过大
如果固件超过ROM限制：
1. 减少图片数量（删除部分情绪图片）
2. 降低图片分辨率
3. 使用更高效的图片压缩

## 扩展功能

### 添加新情绪
1. 准备新的PNG图片（240x240像素）
2. 运行转换脚本生成C文件
3. 在 `ui_chatbot.c` 中添加情绪映射
4. 重新编译和烧录

### 自定义图片
可以替换现有的情绪图片：
1. 替换 `image/` 目录中的PNG文件
2. 重新运行转换脚本
3. 重新编译和烧录

## 版本信息

- 版本：1.0.1
- 更新日期：2024年8月8日
- 支持的开发板：TUYA T5AI_Board
- 固件大小：约4.2MB（包含所有情绪图片） 