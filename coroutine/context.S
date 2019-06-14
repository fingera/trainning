
.globl asmContextEntry, asmContextCall

/*
	rsi 实际入口
	rdi 调用目标 用户显示声明的
	rax 调用源地址 asmContextCall自动拼接的
*/
asmContextEntry:
  // 由于WSL采用了 XMM 优化 而寄存器只有11个 可能因为不对齐造成崩溃 这里padding一下堆栈
	pushq %rdi
	pushq %rdi
	movq %rax, %rdi
	call *%rsi
	movq %rax, %rdi
	movq 80(%rax), %rcx
	popq %rax
	movq %rcx, 80(%rax)

	jmp restoreContext
asmContextCall:
	leaq 8(%rsp), %rax

	// R12 R13 R14 R15 RSI RDI RBP RBX RIP RSP ARGS
	pushq %rsi // args
	pushq %rax // ESP
	pushq -8(%rax) // RIP
	pushq %rbx
	pushq %rbp
	pushq %rdi
	pushq %rsi
	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12

	movq %rsp, %rax

/*
	恢复环境
	rdi 恢复目标
	rax 返回值
*/
restoreContext:
	movq %rdi, %rsp
	popq %r12
	popq %r13
	popq %r14
	popq %r15
	popq %rsi
	popq %rdi
	popq %rbp
	popq %rbx
	popq %r8
	popq %rsp

	jmp *%r8