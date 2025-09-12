# Context switch
#
#   void swtch(struct context *old, struct context *new);
# 
# Save current registers in old. Load from new.	
# swtch函数的目的是保存旧上下文并加载新上下文
# a0=cpu context; a1=p context
# 由于在加载新上下文时，已经将新上下文的 ra 寄存器值加载进来了，所以 ret 指令会跳转到新上下文之前调用 swtch 的地方，从而完成上下文切换

# 比如有两个线程 A 和 B, A 正在运行，调用 swtch(&A->context, &B->context), swtch 会：
# 1. 保存 A 的寄存器到 A->context
# 2. 加载 B 的寄存器从 B->context
# 3. ret 跳转到 B 上次保存的执行位置
# 从调度器的角度看，就是 "切走 A → 切入 B"

.globl swtch
swtch:
        # 这部分代码将旧执行流中**被调用者保存寄存器（callee-saved registers）**的值保存到由a0指向的内存区域。
        # 这些寄存器包括ra（返回地址）、sp（栈指针）和s0 - s11（通用保存寄存器）。
        # 保存这些寄存器是确保当该执行流再次被调度时，可以从正确的地方继续执行
        # ra 保存返回地址即scheduler()中`swtch(&c->context, &p->context)`后继续执行的指令的地址=指令`c->proc = 0`的地址
        sd ra, 0(a0)
        sd sp, 8(a0)
        sd s0, 16(a0)
        sd s1, 24(a0)
        sd s2, 32(a0)
        sd s3, 40(a0)
        sd s4, 48(a0)
        sd s5, 56(a0)
        sd s6, 64(a0)
        sd s7, 72(a0)
        sd s8, 80(a0)
        sd s9, 88(a0)
        sd s10, 96(a0)
        sd s11, 104(a0)

        ld ra, 0(a1)
        ld sp, 8(a1)
        ld s0, 16(a1)
        ld s1, 24(a1)
        ld s2, 32(a1)
        ld s3, 40(a1)
        ld s4, 48(a1)
        ld s5, 56(a1)
        ld s6, 64(a1)
        ld s7, 72(a1)
        ld s8, 80(a1)
        ld s9, 88(a1)
        ld s10, 96(a1)
        ld s11, 104(a1)
        
        # ret 指令本质上是 jalr x0, 0(ra)，它会跳转到 ra 寄存器中保存的地址
        ret

	
