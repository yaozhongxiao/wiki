# start.S

The goal of this article is to provide a brief introduction about the GNU assemble startup file. With this article, you can understand how the processor starts.

The startup code consist of few parts below.
- Architecture and syntax
- Declaration of the Stack area
- Declaration of the Heap area
- Vector table
- Assembler code of Reset handler
- Definition of interrupt handler


## 1. Architecture and syntax
```
    .syntax     unified
    .arch       armv7e-m
```

## 2. Stack area
```
    .section    .stack
    .align      3
#ifdef __STACK_SIZE
    .equ        Stack_Size, __STACK_SIZE
#else
    .equ        Stack_Size, 0x00000400
#endif
    .globl      __StackTop
    .globl      __StackLimit
__StackLimit:
    .space      Stack_Size
    .size       __StackLimit, . - __StackLimit
__StackTop:
    .size       __StackTop, . - __StackTop
```

## 3. Heap area
```
    .section    .heap
    .align      3
#ifdef __HEAP_SIZE
    .equ        Heap_Size, __HEAP_SIZE
#else
    .equ        Heap_Size, 0x00000C00
#endif
    .globl      __HeapBase
    .globl      __HeapLimit
__HeapBase:
    .if Heap_Size
    .space      Heap_Size
    .endif
    .size       __HeapBase, . - __HeapBase
__HeapLimit:
    .size       __HeapLimit, . - __HeapLimit
```
## 4. Vector table
```
.section    .vectors
    .align      2
    .globl      __Vectors
__Vectors:
    .long       __StackTop                 /* Top of Stack */
    .long       Reset_Handler              /* Reset Handler */
    .long       NMI_Handler                /* NMI Handler */
    .long       HardFault_Handler          /* Hard Fault Handler */
    .long       MemManage_Handler          /* MPU Fault Handler */
    .long       BusFault_Handler           /* Bus Fault Handler */
    .long       UsageFault_Handler         /* Usage Fault Handler */
    .long       Default_Handler            /* Reserved */
    .long       Default_Handler            /* Reserved */
    .long       Default_Handler            /* Reserved */
    .long       Default_Handler            /* Reserved */
    .long       SVC_Handler                /* SVCall Handler */
    .long       DebugMon_Handler           /* Debug Monitor Handler */
    .long       Default_Handler            /* Reserved */
    .long       PendSV_Handler             /* PendSV Handler */
    .long       SysTick_Handler            /* SysTick Handler */

    /* External interrupts */
    .long       EMU_IRQHandler             /* 0 - EMU */
    .long       WDOG0_IRQHandler           /* 1 - WDOG0 */
    .long       LDMA_IRQHandler            /* 2 - LDMA */
    /* ------------------- */
    .size       __Vectors, . - __Vectors
```
## 5. Reset handler
```
    .text
    .align      2
    .globl      Reset_Handler
    .type       Reset_Handler, @function
Reset_Handler:
    b       .
    .size   Reset_Handler, . - Reset_Handler
```

## 6. interrupt handler
```
    .align  1
    .weak   Default_Handler
    .type   Default_Handler, @function
Default_Handler:
    b       .
    .size   Default_Handler, . - Default_Handler
```

# Reference
[1]. Understand the GNU assembler startup file of cortex M4: https://community.silabs.com/s/article/understand-the-gnu-assembler-startup-file-of-cortex-m4?language=en_US
