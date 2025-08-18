# DIY Otto Robot 指南
## 项目概述
Otto Robot 是一个开源的人形机器人平台，支持多种功能扩展。本指南将帮助您快速搭建和配置属于自己的 Otto 机器人，并通过涂鸦智能APP实现远程控制。

## 演示视频
扫码观看：

![](https://camo.githubusercontent.com/2b3caf7a7f468ee64fc0ff4234fe0b47a557cd6a19f637a8ef86215b18397104/68747470733a2f2f696d616765732e74757961636e2e636f6d2f66652d7374617469632f646f63732f696d672f30633532643638362d346136302d343365352d613763322d3339613937376465623230342e706e67)

## 一、材料清单
以下是制作 Otto Robot 所需的硬件材料：

1. **外壳**
    - 型号：Otto Robot 3D打印机体外壳  
    - 获取途径1：电商平台请人帮忙代打印~
    - 获取途径2：网上开源项目自己使用3D打印机打印~

- [otto 外壳（博主小鹏分享）](https://makerworld.com.cn/zh/models/1117966-ottorobot-xiao-zhi-ai#profileId-1284462)  
  ![otto 外壳（小鹏分享）图片](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755227094837-7c4555e2-dfe3-493c-a52b-306f3d32a165.png)  

- [otto emo外壳（博主绿荫阿广分享）](https://gitee.com/maker-community/VerdureLab)   
  ![otto emo外壳图片](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755226786529-24912c2f-fdd0-49c8-86c5-5904d526a168.png)  

- [otto 小黄鸭外壳（博主绿荫阿广分享）](https://github.com/maker-community/VerdureLab/tree/main/verdure-duck)  
  ![otto 小黄鸭外壳图片](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755226645805-e9dad500-6e9b-45b9-8ac0-c9e82881a7b0.png)  


+ **舵机**
    - 型号：SG90 180度舵机（不带手臂是4个，带手臂6个）  
    - 购买渠道：淘宝
4. **显示屏**
    - 型号：ST7789 / Otto 鸭子外壳 双眼用的0.96寸160*80 ST7735S屏幕  
    - 购买渠道：淘宝
5. **开发板**
    - 型号：T5 otto 开发板  
    - 购买方式：淘宝/联系涂鸦AI开发者群负责人
    - 嘉立创上基于otto T5开源项目，自己打板:
    - 项目1：[https://oshwhub.com/dream000/ni-hao-tu-ya-otto-ji-qi-ren](https://oshwhub.com/dream000/ni-hao-tu-ya-otto-ji-qi-ren)
    - 项目2：[https://oshwhub.com/txp666/ottorobot](https://oshwhub.com/txp666/ottorobot)



## 二、硬件接线图
| 硬件设备 | 外设 | T5引脚 | 引脚功能 |
| --- | --- | --- | --- |
| 屏幕 | SCL | P14 | SPI0时钟 |
|  | CS | P13 | SPI0片选 |
|  | SDA | P16 | SPI0数据 |
|  | RST | P19 | 屏幕复位 |
|  | DC | P17 | 数据/命令选择 |
|  | BLK | P5 | 屏幕背光 |
| 舵机 | PWM0 | P18 | 左腿舵机 |
|  | PWM1 | P24 | 右腿舵机 |
|  | PWM2 | P32 | 左脚舵机 |
|  | PWM3 | P34 | 右脚舵机 |
|  | PWM4 | P36 | 左手舵机 |
|  | PWM5 | P9 | 右手舵机 |


## 三、组装教程
[Otto Robot 组装教程](https://www.bilibili.com/video/BV1dyjhzzExr?share_source=copy_web) （建议看博主详细教程）

[T5小黄鸭装配教程](https://xhslink.com/m/AcpWKIV1SEm )（T5版本接线使用）



## 四、TuyaOpen文档开发教程（重要）
[涂鸦 TuyaOpen 官方文档](https://tuyaopen.ai/zh/docs/about-tuyaopen)

## 五、代码下载修改编译
### 1. 代码下载
#### 步骤1：安装 Git
根据您的操作系统选择对应的安装方式：

+ **Windows 系统**
    1. 访问 Git 官网（[https://git-scm.com/），点击下载适合](https://git-scm.com/），点击下载适合) Windows 的版本
    2. 运行安装程序，按向导操作
+ **Linux 系统**

```bash
sudo apt update
sudo apt install git
```

+ **macOS 系统**（使用 Homebrew，推荐）
    1. 若未安装 Homebrew，先执行：

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

    2. 安装 Git：

```bash
brew install git
```

#### 步骤2：克隆代码仓库
+ GitHub TuyaOpen 主仓库：[https://github.com/tuya/TuyaOpen](https://github.com/tuya/TuyaOpen)  
+ 本次制作机器人使用仓库中的：[https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_otto_robot](https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_otto_robot)

```bash
git clone https://github.com/tuya/TuyaOpen.git
```

### 2. 配置环境
官方文档：[TuyaOpen 环境准备](https://tuyaopen.ai/zh/docs/quick-start/enviroment-setup#%E7%8E%AF%E5%A2%83%E5%87%86%E5%A4%87)

先配置好环境后，激活 `tos.py` 编译工具：

```bash
cd TuyaOpen
```

根据操作系统选择激活方式：

+ **Linux 系统**

```bash
. ./export.sh
```

+ **macOS 系统**

```bash
. ./export.sh
```

+ **Windows 系统**

```bash
.\export.ps1  # PowerShell（需先执行 `Set-ExecutionPolicy RemoteSigned -Scope LocalMachine`）
# 或
.\export.bat  # CMD
```

### 3. 产品ID（PID）& 授权码修改
+ **PID 修改**： 进入cd apps/tuya.ai/your_otto_robot/ 目录

     执行命令：tos.py config menu

![](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755221489825-d4c223ea-22ea-4102-a267-27dd882b64f0.png)

![](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755221499157-f6250006-ef91-46b1-ab66-3444cd8ab7bd.png)

修改完按S 保存，Q 退出界面

+ 在文件 `apps/tuya.ai/your_otto_robot/include/tuya_config.h` 中，将 PID 确认是否修改为 `<font style="color:#DF2A3F;">jxcogc0qofqaplvo</font>`（也可以使用自己创建 ）
+ **UUID 获取**：访问 Tuya Open 仓库，点击右上角 "Star" 后进群获取 UUID

![](https://cdn.nlark.com/yuque/0/2025/png/55332580/1747998833234-310a2deb-5b01-4ebe-8e85-0b58f3b568f0.png)

+ 在 `apps/tuya.ai/your_otto_robot/include/tuya_config.h` 中把 **<font style="color:#DF2A3F;">UUID 改成您申请到的 UUID （非常重要，否则无法激活配网）</font>**

![](https://cdn.nlark.com/yuque/0/2025/png/55332580/1755219992004-b4a2535c-04d1-403c-a189-9ddc77cff7a9.png)  
_（注：如果您的 T5 模组下单时已烧录 TuyaOpen 的授权码，则无需填写 UUID 和 AUTHKEY）_

### 4. 编译代码，生成固件
参考文档：[TuyaOpen 官方文档](https://tuyaopen.ai/zh/docs/about-tuyaopen)

1. 进入对应的 `your_otto_robot` 目录：

```bash
cd apps/tuya.ai/your_otto_robot/
```

2. 选择配置：

```bash
tos.py config choice 
```

选择 `1` 即可。  
_（若出现 _`tos.py: command not found`_ 报错，说明未成功激活 _`tos.py`_ 编译工具，请检查上述激活步骤）_

3. 编译命令：

```bash
tos.py build
```

## 六、固件烧录指南
官方文档：[TuyaOpen 固件烧录](https://tuyaopen.ai/zh/docs/quick-start/firmware-burning)



## 七、控制效果确认
### 1. AI 运动控制
1. 下载涂鸦智能APP
2. 在APP右上角添加子设备，选择 "机器人"
3. 进入控制界面，即可通过APP控制机器人实现：
    - 左右移动
    - 前后移动
4. 使用语音控制 Otto 机器人前后左右移动（唤醒词："你好，涂鸦"等）

### 2. AI 聊天
使用语音唤醒聊天（唤醒词："你好，涂鸦"，hey,tuya“）

### 3. 功能清单
+ 支持基本行走动作
+ 支持语音指令控制
+ 屏幕显示状态信息
+ 支持视频识别（未来规划）

## 八、资源支持
+ **技术交流**：加入涂鸦AI开发QQ群/微信群 获取技术支持

![](https://cdn.nlark.com/yuque/0/2025/png/55332580/1747998833234-310a2deb-5b01-4ebe-8e85-0b58f3b568f0.png)

![](https://cdn.nlark.com/yuque/0/2025/jpeg/55332580/1747998771203-5ac06211-d6ce-424d-99f9-b431804ebc80.jpeg?x-oss-process=image%2Fformat%2Cwebp)

+ **社区分享**：欢迎在 GitHub 或涂鸦开发者社区分享您的项目心得

祝您成功打造属于自己的智能 Otto 机器人！

## 九、致谢
本项目感谢以下开源作者的支持：

1. [txp666]

本项目感谢以下开源项目的支持：

1. OttoDIYLib

本项目感谢以下开源社区的支持：

1. JLCEDA

