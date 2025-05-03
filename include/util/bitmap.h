#ifndef BITMAP_H
#define BITMAP_H
#include <stddef.h>
typedef struct
{
    unsigned char *bits; // 存储位图的数组
    size_t size;         // 位数（多少位）
} Bitmap;

// 初始化位图
Bitmap *bitmap_create(size_t size);

// 设置某一位为1
void bitmap_set(Bitmap *bm, size_t index);

// 清除某一位为0
void bitmap_clear(Bitmap *bm, size_t index);

// 获取某一位的值（0或1）
int bitmap_get(Bitmap *bm, size_t index);

// 释放位图
void bitmap_free(Bitmap *bm);

#endif
