#include <am.h>
#include <klib.h>

// 提供 C++ 运行时支持函数的最小化实现
// 这些是 C++ ABI（Application Binary Interface） 要求的符号
// 在混合 C 和 C++ 代码/使用 C++ 编译器编译 C 代码/链接 C++ 库时，编译器会生成对这些 ABI 函数的引用，链接时需要找到这些符号
// 提供了空实现，链接时不会报错，程序可以运行（只要不使用对应的 C++ 特性）

// 仅在非 native 平台编译, 因为Native 平台运行在 Linux/macOS 上，有完整的 C++ 运行时库，
// 系统提供的 libc++ 或 libstdc++ 已经实现了这些函数,不需要自己实现
#ifndef __ISA_NATIVE__

// 动态共享对象句柄, DSO = Dynamic Shared Object（动态共享对象）
// 用于标识共享库/动态链接库, C++ 的全局对象析构时需要这个符号
void __dso_handle() {
}

// 静态局部变量保护（获取）
// 保护静态局部变量的线程安全初始化,实现 C++11 的"magic static"特性
// CXA = C++ ABI
void __cxa_guard_acquire() {
}

// 静态局部变量保护（释放）
// 释放静态局部变量初始化的保护锁
void __cxa_guard_release() {
}

// 注册退出清理函数
// 注册在程序退出时调用的析构函数,用于清理全局/静态 C++ 对象
void __cxa_atexit() {
  assert(0);
}

#endif
