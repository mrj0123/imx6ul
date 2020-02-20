/**********************************************
 * Author: younger.liucn@hotmail.com
 * File name: dfbFont.c
 * Description:  dfbFont
 * Version, Desc
 *	1.1    Created
 *	1.2    add config
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>
#include <directfb.h>

//#include "dfbFont.h"

/* config information */
#define DFB_LAYERID_USING		 0
#define DFB_FONT_TYPE_1 "SIMHEI.TTF"


/*********************************************************
 * log flags and func of debug and error infor.[Begin]
 ********************************************************/


#ifdef _DFB_DEBUG_
#define DFB_DEBUG(format, ...)	  do {		 \
	printf("[BootAnimation:%4dL]DEBUG: "format, __LINE__, ##__VA_ARGS__);  \
} while (0)
#else
#define DFB_DEBUG(format, ...)	  do {} while (0)
#endif

#define DFB_NOTICE(format, ...)    do { 	  \
	printf("[BootAnimation:%4dL]INFO: "format, __LINE__, ##__VA_ARGS__);  \
} while (0)

#define DFB_ERROR(format, ...)	  do {		 \
	printf("[BootAnimation:%4dL]ERROR: "format, __LINE__, ##__VA_ARGS__);  \
} while (0)
/*********************************************************
 * log flags and func of debug and error infor.[End]
 ********************************************************/

/*********************************************************
 * Data structure and Global variants [Begin]
 ********************************************************/
/* ʹ��directFB��ͼ�����DFB��Դ */
struct DfbResDsc {
	IDirectFB				*dfb;
	IDirectFBDisplayLayer	*layer;
};
static struct DfbResDsc badsc;

struct WinResDsc {
	IDirectFBWindow 		*window;
	IDirectFBSurface		*surface;
};
static struct WinResDsc winText;
/* ���λ��ֵ(OFFSET_X��OFFSET_Y)����Ϊ-1,��ʾ������ʾ */
#define WINDOW_TEXT_LEN_X	(300)
#define WINDOW_TEXT_LEN_Y	(30)
#define WINDOW_TEXT_OFFSET_X	(0)
#define WINDOW_TEXT_OFFSET_Y	(0)
static char *testString = "Hello world! This is DirectFB!";

static struct WinResDsc winAnim;
/* ���λ��ֵ(OFFSET_X��OFFSET_Y)����Ϊ-1,��ʾ������ʾ */
#define WINDOW_ANIM_LEN_X	(200)
#define WINDOW_ANIM_LEN_Y	(200)
#define WINDOW_ANIM_OFFSET_X	(0)
#define WINDOW_ANIM_OFFSET_Y	(0)

/* config information */
#define ANIM_IMAGES_COUNT		  2
/* support png, jpg */
#define ANIM_IMAGE_FORMAT		  "jpg"
#define ANIM_FILE_NAME_MAX_SIZE   255
#define ANIM_IMG_DEFAULT_WIDTH	  186
#define ANIM_IMG_DEFAULT_HEIGHT   186
#define ANIM_IMG_DEFAULT_FPS	  10
#define ANIM_MAX_RUNNING_MTIME			  (2000000)

#define ANIM_DEFAULT_LOGO_PATH	  "."

static IDirectFBSurface *imgSfc[ANIM_IMAGES_COUNT];
/*********************************************************
 * Data structure and Global variants [End]
 ********************************************************/

/* ��ʼ��/�ͷ���Դdfb/layer  [Begin] */
void freeResources()
{
	/* Release the layer. */
	if (badsc.layer)
		badsc.layer->Release(badsc.layer);

	badsc.dfb->Release(badsc.dfb);

	return ;
}

static void initResources(int argc, char **argv)
{
	DFBResult ret;
	badsc.dfb		= NULL;
	IDirectFB *dfb	= NULL;

	/* ��ʼ��DirectFB */
	DirectFBInit(&argc, &argv);
	DirectFBCreate(&dfb);
	if (!dfb) {
		DFB_ERROR("directfb interface is NULL\n");
		return ;
	}
	badsc.dfb = dfb;

	/* ��ʼ�� display layer������DFB_LAYERID_USING����Ϊ0.*/
	ret = badsc.dfb->GetDisplayLayer(badsc.dfb, DFB_LAYERID_USING, &(badsc.layer));
	if(ret != (DFBResult)DFB_OK) {
		DFB_ERROR("Get layer(%d) failed!\n", DFB_LAYERID_USING);
		goto fail;
	} else {
		DFB_DEBUG("Get layer(%d) independently.\n", DFB_LAYERID_USING);
	}
	return ;
fail:
	freeResources();
	return ;
}
/* ��ʼ��/�ͷ���Դdfb/layer  [End] */

/* ��ʼ��/�ͷ���ԴWinResDsc  [Begin] */
static void deinitWinRes(struct WinResDsc *winres)
{

	winres->surface->SetColor(winres->surface, 0, 0, 0, 0);
	winres->surface->Clear(winres->surface, 0, 0, 0, 0);
	winres->surface->Flip(winres->surface, NULL, DSFLIP_WAITFORSYNC);

	/* Release the window's surface. */
	if(winres->surface)
		winres->surface->Release(winres->surface);
	/* Release the window. */
	if (winres->window)
		winres->window->Release(winres->window);

	return ;
}

static void initWinResPercent(struct WinResDsc *winres,
						int window_offset_x,
						int window_offset_y,
						int window_len_x,
						int window_len_y
						)
{
	DFBResult ret;
	IDirectFBWindow 		*window 	= NULL;
	IDirectFBSurface		*surface	= NULL;
	DFBWindowDescription	desc;
	DFBDisplayLayerConfig	config;

	/* ��ȡdisplay layer�����ã�. */
	badsc.layer->GetConfiguration(badsc.layer, &config);

	/* ����window������������Window */
	desc.flags	 = (DFBWindowDescriptionFlags)(DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_OPTIONS);
	DFB_NOTICE("Layer Screen width %d, height %d !\n", config.width, config.height);
	/* �̶�window��layer����λ�� */
	if (-1 == window_offset_x) {
		desc.posx	 = (config.width - window_len_x) / 2;
	} else {
		desc.posx	 = window_offset_x;
	}
	if (-1 == window_offset_y) {
		desc.posy	 = (config.height - window_len_y) / 2;
	} else {
		desc.posy	 = window_offset_y;
	}
	desc.width	 = window_len_x;
	desc.height  = window_len_y;
	desc.caps	 = (DFBWindowCapabilities)(DWCAPS_NODECORATION);
	desc.options = (DFBWindowOptions) (DWOP_GHOST);
	ret = badsc.layer->CreateWindow(badsc.layer, &desc, &window);
	if(ret != (DFBResult)DFB_OK) {
		DFB_ERROR("Create window failed!\n");
		return ;
	}

	/* ����͸���� */
	ret = window->SetOpacity(window, 0x7F);
	if(ret != (DFBResult)DFB_OK) {
		DFB_ERROR("SetOpacity failed!\n");
		window->Release(window);
		return ;
	}
	window->SetCursorFlags(window, DWCF_INVISIBLE);

	/* ��ȡwindow��surface. */
	ret = window->GetSurface(window, &surface);
	if(ret != (DFBResult)DFB_OK) {
		DFB_ERROR("GetSurface failed!\n");
		return ;
	}
	/*����ִ��clear������ܳ��ֻ���*/
	surface->SetColor(surface, 0, 0, 0, 0);
	surface->Clear(surface, 0, 0, 0, 0);
	surface->Flip(surface, NULL, DSFLIP_WAITFORSYNC);

	winres->window = window;
	winres->surface = surface;
	return ;
}
/* ��ʼ��/�ͷ���ԴWinResDsc  [End] */

/* �����ַ��� */
static void flipText(char* str) {
	int width = 0, height = 0;
	IDirectFBFont* font = NULL;
	DFBFontDescription font_desc;
	IDirectFBSurface *surface = winText.surface;

	surface->GetSize(surface, &width, &height);

	/* font init */
	font_desc.flags = DFDESC_HEIGHT;
	font_desc.height = height / 2;
	badsc.dfb->CreateFont(badsc.dfb, DFB_FONT_TYPE_1, &font_desc, &font);

	surface->Clear(surface, 0x0, 0x0, 0x0, 0xff);
	surface->SetFont(surface, font);
	/* ����ǰ����ɫ */
	surface->SetColor(surface, 0x00, 0xFF, 0x00, 0xFF);
	surface->DrawString(surface, str, strlen(str), width / 2, height / 2, DSTF_CENTER);
	/* �任������surface buffer */
	surface->Flip(surface, NULL, DSFLIP_WAITFORSYNC);
	return ;
}


/* ���ƶ��� [Begin] */
static inline int checkFileExist(const char *filename)
{
	if((access(filename, R_OK)) != -1)
		return 1;

	return 0;
}
static int doLoadImg(const char  *filename,
						IDirectFBSurface	  **surface,
						unsigned int		   *width,
						unsigned int		   *height)
{
	int ret;
	IDirectFB *dfb = badsc.dfb;
	DFBSurfaceDescription img_dsc;
	IDirectFBImageProvider *provider = NULL;

	if(NULL == surface || NULL == filename) {
		DFB_ERROR("doLoadImg() failed for %d.\n", -EINVAL);
		return -EINVAL;
	}

	DFB_DEBUG("doLoadImg() entry:%s .\n", filename);
	/* ���ͼƬ�Ƿ���� */
	if(!checkFileExist(filename)) {
		DFB_ERROR("file %s does not exist.\n", filename);
		return -EINVAL;
	}

	/* ��Ҫ��ʾ��ͼƬ���������Ϣ������һ��image provider�� */
	ret = dfb->CreateImageProvider(dfb, filename, &provider);
	if(ret) {
		DFB_ERROR("CreateImageProvider() for %s failed %d.\n", filename, ret);
		return ret;
	}

	/* ��������provider�е�ͼƬ��Ϣ��ȡ����������surface description�� */
	ret = provider->GetSurfaceDescription(provider, &img_dsc);
	if(ret) {
		DFB_ERROR("GetSurfaceDescription() for %s failed %d.\n",
				filename, ret);
		provider->Release(provider);
		return ret;
	}

	/* ����surface description����surface���ߴ���ͼƬ��С��ȫһ�� */
	ret = dfb->CreateSurface(dfb, &img_dsc, surface);
	if(ret) {
		DFB_ERROR("CreateSurface() for %s failed %d.\n", filename, ret);
		provider->Release(provider);
		return ret;
	}

	/* ��ͼƬ�ʵݸ��ղŽ�����logoƽ�棬�����С��һ�£���������� */
	ret = provider->RenderTo(provider, *surface, NULL);
	if(ret) {
		DFB_ERROR("RenderTo() for %s failed %d.\n", filename, ret);
		(*surface)->Release(*surface);
		provider->Release(provider);
		return ret;
	}

	/* Return width / height? */
	if(width) {
		*width = img_dsc.width;
	}
	if(height){
		*height  = img_dsc.height;
	}

	/* release provider */
	provider->Release(provider);

	DFB_DEBUG("doLoadImg() exit.\n");
	return ret;
}

/* �ͷ�ͼƬ��Դ */
static void deinitImages()
{
  int i = 0;
  for (i = 0; i < ANIM_IMAGES_COUNT; i++) {
	  if (imgSfc[i]) {
		  imgSfc[i]->Release(imgSfc[i]);
	  } else {
		  break;
	  }
  }
  return ;
}
/* ��ʼ��ͼƬ��Դ */
static int initImages()
{
	int ret = 0, i = 0;
	char filename[ANIM_FILE_NAME_MAX_SIZE];
	IDirectFBSurface *tmp_sfc = NULL;

	for (i = 0; i < ANIM_IMAGES_COUNT; i++) {
		imgSfc[i] = NULL;
	}

	for (i = 0; i < ANIM_IMAGES_COUNT; i++) {
		tmp_sfc = NULL;
		memset(filename, 0x0, sizeof(filename));
		snprintf(filename, ANIM_FILE_NAME_MAX_SIZE,
				"%s/%03d.%s", ANIM_DEFAULT_LOGO_PATH, i, ANIM_IMAGE_FORMAT);
		ret = doLoadImg(filename, &tmp_sfc, NULL, NULL);
		if (ret != 0) {
			goto bail;
		}
		imgSfc[i] = tmp_sfc;
	}

	return 0;
bail:
	deinitImages();
	return -1;
}


static void fillDFBSurface(
						IDirectFBSurface *primary_sfc,
						IDirectFBSurface *img_sfc,
						int x, int y)
{
	primary_sfc->Clear(primary_sfc, 0, 0, 0, 255);
	/*
	 * blit��������λͼ����thiz��source�����������ķ�����ϳ�һ��ͼƬ��
	 * ��DirectFB�У��䶨��Ķ���Ҳ�Ǻ���ģ���img_sfc blit��primary_sfc��ȥ��
	 * ����ƽ���ϵ����ݻ��������
	 */
	primary_sfc->Blit(primary_sfc, img_sfc, NULL, x, y);
	/* �任������surface buffer */
	primary_sfc->Flip(primary_sfc, NULL, DSFLIP_WAITFORSYNC);

	return ;
}

/* ��ȡb��a��ʱ����λ������ */
static unsigned long deltaMsecs(struct timespec *a,
		struct timespec *b)
{
	long delta_secs = 0, delta_msecs = 0;

	if(a->tv_sec < b->tv_sec ||
			(a->tv_sec == b->tv_sec && a->tv_nsec < b->tv_nsec)) {
		return 0;
	}

	delta_secs = a->tv_sec - b->tv_sec;
	delta_msecs = (a->tv_nsec - b->tv_nsec) / 1000000;
	while(delta_msecs < 0) {
		delta_secs--;
		delta_msecs += 1000;
	}

	return delta_secs * 1000 + delta_msecs;
}

static void doAnimation()
{
	int ret, i;
	struct timespec before_draw, after_draw;
	unsigned long elapsed_msec, total_msec;
	IDirectFBSurface *primary = winAnim.surface;
	unsigned long interval = (1000 / ANIM_IMG_DEFAULT_FPS);

	if (initImages()) {
		DFB_ERROR("Init images failed!\n");
		return ;
	}

	DFB_NOTICE("Animation start ...\n");
	total_msec = 0;
	i  = 0;
	do {
		if(i >= ANIM_IMAGES_COUNT) {
			i = 0;
		}

		clock_gettime(CLOCK_MONOTONIC, &before_draw);
		fillDFBSurface(primary, imgSfc[i],
				0,0);
		clock_gettime(CLOCK_MONOTONIC, &after_draw);
		/*
		 * ��ȡ��ͼǰ�����ε�ʱ���elapsed_msec
		 * ���elapsed_msec>interval,��������
		 * ��������interval-elapsed_msec
		 */
		elapsed_msec = deltaMsecs(&after_draw, &before_draw);
		if(elapsed_msec < interval) {
			usleep((interval - elapsed_msec) * 1000);
			total_msec += interval;
		} else {
			total_msec += elapsed_msec;
		}
		DFB_DEBUG("elapsed %lu ms \n",
						elapsed_msec < interval ? interval : elapsed_msec);
		if(total_msec >= ANIM_MAX_RUNNING_MTIME) {
			DFB_NOTICE("Stopped by Timeout(%lu).\n", total_msec);
			break;
		}

		i++;
	} while(1);

out:
	DFB_NOTICE("Animation exit with black screen...\n");
	deinitImages();

	return ;
}
/* ���ƶ��� [End] */

static void doShow()
{
	DFB_NOTICE("Text start ...\n");
	flipText(testString);
	//doAnimation();

	return ;
}

int main(int argc, char **argv)
{
	DFB_NOTICE("Animation entry.\n");
	initResources(argc, argv);
	initWinResPercent(&winAnim,
			WINDOW_ANIM_OFFSET_X, WINDOW_ANIM_OFFSET_Y,
			WINDOW_ANIM_LEN_X, WINDOW_ANIM_LEN_Y);
	initWinResPercent(&winText,
			WINDOW_TEXT_OFFSET_X, WINDOW_TEXT_OFFSET_Y,
			WINDOW_TEXT_LEN_X, WINDOW_TEXT_LEN_Y);
	doShow();
	deinitWinRes(&winText);
	deinitWinRes(&winAnim);
	freeResources();

	DFB_NOTICE("Animation exit.\n");
	return 0;
}

