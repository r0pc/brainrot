global _start
_start :
 	;; let
	mov rax, 1
	push rax
	;; /let
	;; let
	mov rax, 2
	push rax
	;; /let
	;; let
	push QWORD [rsp + 0]
	push QWORD [rsp + 16]
	pop rax
	pop rbx
	add rax, rbx
	push rax
	;; /let
	;; exit
	push QWORD [rsp + 0]
	mov rax, 60
	pop rdi
	syscall
 ;; /exit
	mov rax, 60
	mov rdi, 0
	syscall
