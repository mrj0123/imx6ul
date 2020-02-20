#include "dealcmd.h"
#include "a.h"
#include "cJSON.h"
#include <directfb.h>
#include <directfb_strings.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iconv.h>

#if 0
    #define pr_debug(fmt,arg...) \
        myPtf("%s %d: "fmt,__FILE__,__LINE__,##arg)
#else
    #define pr_debug(fmt,arg...)
#endif

typedef unsigned long long u64;
typedef signed long long   s64;
typedef unsigned int	   u32;
typedef signed int	       s32;
typedef unsigned short	   u16;
typedef signed short	   s16;
typedef unsigned char	    u8;
typedef signed char	        s8;

typedef struct __text_t {
	char name[100];
	u16 x;
	u16 y;
} __attribute__((__packed__)) text_t;

typedef struct __pic_t {
	char name[16];
	u16 x;
	u16 y;
	char font[16];
	unsigned int size;
	unsigned int color;
	text_t text;
} __attribute__((__packed__)) pic_t;
const char *pic_path = "./";

/* the super interface */
static IDirectFB *dfb;

/* the primary surface */
static IDirectFBSurface *primary;
//static IDirectFBSurface *dest;

/*屏幕大小 */
static int screen_width;
static int screen_height;

/* image surface of background pic */
static IDirectFBSurface *bkpic_surface;
static DFBSurfaceDescription bkpic_dsc;

/* font interface */
static IDirectFBFont *font;
static char font_name[256];
static int font_size = -1;
static int font_height;

#ifdef USE_ICONV
/* */
static iconv_t g_iconv = (iconv_t)(-1);

static int init_iconv()
{
	int set = 0;
	g_iconv = iconv_open("UTF-8", "GB18030");
	if (g_iconv == (iconv_t)(-1)) {
		return -1;
	}
	iconvctl(g_iconv, ICONV_SET_TRANSLITERATE, (void*)&set);
	return 0;
}

static int release_iconv()
{
	return iconv_close(g_iconv);
}
#endif

//显示服务线程
static void * SecScreenShowThread(void *arg)
{
    return (void *)NULL;
}

int exit_dxfb(void)
{
	/* Release font */
	if (font) {
		font->Release(font);
		font_size = -1;
		font = NULL;
	}

	/* Release the bkpic surface */
	if (bkpic_surface) {
		bkpic_surface->Release(bkpic_surface);
		bkpic_surface = NULL;
	}

	/* Release the primary surface */
	if (primary) {
		primary->Release(primary);
		primary = NULL;
	}

	/* Release the super interface */
	if (dfb) {
		dfb->Release(dfb);
		dfb = NULL;
	}

	return 0;
}

/*  */
int init_dxfb(const char *bk)
{
	DFBResult ret;
	DFBSurfaceDescription surface_dsc;
	IDirectFBImageProvider *provider;

#ifdef USE_ICONV
	/*  */
	if (init_iconv()) {
		pr_debug("iconv init failed\n");
		return -1;
	}
#endif

	/* DirectFB init */
	ret = DirectFBInit(NULL, NULL);
	if (ret != DFB_OK) {
		pr_debug("DirectFBInit failed %d\n", ret);
		goto err;
	}

	/* Create the super interface */
	ret = DirectFBCreate(&dfb);
	if (ret != DFB_OK) {
		pr_debug("DirectFBCreate failed %d\n", ret);
		goto err;
	}

	/* Set the cooperative level to DFSCL_FULLSCREEN */
	ret = dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);
	if (ret != DFB_OK) {
		pr_debug("SetCooperativeLevel DFSCL_FULLSCREEN failed %d\n", ret);
		goto err;
	}

	/* Get the primary surface, the surface of the primary layer. */
	surface_dsc.flags = DSDESC_CAPS;
	surface_dsc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
	ret = dfb->CreateSurface(dfb, &surface_dsc, &primary);
	if (ret != DFB_OK) {
		pr_debug("create primary surface failed %d\n", ret);
		goto err;
	}

	/* Get screen size */
	ret = primary->GetSize(primary, &screen_width, &screen_height);
	if (ret != DFB_OK) {
		pr_debug("primary Getsize failed %d\n", ret);
		goto err;
	}
	pr_debug("screen size: (%d, %d)\n", screen_width, screen_height);
	primary->Clear(primary, 0, 0, 0, 0xFF);

	/* dest: GetSubSurface */
#if 0
	primary->GetSubSurface(primary, NULL, &dest);
	dest->SetBlittingFlags(dest, DSBLIT_BLEND_ALPHACHANNEL);
#endif

	//
	if (bk) {
		//
		ret = dfb->CreateImageProvider(dfb, bk, &provider);
		if (ret != DFB_OK) {
			pr_debug("CreateImageProvider bkpic provider failed %d\n", ret);
			goto err;
		}

		//
		ret = provider->GetSurfaceDescription(provider, &bkpic_dsc);
		if (ret != DFB_OK) {
			pr_debug("bkpic GetSurfaceDescription failed %d\n", ret);
			goto err;
		}

		//
		bkpic_dsc.width = screen_width;
		bkpic_dsc.height = screen_height;

		//
		ret = dfb->CreateSurface(dfb, &bkpic_dsc, &bkpic_surface);
		if (ret != DFB_OK) {
			pr_debug("create bkpic surface failed %d\n", ret);
			goto err;
		}

		// render to bkpic surface, and release provider
		provider->RenderTo(provider, bkpic_surface, NULL);
		provider->Release(provider);

		//
		ret = primary->SetBlittingFlags( primary, DSBLIT_BLEND_ALPHACHANNEL );
		if (ret != DFB_OK) {
			pr_debug("primary DSBLIT_BLEND_ALPHACHANNEL failed %d\n", ret);
			goto err;
		}
		ret = primary->Blit(primary, bkpic_surface, NULL, 0, 0);
		if (ret != DFB_OK) {
			pr_debug("primary Blit bkpic failed %d\n", ret);
			goto err;
		}
	}
	return 0;
err:
	exit_dxfb();
	return -1;
}

/* Load font */
int load_font(const char *fontfile, int height)
{
	DFBResult ret;
	DFBFontDescription desc;

	desc.flags		= (DFDESC_HEIGHT/* | DFDESC_ATTRIBUTES*/);
	desc.height 	= height;
	//desc.attributes = DFFA_NONE;

	// create font from fontfile
	ret = dfb->CreateFont(dfb, fontfile, &desc, &font);
	if (ret != DFB_OK) {
		pr_debug("Create Font failed %d\n", ret);
		return -1;
	}

	// record font_name, font_size
	strncpy(font_name, fontfile, sizeof(font_name - 1));
	font_size = height;

	// get font height
	font->GetHeight(font, &font_height);
	pr_debug("font height = %d\n", font_height);

	// set font to primary
	primary->SetFont(primary, font);
	pr_debug("load font ok\n");
	return 0;
}

/* release dxfb, shutdown DirectFB. */


/* */
int display_bkpic()
{
	if (bkpic_surface == NULL) {
		return -1;
	}
	DFBResult ret = primary->Blit(primary, bkpic_surface, NULL, 0, 0);
	if (ret != DFB_OK) {
		pr_debug("display_bkpic: primary Blit bkpic failed %d\n", ret);
		return -1;
	}
	return 0;
}

/*   */
int add_pic_to_dest(const char *path, int x, int y)
{
	DFBResult ret;
	IDirectFBImageProvider *provider = NULL;
	IDirectFBSurface *pic_surface = NULL;
	DFBSurfaceDescription pic_dsc;

	//
	ret = dfb->CreateImageProvider(dfb, path, &provider);
	if (ret != DFB_OK) {
		pr_debug("CreateImageProvider bkpic provider failed %d\n", ret);
		goto err;
	}

	//
	ret = provider->GetSurfaceDescription(provider, &pic_dsc);
	if (ret != DFB_OK) {
		pr_debug("bkpic GetSurfaceDescription failed %d\n", ret);
		goto err;
	}

	//
	pr_debug("pic size: %d, %d\n", pic_dsc.width, pic_dsc.height);

	//
	ret = dfb->CreateSurface(dfb, &pic_dsc, &pic_surface);
	if (ret != DFB_OK) {
		pr_debug("create bkpic surface failed %d\n", ret);
		goto err;
	}

	// render to bkpic surface, and release provider
	provider->RenderTo(provider, pic_surface, NULL);
	provider->Release(provider);
	provider = NULL;

	//
	ret = primary->SetBlittingFlags(primary, DSBLIT_BLEND_ALPHACHANNEL);
	if (ret != DFB_OK) {
		pr_debug("primary DSBLIT_BLEND_ALPHACHANNEL failed %d\n", ret);
		goto err;
	}
	ret = primary->Blit(primary, pic_surface, NULL, x, y);
	if (ret != DFB_OK) {
		pr_debug("primary Blit bkpic failed %d\n", ret);
		goto err;
	}
	pic_surface->Release(pic_surface);
	return 0;
err:
	/* release resource */
	if (provider) {
		provider->Release(provider);
	}
	if (pic_surface) {
		pic_surface->Release(pic_surface);
	}
	return -1;
}

static int add_text_to_dest(text_t *ptext)
{
	DFBResult ret;
#ifdef USE_ICONV
	char buf[256];
	int inlen = strlen(ptext->name);
	int outlen = sizeof(buf);

	//
	if (iconv(g_iconv, ptext->name, &inlen, buf, &outlen )== -1) {
		pr_debug("can't iconv charset from %s to %s!\n", from_charset, to_charset);
		strcpy(buf, ptext->name);
	}
#endif

	//

	ret = primary->DrawString(primary, /* buf */ptext->name,
		-1, ptext->x, ptext->y, DSTF_LEFT);
	if (ret != DFB_OK) {
		pr_debug("DrawString %s failed\n", ptext->name);
		return -1;
	}
	//pr_debug("DrawString %s at (%d, %d) ok\n", ptext->name, ptext->x, ptext->y);
	return 0;
}

//
int show_component(char *data, int len)
{
	char path[256];
	int i, n = (len - 44) / 12;
	pic_t *pic = (pic_t *)data;
	strcpy(path, pic_path);
	strcat(path, pic->name);
	pr_debug("start add pic img %s, len = %d, n = %d\n",
		path, len, n);
	add_pic_to_dest(path, pic->x, pic->y);
	//strcpy(path, font_path);
	strcat(path, pic->font);
	if (pic->size != font_size || strcmp(path, font_name)) {
		load_font(path, pic->size);
	}
	primary->SetColor(primary, pic->color & 0xFF,
		(pic->color >> 8) & 0xFF, (pic->color >> 16) & 0xFF, 0xff);
	text_t *ptext = &pic->text;
	for (i = 0; i < n; i++, ptext++) {
		ptext->x += pic->x;
		ptext->y += pic->y;
		add_text_to_dest(ptext);
	}
	pr_debug("end pic text\n");
	return 0;
}

//
int show_pic()
{
	primary->Flip(primary, NULL, DSFLIP_ONSYNC);
	return 0;
}
///////////////////////////////////////////////////////////////////////////
//显示照片，但不显示文字
char * fn_dealCmd_ShowPic(char * recv_buf,int recv_len,int * ret_len)
{
    //解析JSON串，获得图片背景、显示位置
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        return NULL;
    }
    //识别当前终端唯一识别号
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        myPtf("JSON_ERROR\n");
        //json串错误
        return NULL;
    }
    cJSON * jBgPic=cJSON_GetObjectItem(jRoot,"BGPicture");
    //背景如果为空，使用默认背景
    myPtf("get pic BGPicture\n");
    if(!jBgPic)
    {
        add_pic_to_dest(SECSCREEN_BG_PIC, 0, 0);
        myPtf("no bgpic,set default pic %s\n",SECSCREEN_BG_PIC);
    }
    else
    {
        char path[50];
        strcpy(path,jBgPic->valuestring);
        myPtf("pic is %s\n",path);
        add_pic_to_dest(path, 0, 0);
        myPtf("set bg pic %s\n",jBgPic->valuestring);
    }
    cJSON_Delete(jRoot);
    show_pic();
    return NULL;
}
//追加文字，不修改原来显示的背景和以前文字
char * fn_dealCmd_AppendTxt(char * recv_buf,int recv_len,int * ret_len)
{
    //解析JSON串，获得图片背景、显示位置
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        return NULL;
    }
    text_t txt;
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        myPtf("JSON_ERROR\n");
        //json串错误
        return NULL;
    }
    cJSON * jTextname=cJSON_GetObjectItem(jRoot,"Text");
    //没有文字，设置为空串
    if(!jTextname)
    {
        strcpy(txt.name, "");
    }
    else
    {
        strcpy(txt.name, jTextname->valuestring);
    }
    cJSON * jPosX=cJSON_GetObjectItem(jRoot,"PosX");
    //设置位置
    if(!jPosX)
    {
        txt.x = SECSCREEN_TXT_POSX;
    }
    else
    {
        txt.x = jPosX->valueint;
    }
    cJSON * jPosY=cJSON_GetObjectItem(jRoot,"PosY");
    //设置位置
    if(!jPosY)
    {
        txt.y = SECSCREEN_TXT_POSY;
    }
    else
    {
        txt.y = jPosY->valueint;
    }
    //设置字体大小
    cJSON * jFontSize=cJSON_GetObjectItem(jRoot,"FontSize");
    if(!jFontSize)
    {
        load_font(SECSCREEN_FONT, SECSCREEN_FONTSIZE);
    }
    else
    {
        load_font(SECSCREEN_FONT, jFontSize->valueint);
    }
    cJSON_Delete(jRoot);
    add_text_to_dest(&txt);
    show_pic();
    return NULL;
}

//显示照片，也显示文字
char * fn_dealCmd_ShowAll(char * recv_buf,int recv_len,int * ret_len)
{
    //解析JSON串，获得图片背景、显示位置
    *ret_len=0;
    if (recv_buf == NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        return NULL;
    }
    //识别当前终端唯一识别号
    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        //json串错误
        myPtf("RECVJSON_ERROR\n");
        return NULL;
    }
    cJSON * jBgPic=cJSON_GetObjectItem(jRoot,"BGPicture");
    //背景如果为空，使用默认背景
    if(!jBgPic)
    {
        add_pic_to_dest(SECSCREEN_BG_PIC, 0, 0);
        myPtf("no bgpic,set default pic %s\n",SECSCREEN_BG_PIC);
    }
    else
    {
        add_pic_to_dest(jBgPic->valuestring, 0, 0);
        myPtf("set bg pic %s\n",jBgPic->valuestring);
    }
    text_t txt;
    cJSON * jTextname=cJSON_GetObjectItem(jRoot,"Text");
    //没有文字，设置为空串
    if(!jTextname)
    {
        strcpy(txt.name, "");
    }
    else
    {
        strcpy(txt.name, jTextname->valuestring);
    }
    cJSON * jPosX=cJSON_GetObjectItem(jRoot,"PosX");
    //设置位置
    if(!jPosX)
    {
        txt.x = SECSCREEN_TXT_POSX;
    }
    else
    {
        txt.x = jPosX->valueint;
    }
    cJSON * jPosY=cJSON_GetObjectItem(jRoot,"PosY");
    //设置位置
    if(!jPosY)
    {
        txt.y = SECSCREEN_TXT_POSY;
    }
    else
    {
        txt.y = jPosY->valueint;
    }
    //设置字体大小
    cJSON * jFontSize=cJSON_GetObjectItem(jRoot,"FontSize");
    if(!jFontSize)
    {
        load_font(SECSCREEN_FONT, SECSCREEN_FONTSIZE);
    }
    else
    {
        load_font(SECSCREEN_FONT, jFontSize->valueint);
    }
    cJSON_Delete(jRoot);
    add_text_to_dest(&txt);
    show_pic();
    return NULL;
}

//获取版本号
char * fn_dealCmd_GetVersion(char *recv_buf,int recv_len,int * ret_len)
{
    char strTemp[128];
    sprintf(strTemp,"{\"secscreen_version\":\"%s\"}",SECSCREENVERSION);
    //json串错误
    char * strRet = malloc(strlen(strTemp)+1);
    memset(strRet,0,strlen(strTemp)+1);
    memcpy(strRet,strTemp,strlen(strTemp));
    *ret_len = strlen(strTemp)+1;
    //返回
    myPtf("ret_len=%d,strRet=%s\n",*ret_len,strRet);
    return strRet;
}

//初始化
int fn_dxfb_init()
{
	//设置背景图
	if (init_dxfb(SECSCREEN_BG_PIC) < 0)
	{
		return -1;
	}
	//设置颜色
	if (primary->SetColor(primary, 0xFF, 0xFF, 0xFF, 0xFF) != DFB_OK) {
		pr_debug("Set color failed\n");
		return -2;
	}
	return 0;
}

//释放
void fn_dxfb_destory()
{
    exit_dxfb();
}
