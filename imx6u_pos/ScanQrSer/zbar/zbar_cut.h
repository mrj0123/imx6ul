#ifndef ZBAR_CUT_H
#define ZBAR_CUT_H

#include "zbar.h"
#include <symbol.h>

typedef struct _qr_data {
	point_t pt[4];
	char data[128];
} qr_data_t;

int zbar_decode(int w, int h, u8 *data, size_t n, u32 pixelformat, qr_data_t *qr);

#endif
