# 编译说明

## 文件依赖关系

```
platform/platform_interface.h (接口定义)
    ↑
core/servo_control.c (核心实现)
    ↑
otto_ninja/otto_ninja_app_servo.c (OttoNinja实现)
    ↑
examples/*.c (示例代码)
```

## 编译步骤

### 方法1: 手动编译

```bash
# 1. 编译核心库
gcc -c core/servo_control.c -I. -Icore -Iplatform -o servo_control.o

# 2. 编译OttoNinja实现（可选）
gcc -c otto_ninja/otto_ninja_app_servo.c -I. -Icore -Iotto_ninja -Iplatform -o otto_ninja_app_servo.o

# 3. 编译平台实现（根据你的平台选择）
gcc -c platform/platform_stm32.c -I. -Iplatform -o platform_stm32.o
# 或
gcc -c platform/platform_esp32.c -I. -Iplatform -o platform_esp32.o

# 4. 编译示例程序
gcc examples/otto_ninja_app_example.c \
    servo_control.o \
    otto_ninja_app_servo.o \
    platform_stm32.o \
    -I. -Icore -Iotto_ninja -Iplatform \
    -o otto_ninja_app

# 5. 链接
# 根据你的平台添加相应的库
```

### 方法2: 使用Makefile

创建 `Makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
INCLUDES = -I. -Icore -Iotto_ninja -Iplatform

# 源文件
CORE_SRC = core/servo_control.c
OTTO_SRC = otto_ninja/otto_ninja_app_servo.c
PLATFORM_SRC = platform/platform_stm32.c  # 根据你的平台修改
EXAMPLE_SRC = examples/otto_ninja_app_example.c

# 目标文件
CORE_OBJ = $(CORE_SRC:.c=.o)
OTTO_OBJ = $(OTTO_SRC:.c=.o)
PLATFORM_OBJ = $(PLATFORM_SRC:.c=.o)

# 最终目标
TARGET = otto_ninja_app

all: $(TARGET)

$(TARGET): $(CORE_OBJ) $(OTTO_OBJ) $(PLATFORM_OBJ) $(EXAMPLE_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(CORE_OBJ) $(OTTO_OBJ) $(PLATFORM_OBJ) $(TARGET)

.PHONY: all clean
```

### 方法3: 集成到现有项目

#### STM32 (使用STM32CubeIDE)

1. 将 `c_port` 目录复制到你的STM32项目
2. 在项目设置中添加包含路径：
   - `c_port/core`
   - `c_port/otto_ninja`
   - `c_port/platform`
3. 添加源文件到编译：
   - `c_port/core/servo_control.c`
   - `c_port/otto_ninja/otto_ninja_app_servo.c`
   - `c_port/platform/platform_stm32.c` (你的实现)
4. 在代码中包含头文件：
   ```c
   #include "servo_control.h"
   #include "otto_ninja_app_servo.h"
   ```

#### ESP32 (使用ESP-IDF)

1. 将 `c_port` 目录复制到 `components` 目录
2. 创建 `components/c_port/CMakeLists.txt`:
   ```cmake
   idf_component_register(
       SRCS 
           "core/servo_control.c"
           "otto_ninja/otto_ninja_app_servo.c"
           "platform/platform_esp32.c"
       INCLUDE_DIRS 
           "core"
           "otto_ninja"
           "platform"
   )
   ```
3. 在 `main/CMakeLists.txt` 中添加：
   ```cmake
   target_link_libraries(${COMPONENT_LIB} PRIVATE c_port)
   ```

## 平台特定配置

### STM32

```c
// 定义平台宏
#define PLATFORM_STM32

// 包含HAL库
#include "stm32f4xx_hal.h"

// 实现 platform_interface.h 中的函数
```

### ESP32

```c
// 定义平台宏
#define PLATFORM_ESP32

// 包含ESP-IDF头文件
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"

// 实现 platform_interface.h 中的函数
```

### Linux

```c
// 定义平台宏
#define PLATFORM_LINUX

// 包含系统头文件
#include <sys/time.h>
#include <unistd.h>

// 实现 platform_interface.h 中的函数
```

## 常见问题

### 1. 找不到头文件

**问题**: `fatal error: servo_control.h: No such file or directory`

**解决**: 确保在编译时添加了正确的包含路径：
```bash
-Ic_port/core -Ic_port/otto_ninja -Ic_port/platform
```

### 2. 未定义的平台函数

**问题**: `undefined reference to 'get_millis'`

**解决**: 确保实现了 `platform/platform_interface.h` 中定义的所有函数，并链接了平台实现文件。

### 3. PWM不工作

**问题**: 舵机不响应

**解决**: 
- 检查PWM频率是否为50Hz
- 检查脉冲宽度范围是否正确（544-2400微秒）
- 检查GPIO引脚配置是否正确
- 使用示波器检查PWM信号

### 4. 时间控制不准确

**问题**: 行走动作不协调

**解决**:
- 确保 `get_millis()` 返回准确的时间值
- 检查系统时钟配置
- 避免在中断中调用时间相关函数

## 调试建议

1. **启用调试输出**: 在关键函数中添加printf输出
2. **使用逻辑分析仪**: 检查PWM信号是否正确
3. **单步调试**: 逐步执行代码，检查变量值
4. **单元测试**: 单独测试每个函数

## 性能优化

1. **减少延时**: 优化不必要的delay调用
2. **PWM硬件加速**: 使用硬件PWM而非软件模拟
3. **中断优化**: 将PWM更新放在中断中处理
4. **内存优化**: 减少全局变量使用

