bits 64

global load_idt

global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

extern isr_handler

load_idt: 
	lidt [rdi]
	ret

%macro ISR_NO_ERROR_CODE 1
	isr%1:
		push 0
		push %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERROR_CODE 1
	isr%1:
		push %1
		jmp isr_common_stub
%endmacro

isr_common_stub:
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
   	push rbp
    	push r8
    	push r9
    	push r10
    	push r11
    	push r12
    	push r13
    	push r14
    	push r15

	mov rdi, rsp
	call isr_handler

	pop r15
    	pop r14
    	pop r13
    	pop r12
    	pop r11
    	pop r10
    	pop r9
    	pop r8
    	pop rbp
    	pop rdi
    	pop rsi
    	pop rdx
    	pop rcx
    	pop rbx
    	pop rax

	add rsp, 16
	iretq 

ISR_NO_ERROR_CODE  0    ; Divide by Zero
ISR_NO_ERROR_CODE  1    ; Debug
ISR_NO_ERROR_CODE  2    ; Non-maskable Interrupt
ISR_NO_ERROR_CODE  3    ; Breakpoint
ISR_NO_ERROR_CODE  4    ; Overflow
ISR_NO_ERROR_CODE  5    ; Bound Range Exceeded
ISR_NO_ERROR_CODE  6    ; Invalid Opcode
ISR_NO_ERROR_CODE  7    ; Device Not Available

ISR_ERROR_CODE     8    ; Double Fault
ISR_NO_ERROR_CODE  9    ; Coprocessor Segment Overrun

ISR_ERROR_CODE    10    ; Invalid TSS
ISR_ERROR_CODE    11    ; Segment Not Present
ISR_ERROR_CODE    12    ; Stack-Segment Fault
ISR_ERROR_CODE    13    ; General Protection Fault
ISR_ERROR_CODE    14    ; Page Fault

ISR_NO_ERROR_CODE 15    ; Reserved

ISR_NO_ERROR_CODE 16    ; x87 Floating-Point Exception
ISR_ERROR_CODE    17    ; Alignment Check
ISR_NO_ERROR_CODE 18    ; Machine Check
ISR_NO_ERROR_CODE 19    ; SIMD Floating-Point Exception
ISR_NO_ERROR_CODE 20    ; Virtualization Exception
ISR_ERROR_CODE    21    ; Control Protection Exception

ISR_NO_ERROR_CODE 22    ; Reserved
ISR_NO_ERROR_CODE 23    ; Reserved
ISR_NO_ERROR_CODE 24    ; Reserved
ISR_NO_ERROR_CODE 25    ; Reserved
ISR_NO_ERROR_CODE 26    ; Reserved
ISR_NO_ERROR_CODE 27    ; Reserved
ISR_NO_ERROR_CODE 28    ; Hypervisor Injection Exception
ISR_ERROR_CODE    29    ; VMM Communication Exception
ISR_ERROR_CODE    30    ; Security Exception

ISR_NO_ERROR_CODE 31    ; Reserved
