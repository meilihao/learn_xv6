//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicking = 0; // printing a panic message
volatile int panicked = 0; // spinning forever at end of a panic

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
} pr;

static char digits[] = "0123456789abcdef";

// 负责将整数转换为字符串并输出
static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if(sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4) // 4bit转换为一个字符. 将 x 的值向左位移 4 位, 将下一个要打印的 4 位十六进制数移到最高位
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]); // 将 x 向右位移 60 位, 这会把 x 的最高 4 位（也就是最左边的 4 位）移到最低位
}

// Print to the console.
int
printf(char *fmt, ...)
{
  va_list ap; // va_list 是一个宏，用于访问可变参数列表, ap 是一个指向参数列表的指针
  int i, cx, c0, c1, c2;
  char *s; // 用于处理 %s 格式

  // 在非紧急状态下，获取一个锁 (pr.lock). 因为 printf 可能会在多核环境中被多个 CPU 核心同时调用.
  // 锁确保了同一时间只有一个核心可以执行 printf，防止输出混乱
  if(panicking == 0)
    acquire(&pr.lock);

  va_start(ap, fmt); // 这是一个宏，用于初始化 ap 指针，使其指向可变参数列表的第一个参数。它以最后一个固定参数 (fmt) 为基准来定位可变参数
  for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
    if(cx != '%'){ // 如果当前字符不是 %，说明它是一个普通字符
      consputc(cx);
      continue;
    }
    i++; // 跳到下一个字符，准备解析格式占位符
    c0 = fmt[i+0] & 0xff;
    c1 = c2 = 0;
    if(c0) c1 = fmt[i+1] & 0xff;
    if(c1) c2 = fmt[i+2] & 0xff;
    if(c0 == 'd'){
      // va_arg(ap, int) 从参数列表中取出下一个 int 类型的参数
      printint(va_arg(ap, int), 10, 1);
    } else if(c0 == 'l' && c1 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if(c0 == 'u'){
      printint(va_arg(ap, uint32), 10, 0);
    } else if(c0 == 'l' && c1 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if(c0 == 'x'){
      printint(va_arg(ap, uint32), 16, 0);
    } else if(c0 == 'l' && c1 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if(c0 == 'p'){
      printptr(va_arg(ap, uint64));
    } else if(c0 == 'c'){
      consputc(va_arg(ap, uint));
    } else if(c0 == 's'){
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
    } else if(c0 == '%'){
      consputc('%');
    } else if(c0 == 0){
      break;
    } else {
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c0);
    }

  }
  va_end(ap); // 这是一个宏，用于清理 va_list 资源，在所有参数处理完毕后调用

  if(panicking == 0)
    release(&pr.lock);

  return 0;
}

void
panic(char *s)
{
  panicking = 1;
  printf("panic: ");
  printf("%s\n", s);
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
}
