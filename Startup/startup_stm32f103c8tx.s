                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __initial_sp
                IMPORT  SystemInit
                IMPORT  main

; 栈大小 1KB。当前工程没有 RTOS 和大数组，足够启动与控制循环使用。
Stack_Size      EQU     0x00000400
                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

; 中断向量表。只显式放入本工程使用的 SysTick、EXTI4、EXTI9_5。
                AREA    RESET, DATA, READONLY
__Vectors
                DCD     __initial_sp
                DCD     Reset_Handler
                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     0, 0, 0, 0, 0, 0, 0
                DCD     SVC_Handler
                DCD     0, 0
                DCD     PendSV_Handler
                DCD     SysTick_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     EXTI4_IRQHandler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     EXTI9_5_IRQHandler

                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler
; 上电后先配置系统时钟，再进入 C 语言 main()。
                BL      SystemInit
                BL      main
                B       .
                ENDP

Default_Handler PROC
                EXPORT  Default_Handler
                B       .
                ENDP

NMI_Handler     PROC
                EXPORT  NMI_Handler             [WEAK]
                B       Default_Handler
                ENDP

HardFault_Handler PROC
                EXPORT  HardFault_Handler       [WEAK]
                B       Default_Handler
                ENDP

SVC_Handler     PROC
                EXPORT  SVC_Handler             [WEAK]
                B       Default_Handler
                ENDP

PendSV_Handler  PROC
                EXPORT  PendSV_Handler          [WEAK]
                B       Default_Handler
                ENDP

SysTick_Handler PROC
                EXPORT  SysTick_Handler         [WEAK]
                B       Default_Handler
                ENDP

EXTI4_IRQHandler PROC
                EXPORT  EXTI4_IRQHandler        [WEAK]
                B       Default_Handler
                ENDP

EXTI9_5_IRQHandler PROC
                EXPORT  EXTI9_5_IRQHandler      [WEAK]
                B       Default_Handler
                ENDP

                END
