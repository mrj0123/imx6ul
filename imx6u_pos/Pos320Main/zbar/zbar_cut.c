#include "system.h"
#include "zbar_cut.h"
#include <stdio.h>
#include "image.h"
#include <sys/time.h>
#include "zbar_cut.h"

/* 准备初始数据zbar初始数据,只用初始化一次; */
int zbar_init(zbar_image_scanner_t **scanner, struct zbar_image_s **image)
{
	/* wrap image data */
	*image = zbar_image_create();
	*scanner = zbar_image_scanner_create();
	if (!*image || !*scanner) {
		return -1;
	}
	/* configure the reader */
	zbar_image_scanner_set_config(*scanner, 0, ZBAR_CFG_ENABLE, 1);
	return 0;
}

int zbar_decode(int w, int h, u8 *data, size_t n, u32 pixelformat, qr_data_t *qr)
{
	/* scan the image for barcodes; */
	/* wrap image data */
	struct zbar_image_s *image = NULL;
	zbar_image_scanner_t *scanner = NULL;

	/* obtain image data */
	int width = w;
	int height = h;
	int ret;
    //printf("zbar init\n");
	if (zbar_init(&scanner, &image)) {
		return -1;
	}
	/* 获取图像; */
    zbar_image_set_size(image, width, height);
    //zbar_image_set_format(image, fourcc('Y','U','Y','V'));
    zbar_image_set_format(image, fourcc('U','Y','V','Y'));
    zbar_image_set_data(image, data, n, NULL);
    //printf("zbar set image\n");

    zbar_image_t *test = zbar_image_convert(image, fourcc('Y','8','0','0'));
    //printf("zbar convert image\n");
    if(!test)
        return (-2);
        /*
    printf("converted: %d x %d (%lx) %08lx\n",
           zbar_image_get_width(test),
           zbar_image_get_height(test),
           zbar_image_get_data_length(test),
           zbar_image_get_format(test));
        */
	zbar_image_destroy(image);

	image = test;
	unsigned int fmt = zbar_image_get_format(image);
	printf("processing: %.4s(%08" PRIx32 ") %dx%d @%p\n",
			(char*)&fmt, fmt,
			zbar_image_get_width(image), zbar_image_get_height(image),
			zbar_image_get_data(image));

	/* scan the image for barcodes; */
	n = zbar_scan_image(scanner, image);

    /* extract results */
    ret=0;
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\" [(%d, %d), (%d, %d), (%d, %d), (%d, %d)]\n",
               zbar_get_symbol_name(typ), data,
               zbar_symbol_get_loc_x(symbol, 0), zbar_symbol_get_loc_x(symbol, 0),
               zbar_symbol_get_loc_x(symbol, 1), zbar_symbol_get_loc_y(symbol, 1),
               zbar_symbol_get_loc_x(symbol, 2), zbar_symbol_get_loc_y(symbol, 2),
               zbar_symbol_get_loc_x(symbol, 3), zbar_symbol_get_loc_y(symbol, 3)
               );
        ret=1;
		memset(qr->data, 0, sizeof(qr->data));
		strncpy(qr->data, data, sizeof(qr->data) - 1);
		qr->pt[0].x = zbar_symbol_get_loc_x(symbol, 0);
		qr->pt[0].y = zbar_symbol_get_loc_y(symbol, 0);
		qr->pt[1].x = zbar_symbol_get_loc_x(symbol, 1);
		qr->pt[1].y = zbar_symbol_get_loc_y(symbol, 1);
		qr->pt[2].x = zbar_symbol_get_loc_x(symbol, 2);
		qr->pt[2].y = zbar_symbol_get_loc_y(symbol, 2);
		qr->pt[3].x = zbar_symbol_get_loc_x(symbol, 3);
		qr->pt[3].y = zbar_symbol_get_loc_y(symbol, 3);
    }

	/* clean up */
	zbar_image_destroy(image);
	zbar_image_scanner_destroy(scanner);
	return ret;
}
