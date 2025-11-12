#include <am.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

// UART（Universal Asynchronous Receiver/Transmitter，通用异步收发器）
// 在 Native 平台上，UART 被映射到：TX（发送） → stdout（标准输出），RX（接收） → stdin（标准输入）
// 这样就可以通过终端进行字符 I/O。

// 将标准输入设置为非阻塞模式
// 非阻塞模式：当读取操作无法立即完成时，立即返回，不会等待数据到达
// 适合游戏等需要快速响应的场景
void __am_uart_init() {
  // 获取文件标志, STDIN_FILENO - 标准输入文件描述符,F_GETFL - 获取文件状态标志
  // 返回值：成功：文件状态标志(位掩码),失败：-1
  int ret = fcntl(STDIN_FILENO, F_GETFL);
  assert(ret != -1);
  // 添加非阻塞标志
  int flag = ret | O_NONBLOCK;
  // 设置新的文件标志
  ret = fcntl(STDIN_FILENO, F_SETFL, flag);
  assert(ret != -1);
}

void __am_uart_config(AM_UART_CONFIG_T *cfg) {
  cfg->present = true; //UART 总是可用
}

// 通过 UART 发送一个字符
void __am_uart_tx(AM_UART_TX_T *uart) {
  putchar(uart->data);
}

// 从 UART 接收一个字符
/*
在非阻塞模式下：
如果有输入
ret = 'A' (65)
如果没有输入（非阻塞模式）
ret = EOF (-1)
*/
void __am_uart_rx(AM_UART_RX_T *uart) {
  int ret = fgetc(stdin);
  // 确保返回值明确为 -1,避免 EOF 在不同平台上的差异
  if (ret == EOF) ret = -1;
  uart->data = ret;
}
