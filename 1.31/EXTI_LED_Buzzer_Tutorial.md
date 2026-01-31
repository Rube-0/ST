# STM32 EXTI 外部中断控制 LED 和蜂鸣器 教程

## 📚 目录
1. [项目概述](#项目概述)
2. [硬件配置](#硬件配置)
3. [EXTI 工作原理](#exti-工作原理)
4. [代码实现步骤](#代码实现步骤)
5. [完整代码](#完整代码)
6. [测试方法](#测试方法)
7. [常见问题](#常见问题)

---

## 项目概述

本教程教你如何使用 STM32F103 的 **EXTI（外部中断）** 功能，通过按键控制 LED 和蜂鸣器。

### 功能描述
| 按键 | 引脚 | 功能 |
|------|------|------|
| KEY1 | PE3 | 控制 LED0 翻转 |
| KEY2 | PE4 | 控制 LED1 翻转 |
| KEY_UP | PA0 | 控制蜂鸣器翻转 |

---

## 硬件配置

### 引脚分配表

| 引脚 | 功能 | 模式 | 说明 |
|------|------|------|------|
| PE3 | KEY1 | 外部中断（下降沿） | 上拉输入，按下为低电平 |
| PE4 | KEY2 | 外部中断（下降沿） | 上拉输入，按下为低电平 |
| PA0 | KEY_UP | 外部中断（上升沿） | 下拉输入，按下为高电平 |
| PE5 | LED0 | 推挽输出 | 低电平点亮 |
| PB5 | LED1 | 推挽输出 | 低电平点亮 |
| PB8 | BUZZER | 推挽输出 | 高电平响 |

### 中断优先级配置

```c
HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);  // KEY2 - 最高优先级
HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);  // KEY1 - 次高优先级
HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);  // KEY_UP - 较低优先级
```

---

## EXTI 工作原理

### 什么是 EXTI？

**EXTI（External Interrupt）外部中断** 是 STM32 的一种中断机制，当 GPIO 引脚检测到电平变化时，会触发中断，CPU 暂停当前任务去执行中断服务程序。

### 中断触发模式

| 模式 | 宏定义 | 说明 |
|------|--------|------|
| 上升沿触发 | `GPIO_MODE_IT_RISING` | 电平从低→高时触发 |
| 下降沿触发 | `GPIO_MODE_IT_FALLING` | 电平从高→低时触发 |
| 双边沿触发 | `GPIO_MODE_IT_RISING_FALLING` | 任何电平变化都触发 |

### 工作流程图

```
┌─────────────┐
│  按键按下    │
└──────┬──────┘
       ▼
┌─────────────────────┐
│  GPIO检测电平变化    │
│  (下降沿/上升沿)     │
└──────┬──────────────┘
       ▼
┌─────────────────────┐
│  触发 EXTI 中断      │
│  EXTIx_IRQHandler() │
└──────┬──────────────┘
       ▼
┌─────────────────────┐
│  HAL_GPIO_EXTI_     │
│  IRQHandler()       │
└──────┬──────────────┘
       ▼
┌─────────────────────────────┐
│  HAL_GPIO_EXTI_Callback()   │  ← 用户编写的回调函数
│  根据 GPIO_Pin 判断按键     │
│  执行 LED/Buzzer 控制       │
└─────────────────────────────┘
```

---

## 代码实现步骤

### 第一步：在 main.h 中添加宏定义

打开 `Core/Inc/main.h`，在 `/* USER CODE BEGIN Private defines */` 区域添加：

```c
/* USER CODE BEGIN Private defines */
// ========== LED 引脚定义 ==========
#define LED0_PIN        GPIO_PIN_5
#define LED0_GPIO_PORT  GPIOE

#define LED1_PIN        GPIO_PIN_5
#define LED1_GPIO_PORT  GPIOB

// ========== 蜂鸣器引脚定义 ==========
#define BUZZER_PIN        GPIO_PIN_8
#define BUZZER_GPIO_PORT  GPIOB

// ========== 按键引脚定义 ==========
#define KEY1_PIN        GPIO_PIN_3
#define KEY1_GPIO_PORT  GPIOE

#define KEY2_PIN        GPIO_PIN_4
#define KEY2_GPIO_PORT  GPIOE

#define KEY_UP_PIN      GPIO_PIN_0
#define KEY_UP_GPIO_PORT GPIOA

// ========== LED 控制宏 ==========
#define LED0_ON()       HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_RESET)
#define LED0_OFF()      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_SET)
#define LED0_TOGGLE()   HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN)

#define LED1_ON()       HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF()      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_TOGGLE()   HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN)

// ========== 蜂鸣器控制宏 ==========
#define BUZZER_ON()     HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET)
#define BUZZER_OFF()    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET)
#define BUZZER_TOGGLE() HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_PIN)

/* USER CODE END Private defines */
```

### 第二步：在 main.c 中编写中断回调函数

#### 🔑 关于 `HAL_GPIO_EXTI_Callback` 的说明

**Q: 这个函数需要先声明吗？**

**A: 不需要！** 这是 HAL 库中的 **弱定义函数（weak function）**。

在 HAL 库源码 `stm32f1xx_hal_gpio.c` 中已经有默认实现：

```c
__weak void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  UNUSED(GPIO_Pin);
  /* 用户应在自己的文件中重新实现此函数 */
}
```

**`__weak` 关键字的作用：**
- 如果用户没有定义这个函数 → 使用 HAL 库中的空实现
- 如果用户定义了同名函数 → 编译器自动使用用户的版本（覆盖弱定义）
- 函数原型已在 `stm32f1xx_hal_gpio.h` 中声明，无需重复声明

**中断调用链路：**
```
按键中断触发
    ↓
EXTI3_IRQHandler()           ← stm32f1xx_it.c 中的中断入口
    ↓
HAL_GPIO_EXTI_IRQHandler()   ← HAL库函数，清除中断标志
    ↓
HAL_GPIO_EXTI_Callback()     ← 你重写的函数会被自动调用！
```

打开 `Core/Src/main.c`，在 `/* USER CODE BEGIN 0 */` 区域添加：

```c
/* USER CODE BEGIN 0 */

/**
  * @brief  EXTI 中断回调函数
  * @param  GPIO_Pin: 触发中断的引脚
  * @note   当按键按下时，此函数被调用
  *         - KEY1 (PE3): 控制 LED0 翻转
  *         - KEY2 (PE4): 控制 LED1 翻转
  *         - KEY_UP (PA0): 控制蜂鸣器翻转
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // 简单的软件消抖延时
    HAL_Delay(10);
    
    switch(GPIO_Pin)
    {
        case KEY1_PIN:  // PE3 - KEY1 按下
            // 再次确认按键状态（消抖）
            if(HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN) == GPIO_PIN_RESET)
            {
                LED0_TOGGLE();  // LED0 翻转
            }
            break;
            
        case KEY2_PIN:  // PE4 - KEY2 按下
            if(HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_PIN) == GPIO_PIN_RESET)
            {
                LED1_TOGGLE();  // LED1 翻转
            }
            break;
            
        case KEY_UP_PIN:  // PA0 - KEY_UP 按下
            if(HAL_GPIO_ReadPin(KEY_UP_GPIO_PORT, KEY_UP_PIN) == GPIO_PIN_SET)
            {
                BUZZER_TOGGLE();  // 蜂鸣器翻转
            }
            break;
            
        default:
            break;
    }
}

/* USER CODE END 0 */
```

### 第三步：在 main() 函数中初始化外设状态

在 `/* USER CODE BEGIN 2 */` 区域添加：

```c
/* USER CODE BEGIN 2 */

// 初始化：关闭所有 LED 和蜂鸣器
LED0_OFF();
LED1_OFF();
BUZZER_OFF();

/* USER CODE END 2 */
```

---

## 完整代码

### main.h 完整代码

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* USER CODE BEGIN Private defines */
// ========== LED 引脚定义 ==========
#define LED0_PIN        GPIO_PIN_5
#define LED0_GPIO_PORT  GPIOE

#define LED1_PIN        GPIO_PIN_5
#define LED1_GPIO_PORT  GPIOB

// ========== 蜂鸣器引脚定义 ==========
#define BUZZER_PIN        GPIO_PIN_8
#define BUZZER_GPIO_PORT  GPIOB

// ========== 按键引脚定义 ==========
#define KEY1_PIN        GPIO_PIN_3
#define KEY1_GPIO_PORT  GPIOE

#define KEY2_PIN        GPIO_PIN_4
#define KEY2_GPIO_PORT  GPIOE

#define KEY_UP_PIN      GPIO_PIN_0
#define KEY_UP_GPIO_PORT GPIOA

// ========== LED 控制宏 ==========
#define LED0_ON()       HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_RESET)
#define LED0_OFF()      HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, GPIO_PIN_SET)
#define LED0_TOGGLE()   HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN)

#define LED1_ON()       HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF()      HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_TOGGLE()   HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN)

// ========== 蜂鸣器控制宏 ==========
#define BUZZER_ON()     HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET)
#define BUZZER_OFF()    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET)
#define BUZZER_TOGGLE() HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_PIN)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
```

### main.c 关键代码

```c
/* USER CODE BEGIN 0 */

/**
  * @brief  EXTI 中断回调函数
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HAL_Delay(10);  // 消抖
    
    switch(GPIO_Pin)
    {
        case KEY1_PIN:
            if(HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN) == GPIO_PIN_RESET)
            {
                LED0_TOGGLE();
            }
            break;
            
        case KEY2_PIN:
            if(HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_PIN) == GPIO_PIN_RESET)
            {
                LED1_TOGGLE();
            }
            break;
            
        case KEY_UP_PIN:
            if(HAL_GPIO_ReadPin(KEY_UP_GPIO_PORT, KEY_UP_PIN) == GPIO_PIN_SET)
            {
                BUZZER_TOGGLE();
            }
            break;
            
        default:
            break;
    }
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    /* USER CODE BEGIN 2 */
    LED0_OFF();
    LED1_OFF();
    BUZZER_OFF();
    /* USER CODE END 2 */

    while (1)
    {
        // 主循环保持空闲，所有工作由中断完成
    }
}
```

### gpio.c 配置代码（由 STM32CubeMX 生成）

```c
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5|GPIO_PIN_8, GPIO_PIN_RESET);

    /* Configure GPIO pins : PE3 PE4 (KEY1, KEY2) */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // 下降沿触发
    GPIO_InitStruct.Pull = GPIO_PULLUP;           // 上拉
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Configure GPIO pin : PE5 (LED0) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* Configure GPIO pin : PA0 (KEY_UP) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;   // 上升沿触发
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;         // 下拉
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure GPIO pins : PB5 PB8 (LED1, BUZZER) */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* EXTI interrupt init */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}
```

---

## 测试方法

1. **编译工程** - 使用 Keil/STM32CubeIDE 编译
2. **下载程序** - 通过 ST-Link 下载到开发板
3. **测试功能**：
   - 按下 **KEY1** → LED0 翻转（亮/灭）
   - 按下 **KEY2** → LED1 翻转（亮/灭）
   - 按下 **KEY_UP** → 蜂鸣器翻转（响/停）

---

## 常见问题

### Q1: 为什么 KEY1/KEY2 用下降沿，KEY_UP 用上升沿？

**A:** 这取决于硬件电路设计：
- KEY1/KEY2 连接上拉电阻，平时为高电平，按下接地变低电平 → **下降沿**
- KEY_UP 连接下拉电阻，平时为低电平，按下接 VCC 变高电平 → **上升沿**

### Q2: 为什么要做消抖处理？

**A:** 机械按键在按下/松开瞬间会产生抖动，可能在几毫秒内多次触发中断。消抖方法：
1. **硬件消抖**：加 RC 滤波电路
2. **软件消抖**：延时 + 再次读取确认

### Q3: 中断回调函数中能用 HAL_Delay() 吗？

**A:** 可以，但不推荐在中断中使用长延时。因为：
- 中断会阻塞其他中断和主循环
- 复杂操作应该设置标志位，在主循环中处理

### Q4: 如何添加更多按键？

**A:** 
1. 在 STM32CubeMX 中配置新的 GPIO 为 EXTI 模式
2. 在 `main.h` 中添加引脚定义
3. 在 `HAL_GPIO_EXTI_Callback()` 中添加新的 `case` 分支

---

## 扩展功能

### 长按检测示例

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_PIN)
    {
        uint32_t press_time = 0;
        
        // 等待按键松开，同时计时
        while(HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN) == GPIO_PIN_RESET)
        {
            HAL_Delay(10);
            press_time += 10;
            
            if(press_time > 1000)  // 长按超过1秒
            {
                // 执行长按操作
                LED0_ON();
                LED1_ON();
                return;
            }
        }
        
        // 短按操作
        LED0_TOGGLE();
    }
}
```

### 蜂鸣器播放旋律示例

```c
void Buzzer_Beep(uint16_t times, uint16_t delay_ms)
{
    for(uint16_t i = 0; i < times; i++)
    {
        BUZZER_ON();
        HAL_Delay(delay_ms);
        BUZZER_OFF();
        HAL_Delay(delay_ms);
    }
}

// 在回调中使用
case KEY_UP_PIN:
    Buzzer_Beep(3, 100);  // 响3声
    break;
```

---

## 总结

本教程介绍了：
1. ✅ EXTI 外部中断的工作原理
2. ✅ GPIO 引脚配置方法
3. ✅ HAL 库中断回调函数的编写
4. ✅ 按键消抖处理
5. ✅ LED 和蜂鸣器的控制方法

通过这个项目，你掌握了 STM32 外部中断的基本用法，可以扩展到更复杂的应用场景！
