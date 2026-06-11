bits 64

global load_gdt
global load_tss
global jump_to_user

; Load the task register with the TSS selector
load_tss:
	ltr di
	ret

; rdi = user RIP, rsi = user stack top
; Builds the 5-value frame iretq pops when changing privilege,
; then "returns" into ring 3
jump_to_user:
	mov ax, 0x1B       ; user data selector (0x18 | RPL 3)
	mov ds, ax
	mov es, ax

	push 0x1B          ; SS
	push rsi           ; RSP
	push 0x202         ; RFLAGS (IF=1: interrupts stay on in ring 3)
	push 0x23          ; CS (0x20 | RPL 3)
	push rdi           ; RIP
	iretq

load_gdt:
	lgdt[rdi]

	; Set the permissions for all data related registers 

	mov ax, 0x10       ; 0x10 (2nd byte) => Kernel Data Segment
	mov ds, ax         ; Data Segment
	mov es, ax         ; Extra Segment
	mov fs, ax         ; File Segment
	mov gs, ax         ; Global Segment
	mov ss, ax         ; Stack Segment

	pop rdi            ; Store the return address (RIP) in rdi
	mov rax, 0x08      ; 0x08 (1st byte) => Kernel Code Segment
	push rax           ; Push 0x08 onto the stack
	push rdi           ; Push the return address onto the stack
	retfq              ; Return far quad (long jump back)
