#include <stdio.h>
#include <assert.h>

/*
这是一个文件系统测试程序，测试文件的读写、定位和修改操作。
文件 /share/files/num 包含 1000 行数字（每行 5 字节：4 位数字 + 换行符），程序会验证文件大小、读取特定位置的数据、修改部分内容，并验证修改结果。
*/
int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);
  // 将文件指针移动到文件末尾, 0: 偏移量, SEEK_END: 文件末尾
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  // 将文件指针移动到第 500 行的开始位置, SEEK_SET: 文件开头
  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
