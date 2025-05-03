#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>

Bitmap *bitmap_create(size_t size) {
    Bitmap *bm = malloc(sizeof(Bitmap));
    if (!bm) return NULL;

    bm->size = size;
    size_t byte_size = (size + 7) / 8; // 向上取整
    bm->bits = calloc(byte_size, sizeof(unsigned char));
    if (!bm->bits) {
        free(bm);
        return NULL;
    }
    return bm;
}

// 设置某一位为1
void bitmap_set(Bitmap *bm, size_t index) {
    if (index >= bm->size) return;
    bm->bits[index / 8] |= (1 << (index % 8));
}

// 清除某一位为0
void bitmap_clear(Bitmap *bm, size_t index) {
    if (index >= bm->size) return;
    bm->bits[index / 8] &= ~(1 << (index % 8));
}

// 获取某一位的值（0或1）
int bitmap_get(Bitmap *bm, size_t index) {
    if (index >= bm->size) return 0;
    return (bm->bits[index / 8] >> (index % 8)) & 1;
}

// 释放位图
void bitmap_free(Bitmap *bm) {
    if (bm) {
        free(bm->bits);
        free(bm);
    }
}
