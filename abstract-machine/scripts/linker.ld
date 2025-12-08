/* 指定程序入口点为_start, 程序启动时从 _start 开始执行*/
ENTRY(_start)
/* 
定义程序头（Program Headers）
text PT_LOAD：代码段标记为可加载 
data PT_LOAD：数据段标记为可加载
*/
PHDRS { text PT_LOAD; data PT_LOAD; }

SECTIONS {
  /* 
  _pmem_start and _entry_offset are defined in LDFLAGS
  . ：位置计数器（Location Counter），当前输出地址
  含义：从 0x80000000 开始布局所有段。
  */
  . = _pmem_start + _entry_offset;
  /* 
  定义 .text 段（代码段）的布局规则
  *：通配符，匹配所有输入文件(.o 和 .a); (entry)：段名，匹配名为 entry 的段
  *(entry):通配符模式，匹配所有输入文件中的 entry 段
  *(.text*):通配符模式，匹配所有输入文件中以 .text 开头的段
  使用 -ffunction-sections 时，每个函数可能被放入独立段：.text.main .text._trm_init ...
  ": text":将 .text 输出段关联到 text 程序头(第四行定义),告诉链接器该段需要加载到内存,在 ELF 文件的程序头表中创建对应条目
  */
  .text : {
    *(entry)
    *(.text*)
  } : text
  /* 
  etext 和 _etext：指向 .text 段结束地址
  用途：在 C 代码中获取代码段边界
  */
  etext = .;
  _etext = .;
  /* .rodata 段（只读数据），包含：字符串字面量、常量数组等只读数据 */
  .rodata : {
    *(.rodata*)
  }
  /* .data 段（已初始化数据），包含：已初始化的全局变量和静态变量
  ": data" ：关联到 data 程序头
  */
  .data : {
    *(.data*)
  } : data
  /* edata 和 _data：指向 .data 段结束地址 */
  edata = .;
  _data = .;
  /* .bss：未初始化的全局/静态变量
  _bss_start = .; : 将 .bss 段的起始地址保存到 _bss_start，供 C 代码使用
  .sbss：小 .bss（某些架构）
  .scommon：小公共数据
  */
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }
  /* ALIGN(0x1000)：向上对齐到 4KB（0x1000）边界 */
  _stack_top = ALIGN(0x1000);
  /* 将位置计数器移动到栈顶上方 32KB（0x8000），为栈预留 32KB 空间*/
  . = _stack_top + 0x8000;
  /* _stack_pointer：栈指针初始值(栈顶),在_start(start.S)中设置栈指针*/
  _stack_pointer = .;
  /* end 和 _end：程序结束地址, 用途：标记程序占用的内存范围*/
  end = .;
  _end = .;
  /* _heap_start：堆起始地址(4KB 对齐),在 C 代码中定义堆区域*/
  _heap_start = ALIGN(0x1000);
}