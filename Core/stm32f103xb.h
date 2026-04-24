#ifndef CORE_STM32F103XB_H
#define CORE_STM32F103XB_H

#include <stdint.h>

/*
 * 这是本工程自带的 STM32F103C8T6 最小寄存器头文件。
 * 目的：不依赖 HAL/SPL，Keil 安装好后即可打开工程编译。
 * 只定义本工程用到的外设和位，不追求覆盖整颗芯片。
 */

#define __IO volatile

/* RCC 时钟控制寄存器。 */
typedef struct {
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t APB2RSTR;
    __IO uint32_t APB1RSTR;
    __IO uint32_t AHBENR;
    __IO uint32_t APB2ENR;
    __IO uint32_t APB1ENR;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
} RCC_TypeDef;

/* STM32F1 GPIO 寄存器：CRL/CRH 决定每个引脚模式。 */
typedef struct {
    __IO uint32_t CRL;
    __IO uint32_t CRH;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t BRR;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

/* AFIO 用于 EXTI 端口映射和调试口/复用功能配置。 */
typedef struct {
    __IO uint32_t EVCR;
    __IO uint32_t MAPR;
    __IO uint32_t EXTICR[4];
    __IO uint32_t MAPR2;
} AFIO_TypeDef;

/* EXTI 外部中断，用于 PA4/PA5 软件编码器。 */
typedef struct {
    __IO uint32_t IMR;
    __IO uint32_t EMR;
    __IO uint32_t RTSR;
    __IO uint32_t FTSR;
    __IO uint32_t SWIER;
    __IO uint32_t PR;
} EXTI_TypeDef;

/* TIM1-TIM4 共用的基础寄存器布局，本工程使用 PWM 和编码器模式。 */
typedef struct {
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
    __IO uint32_t RCR;
    __IO uint32_t CCR1;
    __IO uint32_t CCR2;
    __IO uint32_t CCR3;
    __IO uint32_t CCR4;
    __IO uint32_t BDTR;
    __IO uint32_t DCR;
    __IO uint32_t DMAR;
} TIM_TypeDef;

/* 外设基地址，来自 STM32F103 参考手册存储器映射。 */
#define PERIPH_BASE               0x40000000UL
#define APB1PERIPH_BASE           PERIPH_BASE
#define APB2PERIPH_BASE           (PERIPH_BASE + 0x10000UL)
#define AHBPERIPH_BASE            (PERIPH_BASE + 0x20000UL)

#define TIM2_BASE                 (APB1PERIPH_BASE + 0x0000UL)
#define TIM3_BASE                 (APB1PERIPH_BASE + 0x0400UL)
#define TIM4_BASE                 (APB1PERIPH_BASE + 0x0800UL)
#define GPIOA_BASE                (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE                (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE                (APB2PERIPH_BASE + 0x1000UL)
#define AFIO_BASE                 (APB2PERIPH_BASE + 0x0000UL)
#define EXTI_BASE                 (APB2PERIPH_BASE + 0x0400UL)
#define TIM1_BASE                 (APB2PERIPH_BASE + 0x2C00UL)
#define RCC_BASE                  (AHBPERIPH_BASE + 0x1000UL)

#define TIM1                      ((TIM_TypeDef *)TIM1_BASE)
#define TIM2                      ((TIM_TypeDef *)TIM2_BASE)
#define TIM3                      ((TIM_TypeDef *)TIM3_BASE)
#define TIM4                      ((TIM_TypeDef *)TIM4_BASE)
#define GPIOA                     ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                     ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                     ((GPIO_TypeDef *)GPIOC_BASE)
#define AFIO                      ((AFIO_TypeDef *)AFIO_BASE)
#define EXTI                      ((EXTI_TypeDef *)EXTI_BASE)
#define RCC                       ((RCC_TypeDef *)RCC_BASE)

#define FLASH_ACR                 (*((__IO uint32_t *)0x40022000UL))
#define SYST_CSR                  (*((__IO uint32_t *)0xE000E010UL))
#define SYST_RVR                  (*((__IO uint32_t *)0xE000E014UL))
#define SYST_CVR                  (*((__IO uint32_t *)0xE000E018UL))
#define NVIC_ISER0                (*((__IO uint32_t *)0xE000E100UL))
#define NVIC_ISER1                (*((__IO uint32_t *)0xE000E104UL))

/* RCC 位定义。 */
#define RCC_CR_HSEON              (1UL << 16)
#define RCC_CR_HSERDY             (1UL << 17)
#define RCC_CR_PLLON              (1UL << 24)
#define RCC_CR_PLLRDY             (1UL << 25)
#define RCC_CFGR_SW_PLL           (2UL << 0)
#define RCC_CFGR_SWS_PLL          (2UL << 2)
#define RCC_CFGR_HPRE_DIV1        (0UL << 4)
#define RCC_CFGR_PPRE1_DIV2       (4UL << 8)
#define RCC_CFGR_PPRE2_DIV1       (0UL << 11)
#define RCC_CFGR_PLLSRC_HSE       (1UL << 16)
#define RCC_CFGR_PLLMULL9         (7UL << 18)

#define RCC_APB2ENR_AFIOEN        (1UL << 0)
#define RCC_APB2ENR_IOPAEN        (1UL << 2)
#define RCC_APB2ENR_IOPBEN        (1UL << 3)
#define RCC_APB2ENR_IOPCEN        (1UL << 4)
#define RCC_APB2ENR_TIM1EN        (1UL << 11)
#define RCC_APB1ENR_TIM2EN        (1UL << 0)
#define RCC_APB1ENR_TIM3EN        (1UL << 1)
#define RCC_APB1ENR_TIM4EN        (1UL << 2)

/* GPIO MODE/CNF 组合值，供 gpio_config() 使用。 */
#define GPIO_MODE_INPUT           0x0U
#define GPIO_MODE_OUTPUT_2MHZ     0x2U
#define GPIO_MODE_OUTPUT_50MHZ    0x3U
#define GPIO_CNF_IN_ANALOG        0x0U
#define GPIO_CNF_IN_FLOATING      0x1U
#define GPIO_CNF_IN_PULL          0x2U
#define GPIO_CNF_OUT_PP           0x0U
#define GPIO_CNF_OUT_OD           0x1U
#define GPIO_CNF_AF_PP            0x2U

/* TIM 常用控制位。 */
#define TIM_CR1_CEN               (1UL << 0)
#define TIM_CR1_ARPE              (1UL << 7)
#define TIM_EGR_UG                (1UL << 0)
#define TIM_CCER_CC1E             (1UL << 0)
#define TIM_CCER_CC2E             (1UL << 4)
#define TIM_CCER_CC3E             (1UL << 8)
#define TIM_CCER_CC4E             (1UL << 12)
#define TIM_BDTR_MOE              (1UL << 15)

#define EXTI4_IRQn                10
#define EXTI9_5_IRQn              23

/* 启动文件和系统时钟文件使用的 CMSIS 风格接口。 */
void SystemInit(void);
extern uint32_t SystemCoreClock;

#endif
