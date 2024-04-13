# swapco(context_t curr,context_t pending)
.globl swapco
swapco:
	leaq 8(%rsp),%rax
	leaq 112(%rdi),%rsp 
	pushq %rax  
	pushq %rbx  
	pushq %rcx
	pushq %rdx
	pushq -8(%rax)
	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq %rsi, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r9
	popq %r8
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rax
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rsp
	pushq %rax
	xorl %eax, %eax
	ret