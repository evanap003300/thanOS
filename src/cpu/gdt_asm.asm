bits 64

global load_gdt

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
