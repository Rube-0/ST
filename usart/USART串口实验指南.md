# STM32F103 USART串口实验完整指南

## 目录
1. [实验介绍](#实验介绍)
2. [硬件准备](#硬件准备)
3. [项目配置](#项目配置)
4. [基础实验：发送固定字符](#基础实验发送固定字符)
5. [实验二：接收数据并回显](#实验二接收数据并回显)
6. [实验三：中断接收数据](#实验三中断接收数据)
7. [实验四：printf重定向](#实验四printf重定向)
8. [实验五：完整的USART通信系统](#实验五完整的usart通信系统)
9. [故障排查](#故障排查)

---

## 实验介绍

本实验基于STM32F103xe微控制器的USART1接口，通过USB-UART模块与PC端进行串行通信。

**当前配置参数：**
- USART1接口
- 波特率：115200 bps
- 数据位：8位
- 停止位：1位
- 校验位：无
- TX管脚：PA9（引脚号）
- RX管脚：PA10（引脚号）
- 中断优先级：3

---

## 硬件准备

### 1. 需要的硬件
- STM32F103开发板 × 1
- USB-UART模块（如CH340、PL2303等）× 1
- Micro USB数据线 × 1
- USB串口线或直连杜邦线

### 2. 硬件连接
```
USB-UART模块    →    STM32开发板
GND             →    GND
TX              →    PA10（USART1 RX）
RX              →    PA9（USART1 TX）
VCC             →    3.3V（可选）
```

### 3. 驱动程序安装
- 安装USB-UART模块的驱动程序
- 在设备管理器中查看COM端口号（如COM3）
- 记录COM端口号供后续使用

### 4. 串口调试工具
推荐使用以下工具之一：
- **串口助手**（Windows）
- **Putty**（跨平台）
- **CoolTerm**（跨平台）
- **VS Code SerialMonitor**插件

---

## 项目配置

### 1. 当前配置状态
项目已配置以下内容：
- ✅ USART1时钟已启用
- ✅ PA9/PA10 GPIO已配置
- ✅ UART中断已启用
- ✅ 波特率设置为115200

### 2. CMakeLists.txt配置验证
确保项目包含以下驱动库：
```cmake
# 应包含以下内容
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
```

### 3. 编译构建
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake

# 编译
cmake --build .

# 生成烧录文件
# 会生成 usart.elf, usart.hex, usart.bin
```

---

## 基础实验：发送固定字符

### 目标
在main函数中初始化USART1，发送固定字符串到PC端。

### 步骤1：编辑main.c文件

修改 `Core/Src/main.c`，在main函数中添加USART1初始化和发送代码：

```c
#include "stdio.h"
#include "string.h"

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* 准备发送的字符串 */
  uint8_t tx_data[] = "Hello USART1!\r\n";
  
  while (1)
  {
    /* 发送字符串 */
    HAL_UART_Transmit(&huart1, tx_data, strlen((char*)tx_data), 100);
    
    /* 延时1秒 */
    HAL_Delay(1000);
  }

  return 0;
}
```

### 步骤2：编译并烧录
```bash
cmake --build build
# 使用烧录工具（如ST-Link烧录器）烧录生成的.hex或.bin文件
```

### 步骤3：验证实验
1. 打开串口调试工具
2. 选择对应COM端口，设置波特率为115200
3. 打开端口连接
4. **预期结果：** 每秒显示一次 "Hello USART1!"

---

## 实验二：接收数据并回显

### 目标
接收来自PC端的数据，并将其回显到PC端。

### 步骤1：修改main.c

```c
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  uint8_t rx_data;
  uint8_t tx_msg[] = "You sent: ";
  
  while (1)
  {
    /* 接收单个字节数据 */
    if (HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK)
    {
      /* 发送提示信息 */
      HAL_UART_Transmit(&huart1, tx_msg, sizeof(tx_msg)-1, 100);
      
      /* 回显接收到的数据 */
      HAL_UART_Transmit(&huart1, &rx_data, 1, 100);
      
      /* 发送换行符 */
      uint8_t newline[] = "\r\n";
      HAL_UART_Transmit(&huart1, newline, 2, 100);
    }
  }

  return 0;
}
```

### 步骤2：测试步骤
1. 编译并烧录
2. 打开串口调试工具
3. 在调试工具中输入任意字符
4. **预期结果：** 显示 "You sent: [输入的字符]"

---

## 实验三：中断接收数据

### 目标
使用中断方式接收USART数据，避免阻塞主程序。

### 步骤1：在usart.h中添加函数声明

```c
/* USER CODE BEGIN Prototypes */

void USART1_IRQHandler(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END Prototypes */
```

### 步骤2：编辑main.c添加全局变量

```c
#include "usart.h"

/* 接收缓冲区 */
uint8_t rx_buffer[256];
uint8_t rx_index = 0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* 启动中断接收（一次接收1字节） */
  HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);

  while (1)
  {
    /* 主程序可以执行其他任务 */
    HAL_Delay(100);
  }

  return 0;
}
```

### 步骤3：添加中断回调函数

在 `Core/Src/usart.c` 中添加回调函数：

```c
/* USER CODE BEGIN 1 */

extern uint8_t rx_buffer[256];
extern uint8_t rx_index;

/* 接收完成回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART1)
  {
    /* 处理接收到的数据 */
    if(rx_buffer[rx_index] == '\r')
    {
      /* 接收到回车符，发送缓冲区中的数据 */
      rx_buffer[rx_index] = '\n';
      HAL_UART_Transmit(&huart1, rx_buffer, rx_index+1, 100);
      rx_index = 0;
    }
    else
    {
      /* 继续接收 */
      rx_index++;
      if(rx_index >= 256) rx_index = 0;
    }
    
    /* 重新启动中断接收 */
    HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1);
  }
}

/* USER CODE END 1 */
```

### 步骤4：测试
1. 编译并烧录
2. 打开串口调试工具
3. 输入一行文本，按回车键
4. **预期结果：** 接收到的文本被回显

---

## 实验四：printf重定向

### 目标
将C标准库的printf函数重定向到USART1，方便调试输出。

### 步骤1：修改syscalls.c

在 `Core/Src/syscalls.c` 中找到 `_write` 函数，修改如下：

```c
/* USER CODE BEGIN COMMON */
#include "usart.h"
/* USER CODE END COMMON */

int _write(int file, char *ptr, int len)
{
  /* USER CODE BEGIN _write */
  
  /* 重定向输出到USART1 */
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 100);
  
  return len;
  
  /* USER CODE END _write */
}
```

### 步骤2：在main.c中使用printf

```c
#include "stdio.h"
#include "usart.h"

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  printf("=== STM32F103 USART Printf Test ===\r\n");
  
  int counter = 0;
  while (1)
  {
    printf("Counter: %d\r\n", counter++);
    HAL_Delay(1000);
  }

  return 0;
}
```

### 步骤3：验证
1. 编译并烧录
2. 打开串口调试工具
3. **预期结果：** 显示递增的计数值

---

## 实验五：完整的USART通信系统

### 目标
实现一个完整的、生产级别的USART通信系统，包含：
- 命令解析
- 错误处理
- 超时机制

### 步骤1：创建USART应用层头文件

在 `Core/Inc/` 下创建 `uart_app.h`：

```c
#ifndef __UART_APP_H__
#define __UART_APP_H__

#include <stdint.h>
#include "usart.h"

#define UART_RX_BUFFER_SIZE 256
#define UART_RX_TIMEOUT 1000  /* 1 second */

typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    uint16_t length;
    uint8_t complete;
} UART_RxData_t;

/* 函数声明 */
void UART_Init(void);
void UART_Process(void);
void UART_SendString(const char *str);
void UART_SendHex(uint8_t value);

#endif
```

### 步骤2：创建USART应用层源文件

在 `Core/Src/` 下创建 `uart_app.c`：

```c
#include "uart_app.h"
#include "string.h"
#include "stdio.h"

static UART_RxData_t uart_rx_data = {0};
static uint32_t rx_last_time = 0;

void UART_Init(void)
{
    /* 启动中断接收 */
    HAL_UART_Receive_IT(&huart1, uart_rx_data.buffer, 1);
}

void UART_SendString(const char *str)
{
    if(str)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 100);
    }
}

void UART_SendHex(uint8_t value)
{
    char hex_str[3];
    sprintf(hex_str, "%02X", value);
    UART_SendString(hex_str);
}

void UART_Process(void)
{
    /* 检查是否接收完成或超时 */
    if(uart_rx_data.complete)
    {
        /* 处理接收到的数据 */
        UART_SendString("\r\n[ECHO] ");
        HAL_UART_Transmit(&huart1, uart_rx_data.buffer, uart_rx_data.length, 100);
        UART_SendString("\r\n");
        
        /* 重置接收状态 */
        uart_rx_data.complete = 0;
        uart_rx_data.length = 0;
        
        /* 重新启动接收 */
        HAL_UART_Receive_IT(&huart1, uart_rx_data.buffer, 1);
    }
}

/* 接收完成回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        uint8_t rx_byte = uart_rx_data.buffer[uart_rx_data.length];
        
        /* 处理接收到的字节 */
        if(rx_byte == '\r' || rx_byte == '\n')
        {
            uart_rx_data.complete = 1;
        }
        else if(uart_rx_data.length < UART_RX_BUFFER_SIZE - 1)
        {
            uart_rx_data.length++;
            HAL_UART_Receive_IT(&huart1, &uart_rx_data.buffer[uart_rx_data.length], 1);
        }
    }
}
```

### 步骤3：更新main.c

```c
#include "uart_app.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    UART_Init();
    
    printf("\r\n\r\n");
    printf("=================================\r\n");
    printf("STM32F103 USART Demo v1.0\r\n");
    printf("=================================\r\n");
    printf("Baud Rate: 115200\r\n");
    printf("Please send data...\r\n");
    printf("=================================\r\n\r\n");
    
    while (1)
    {
        UART_Process();
        HAL_Delay(10);
    }

    return 0;
}
```

### 步骤4：测试完整系统
1. 编译并烧录
2. 打开串口调试工具
3. **预期结果：** 显示初始化信息，并可以接收和回显数据

---

## 故障排查

### 问题1：无法接收数据
**症状：** 串口调试工具中没有任何输出
**排查步骤：**
1. ✓ 检查硬件连接（TX/RX是否接反）
2. ✓ 验证波特率设置为115200
3. ✓ 检查驱动程序是否正确安装
4. ✓ 确认COM端口号是否正确
5. ✓ 使用万用表验证USART1 TX脚是否有电平变化

### 问题2：接收数据乱码
**症状：** 显示的内容无法识别
**原因与解决：**
- 波特率不匹配 → 确认PC端波特率为115200
- 奇偶校验设置不对 → 确认设置为"无奇偶校验"
- 数据位/停止位不对 → 确认为8位数据、1个停止位

### 问题3：发送数据不出去
**症状：** 串口调试工具中无任何反应
**排查步骤：**
1. ✓ 检查TX脚连接（应连接到USB-UART的RX脚）
2. ✓ 验证USART1时钟已启用
3. ✓ 检查PA9的GPIO配置（应为推挽输出）
4. ✓ 使用示波器或逻辑分析仪监测PA9脚

### 问题4：中断接收不工作
**症状：** 中断方式接收没有响应
**原因与解决：**
- 中断未启用 → 检查 `HAL_NVIC_EnableIRQ(USART1_IRQn)` 是否被调用
- 中断处理函数错误 → 确保 `HAL_UART_RxCpltCallback` 被正确实现
- 重新启动接收遗漏 → 确保在回调函数中调用 `HAL_UART_Receive_IT`

### 问题5：printf输出为空
**症状：** printf没有任何输出
**原因与解决：**
- `_write` 函数未重定向 → 检查syscalls.c中的_write函数
- 标准库链接错误 → 确保项目链接了libc
- 缓冲区问题 → 在printf后添加 `fflush(stdout)`

---

## 常用代码片段

### 发送单个字节
```c
uint8_t data = 0x55;
HAL_UART_Transmit(&huart1, &data, 1, 100);
```

### 发送字符串
```c
char *str = "Hello World\r\n";
HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
```

### 接收单个字节
```c
uint8_t rx_data;
if(HAL_UART_Receive(&huart1, &rx_data, 1, 1000) == HAL_OK) {
    // 接收成功
}
```

### 接收一行数据（阻塞方式）
```c
uint8_t buffer[100];
uint8_t *p = buffer;
while(1) {
    HAL_UART_Receive(&huart1, p, 1, 100);
    if(*p == '\r') {
        *p = '\0';
        break;
    }
    p++;
}
```

---

## 参考资源

- STM32F103 参考手册
- STM32 HAL库用户手册
- USART协议规范
- 推荐串口调试工具：[CoolTerm](http://freeware.the-meiers.org/)

---

## 总结

通过本实验，您已经学会了：
1. ✓ USART1的硬件配置和初始化
2. ✓ 基本的字符发送和接收
3. ✓ 中断驱动的数据处理
4. ✓ printf函数重定向
5. ✓ 完整的通信系统设计
6. ✓ 常见问题的排查方法

现在您可以将这些知识应用到实际项目中了！

---

**文档版本：** 1.0  
**最后更新：** 2026年1月31日  
**作者：** STM32开发指南
