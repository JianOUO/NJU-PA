#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <NDL.h>
#include <BMP.h>

int main() {
  NDL_Init(0);
  int w, h;
  // 加载 BMP 图片文件, 返回图片宽度、高度和像素数据指针
  void *bmp = BMP_Load("/share/pictures/projectn.bmp", &w, &h);
  // 确保图片加载成功
  assert(bmp);
  // 打开显示画布, 设置窗口大小
  NDL_OpenCanvas(&w, &h);
  // 在画布的0,0位置绘制宽为w,高为h的矩形, 内容为bmp的像素数据
  NDL_DrawRect(bmp, 0, 0, w, h);
  // 释放图片数据占用的内存
  free(bmp);
  NDL_Quit();
  printf("Test ends! Spinning...\n");
  // 通过死循环保持窗口打开
  while (1);
  return 0;
}
