#include <BDF.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
ch: 字符编码（ASCII 码，0-255）
bbx: 字符边界框数组 [width, height, x_offset, y_offset]
bbx[0]: 字符位图宽度
bbx[1]: 字符位图高度
bbx[2]: X 方向偏移（相对于字符原点）
bbx[3]: Y 方向偏移（相对于基线）
bitmap: 原始位图数据数组（每行一个 uint32_t）
count: 位图行数（等于 bbx[1]）

create 函数的作用：
统一尺寸：将所有字符转换为相同的 w × h 尺寸
对齐处理：根据 bbx 的偏移量正确放置字符位图
格式转换：将原始位图转换为统一的位图格式（每行一个 uint32_t）
*/
void BDF_Font::create(uint32_t ch, int *bbx, uint32_t *bitmap, int count) {
  font[ch] = new uint32_t[h];
  for (int y = 0; y < h; y ++) {
    uint32_t row = 0;
    for (int x = 0; x < w; x ++) {
      int x1 = x - bbx[2];
      int y1 = y - (h - bbx[1] - bbx[3]) - h1;
      if (x1 >= 0 && y1 >= 0 && y1 < bbx[1]) {
        uint32_t mask = bitmap[y1];
        if ( (mask >> (x1)) & 1 ) {
          row |= (1 << x);
        }
      }
    }
    font[ch][y] = row;
  }
}

BDF_Font::BDF_Font(const char *fname) {
  memset(font, 0, sizeof(font));
  FILE *fp = fopen(fname, "r");
  assert(fp);

  // 行缓冲区和命令缓冲区
  char buf[256], cmd[32];
  // valid_file: 标记是否找到 STARTFONT
  // in_bitmap: 标记是否在 BITMAP 段内
  bool valid_file = false, in_bitmap = false;
  // bm[32]: 临时存储当前字符的位图数据（最多 32 行）
  // ch: 当前字符编码
  uint32_t bm[32], ch = '\0';
  // bm_idx: 位图行索引
  // bm_bbx[4]: 当前字符的边界框
  int bm_idx, bm_bbx[4];

  // fgets(buf, 256, fp): 逐行读取文件
  while (fgets(buf, 256, fp)) {
    // sscanf(buf, "%s ", cmd): 提取第一个单词作为命令
    sscanf(buf, "%s ", cmd);
    // STARTFONT: 标记文件开始
    if (strcmp(cmd, "STARTFONT") == 0) {
      valid_file = true;
    }
    // FONTBOUNDINGBOX: 解析字体全局尺寸
    // 格式：FONTBOUNDINGBOX 8 15 0 -4
    if (strcmp(cmd, "FONTBOUNDINGBOX") == 0) {
      sscanf(buf, "%*s %d %d %d %d", &w, &h, &w1, &h1);
    }
    // STARTCHAR: 开始定义新字符
    // 格式：STARTCHAR 0020（十六进制字符编码）
    // ch 存储字符编码（如 0x20 = 空格）
    if (strcmp(cmd, "STARTCHAR") == 0) {
      sscanf(buf, "%*s %x", &ch);
    }
    // BBX: 解析字符边界框
    // 格式：BBX 2 8 3 0
    // bm_bbx[0] = 宽度（2）
    // bm_bbx[1] = 高度（8）
    // bm_bbx[2] = X 偏移（3）
    // bm_bbx[3] = Y 偏移（0）
    if (strcmp(cmd, "BBX") == 0) {
      sscanf(buf, "%*s %d %d %d %d", &bm_bbx[0], &bm_bbx[1], &bm_bbx[2], &bm_bbx[3]);
    }
    // ENDCHAR: 字符定义结束
    if (strcmp(cmd, "ENDCHAR") == 0) {
      // 如果 ch < 256，调用 create() 将位图转换为统一格式
      printf("%d\n", ch);
      if (ch < 256) {
        create(ch, bm_bbx, bm, bm_idx);
      }
      // 重置 in_bitmap 和 ch
      in_bitmap = false;
      ch = '\0';
    // BITMAP: 开始位图数据段
    } else if (strcmp(cmd, "BITMAP") == 0) {
      // 设置 in_bitmap = true, 重置 bm_idx = 0
      in_bitmap = true;
      bm_idx = 0;
    // 当 in_bitmap == true 时，解析十六进制位图行
    } else if (in_bitmap) {
      /*
      解析 "80"（空格字符的第一行）
      初始化：bm[bm_idx] = 0, idx = 0
      遍历字符：
      '8': val = 8（二进制 1000）
      '0': val = 0（二进制 0000）
      处理每个十六进制数字（4 位）：
      '8': 1000
      i=0: (8 >> 3) & 1 = 1 → bm[0] |= 1 << 0 → bm[0] = 0b0001
      i=1: (8 >> 2) & 1 = 0 → 不设置
      i=2: (8 >> 1) & 1 = 0 → 不设置
      i=3: (8 >> 0) & 1 = 0 → 不设置
      '0': 0000 → 所有位都是 0
      结果：bm[0] = 0b00000001
      */
      int idx = 0;
      bm[bm_idx] = 0;
      for (const char *p = buf; *p != '\n'; p ++) {
        int val;
        if (*p >= '0' && *p <= '9') val = *p - '0';
        else val = *p - 'A' + 10;
        for (int i = 0; i < 4; i ++) {
          if ((val >> (3 - i)) & 1) {
            bm[bm_idx] |= 1 << idx;
          }
          idx ++;
        }
      }
      bm_idx ++;
    }
    // ENDFONT: 文件结束，退出循环
    if (strcmp(cmd, "ENDFONT") == 0) {
      break;
    }
  }

  fclose(fp);
}

BDF_Font::~BDF_Font() {
  for (uint32_t *pixels: font) {
    if (pixels) delete [] pixels;
  }
}
