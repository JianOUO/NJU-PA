#ifndef __BDF_H__
#define __BDF_H__

#include <stdint.h>
// BDF_Font 用于解析和存储 BDF（Bitmap Distribution Format）字体文件。
// BDF 是一种位图字体格式，常用于嵌入式系统
class BDF_Font {
private:
  // 字体边界框的偏移量（从 FONTBOUNDINGBOX 解析）
  // w1: X 方向偏移（通常为 0 或负值）
  // h1: Y 方向偏移（通常为负值，表示基线以下）
  // 用于字符对齐和定位
  int w1, h1;

  // 将单个字符的位图数据转换为统一格式
  // ch: 字符编码（ASCII 码）
  // bbx: 字符边界框 [width, height, x_offset, y_offset]
  // bitmap: 原始位图数据（十六进制字符串转换而来）
  // count: 位图行数
  void create(uint32_t ch, int *bbx, uint32_t *bitmap, int count);

public:
  // 字体名称（未在构造函数中设置，可能未使用）
  const char *name;
  // 字体统一尺寸
  // w: 字符宽度（像素）
  // h: 字符高度（像素）
  // 所有字符使用相同尺寸，便于渲染对齐
  int w, h;
  /*
  字符位图数组
  索引为字符 ASCII 码（0-255）, font[ch][y] 表示字符 ch 的第 y 行
  每行是一个 uint32_t，从右到左表示像素（LSB 对应最右像素）
  每个 uint32_t 的每一位代表一个像素：
  位为 1：前景色（字符像素）
  位为 0：背景色（透明/空白）
  位的顺序：从右到左（LSB 是最右像素）
  第 0 位（LSB）：最右列（x=0）
  第 1 位：右数第 2 列（x=1）
  ...
  第 31 位（MSB）：最左列（x=31）
  font['A'][0]  = 0b00010000;  // 第 0 行：第 4 列有像素
  font['A'][1]  = 0b00101000;  // 第 1 行：第 3、5 列有像素
  font['A'][2]  = 0b01000100;  // 第 2 行：第 2、6 列有像素
  ... 其他行
  */
  uint32_t *font[256];

  BDF_Font(const char *filename);
  ~BDF_Font();
};

#endif
