#include <stdio.h>

/*
这是一个测试 C++ 全局对象静态初始化的程序，演示：
全局对象的构造函数在 main() 之前执行
全局对象的析构函数在程序退出时执行
C++ 预定义宏 __func__ 和 __LINE__ 的使用
*/

class Test {
public:
  Test()  { printf("%s,%d: Hello, Project-N!\n", __func__, __LINE__); }
  ~Test() { printf("%s,%d: Goodbye, Project-N!\n", __func__, __LINE__); }
};

// test：全局 Test 对象
Test test;

int main() {
  printf("%s,%d: Hello world!\n", __func__, __LINE__);
  return 0;
}
