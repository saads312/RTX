.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.global SVC_Handler_Main
.global PendSV_Handler_Main
.global change_task

.section  .text.SVC_Handler
.weak  SVC_Handler
.type  SVC_Handler, %function
SVC_Handler:
    TST lr, #4
    ITE EQ
    MRSEQ r0, MSP
    MRSNE r0, PSP
    B SVC_Handler_Main

.section  .text.PendSV_Handler
.weak  PendSV_Handler
.type  PendSV_Handler, %function
PendSV_Handler:
    MRS r0, psp
    STMFD r0!, {r4-r11}
    MSR psp, r0
    BL change_task
    MRS r0, psp
    LDMIA r0!, {r4-r11}
    MSR psp, r0
    MOV lr, #0xFFFFFFFD
    BX lr
