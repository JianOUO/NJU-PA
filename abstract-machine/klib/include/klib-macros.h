#ifndef KLIB_MACROS_H__
#define KLIB_MACROS_H__

#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))
#define LENGTH(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define RANGE(st, ed)       (Area) { .start = (void *)(st), .end = (void *)(ed) }
#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)

#define STRINGIFY(s)        #s
#define TOSTRING(s)         STRINGIFY(s)
#define _CONCAT(x, y)       x ## y
#define CONCAT(x, y)        _CONCAT(x, y)

#define putstr(s) \
  ({ for (const char *p = s; *p; p++) putch(*p); })

// 对IOE的读写API进行封装, 方便对抽象寄存器的访问
#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })

#define io_write(reg, ...) \
  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
    ioe_write(reg, &__io_param); })

//这是一个编译期静态断言宏，用于在编译时检查常量表达式。如果条件不满足，会导致编译错误，而不是运行时错误。
// 参数 const_cond 必须是一个编译期常量表达式,例如：sizeof(int) == 4, MAX_SIZE > 100 等
// static char: 声明一个静态字符数组,static 确保变量在编译期就确定大小
// CONCAT(_static_assert_, __LINE__): 生成唯一的变量名，__LINE__ 是编译器预定义宏，表示当前行号，这样可以在在同一作用域中多次使用static_assert，避免变量名冲突
// [(const_cond) ? 1 : -1]: 如果 const_cond 为真：数组大小为 1（合法），否则为 -1（非法），编译器会报错
// __attribute__((unused)): 告诉编译器这个变量不会被使用,避免产生 "unused variable" 警告,因为这个数组只是用来触发编译错误，不会真正使用
#define static_assert(const_cond) \
  static char CONCAT(_static_assert_, __LINE__) [(const_cond) ? 1 : -1] __attribute__((unused))

#define panic_on(cond, s) \
  ({ if (cond) { \
      putstr("AM Panic: "); putstr(s); \
      putstr(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
      halt(1); \
    } })

#define panic(s) panic_on(1, s)

#endif
