#include <hi_type.h>
#include <hi_defines.h>
#include <hi_comm_region.h>
#include <mpi_region.h>
#include <stdlib.h>
#include <string.h>
#include "osd_font/osd_font.h"
#include "media_osd_interface.h"
#include "media_osd.h"

typedef struct _IpcamMediaOsdPrivate
{
    IpcamOsdFont *osd_font;
    guint32 font_size[IPCAM_OSD_TYPE_LAST];
    Color color[IPCAM_OSD_TYPE_LAST];
    RGN_HANDLE RgnHandle[IPCAM_OSD_TYPE_LAST];
    VENC_GRP VencGrp;
    RGN_ATTR_S stRgnAttr[IPCAM_OSD_TYPE_LAST];
    RGN_CHN_ATTR_S stChnAttr[IPCAM_OSD_TYPE_LAST];
    gchar *content[IPCAM_OSD_TYPE_LAST];
} IpcamMediaOsdPrivate;

static void ipcam_iosd_interface_init(IpcamIOSDInterface *iface);
static gint32 ipcam_media_osd_set_content(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, const gchar *content);

G_DEFINE_TYPE_WITH_CODE(IpcamMediaOsd, ipcam_media_osd, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(IPCAM_TYPE_IOSD,
                                              ipcam_iosd_interface_init));

static void ipcam_media_osd_init(IpcamMediaOsd *self)
{
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    IPCAM_OSD_TYPE type;
    for (type = IPCAM_OSD_TYPE_DATETIME; type < IPCAM_OSD_TYPE_LAST; type++)
    {
        priv->RgnHandle[type] = type;
        priv->content[type] = NULL;
    }
    priv->VencGrp = 0;
}
static void ipcam_media_osd_class_init(IpcamMediaOsdClass *klass)
{
    g_type_class_add_private(klass, sizeof(IpcamMediaOsdPrivate));
}
static gint32 ipcam_media_osd_start(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, IpcamOSDParameter *parameter)
{
    HI_S32 s32Ret = HI_FAILURE;
    MPP_CHN_S stChn;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);

    priv->osd_font = ipcam_osd_font_new();
    g_object_ref(priv->osd_font);

    do
    {
        priv->font_size[type] = parameter->font_size;
        priv->color[type] = parameter->color;
        
        priv->stRgnAttr[type].enType = OVERLAY_RGN;
        priv->stRgnAttr[type].unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
        priv->stRgnAttr[type].unAttr.stOverlay.stSize.u32Width  = (parameter->font_size / 16 + 1) * 16 * 30;
        priv->stRgnAttr[type].unAttr.stOverlay.stSize.u32Height = (parameter->font_size / 16 + 1) * 16;
        priv->stRgnAttr[type].unAttr.stOverlay.u32BgColor = 0x7FFF;

        s32Ret = HI_MPI_RGN_Create(priv->RgnHandle[type], &priv->stRgnAttr[type]);
        if (HI_SUCCESS != s32Ret)
        {
            g_critical("HI_MPI_RGN_Create (%d) failed with %#x!\n", priv->RgnHandle[type], s32Ret);
            break;
        }

        stChn.enModId = HI_ID_GROUP;
        stChn.s32DevId = priv->VencGrp;
        stChn.s32ChnId = 0;

        memset(&priv->stChnAttr[type], 0, sizeof(priv->stChnAttr[type]));
        priv->stChnAttr[type].bShow = parameter->is_show;
        priv->stChnAttr[type].enType = OVERLAY_RGN;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stPoint.s32X = (parameter->position.x / 16 + 1) * 16;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stPoint.s32Y = (parameter->position.y / 16 + 1) * 16;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.u32BgAlpha = 0;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.u32FgAlpha = 64;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.u32Layer = 0;

        priv->stChnAttr[type].unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

        priv->stChnAttr[type].unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 48;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
        priv->stChnAttr[type].unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_TRUE;

        s32Ret = HI_MPI_RGN_AttachToChn(priv->RgnHandle[type], &stChn, &priv->stChnAttr[type]);
        if (HI_SUCCESS != s32Ret)
        {
            g_critical("HI_MPI_RGN_AttachToChn (%d) failed with %#x!\n", priv->RgnHandle[type], s32Ret);
            break;
        }
    } while (FALSE);
    
    return s32Ret;
}
static gint32 ipcam_media_osd_set_region_attr(IpcamMediaOsd *self, IPCAM_OSD_TYPE type)
{
    HI_S32 s32Ret = HI_FAILURE;
    MPP_CHN_S stChn;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    stChn.enModId = HI_ID_GROUP;
    stChn.s32DevId = priv->VencGrp;
    stChn.s32ChnId = 0;
    s32Ret = HI_MPI_RGN_SetDisplayAttr(priv->RgnHandle[type], &stChn, &priv->stChnAttr[type]);
    if (HI_SUCCESS != s32Ret)
    {
        g_critical("HI_MPI_RGN_SetDisplayAttr (%d) failed with %#x!\n", priv->RgnHandle[type], s32Ret);
    }
    return s32Ret;
}
static gint32 ipcam_media_osd_show(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, const gboolean show)
{
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    priv->stChnAttr[type].bShow = show;
    return ipcam_media_osd_set_region_attr(self, type);
}
static gint32 ipcam_media_osd_set_pos(IpcamMediaOsd *self, IPCAM_OSD_TYPE type,  const Point pos)
{
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    priv->stChnAttr[type].unChnAttr.stOverlayChn.stPoint.s32X = pos.x;
    priv->stChnAttr[type].unChnAttr.stOverlayChn.stPoint.s32Y = pos.y;
    return ipcam_media_osd_set_region_attr(self, type);
}
static gint32 ipcam_media_osd_set_fontsize(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, const guint fsize)
{
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    priv->font_size[type] = fsize;
    return ipcam_media_osd_set_content(self, type, priv->content[type]);
}
static gint32 ipcam_media_osd_set_color(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, const Color color)
{
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    priv->color[type] = color;
    return ipcam_media_osd_set_content(self, type, priv->content[type]);
}
static gint32 ipcam_media_osd_set_content(IpcamMediaOsd *self, IPCAM_OSD_TYPE type, const gchar *content)
{
    HI_S32 s32Ret = HI_FAILURE;
    gboolean bRet = FALSE;
    BITMAP_S stBitmap;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);

    if (priv->content[type] != content && type != IPCAM_OSD_TYPE_DATETIME)
    {
        g_free(priv->content[type]);
        priv->content[type] = g_strdup(content);
    }
        
    g_return_val_if_fail((NULL != priv->osd_font), s32Ret);
    g_return_val_if_fail((NULL != content), s32Ret);

    g_object_set(priv->osd_font,
                 "font-size", priv->font_size[type],
                 "font-color", priv->color[type].value,
                 NULL);
     
    stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
    bRet = ipcam_osd_font_render_text(priv->osd_font,
                                      content,
                                      &stBitmap.pData,
                                      &stBitmap.u32Width,
                                      &stBitmap.u32Height);
    if (bRet)
    {
        s32Ret = HI_MPI_RGN_SetBitMap(priv->RgnHandle[type], &stBitmap);
        if(s32Ret != HI_SUCCESS)
        {
            g_critical("HI_MPI_RGN_SetBitMap failed with %#x!\n", s32Ret);
        }
        g_free(stBitmap.pData);
    }
    
    return s32Ret;
}
static gint32 ipcam_media_osd_stop(IpcamMediaOsd *self)
{
    HI_S32 s32Ret = HI_FAILURE;
    MPP_CHN_S stChn;
    IPCAM_OSD_TYPE type;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);

    stChn.enModId = HI_ID_GROUP;
    stChn.s32DevId = priv->VencGrp;
    stChn.s32ChnId = 0;

    for (type = IPCAM_OSD_TYPE_DATETIME; type < IPCAM_OSD_TYPE_LAST; type++)
    {
        s32Ret = HI_MPI_RGN_DetachFrmChn(priv->RgnHandle[type], &stChn);
        if(HI_SUCCESS != s32Ret)
        {
            g_critical("HI_MPI_RGN_DetachFrmChn (%d) failed with %#x!\n", priv->RgnHandle[0], s32Ret);
        }
        s32Ret = HI_MPI_RGN_Destroy(priv->RgnHandle[type]);
        if (HI_SUCCESS != s32Ret)
        {
            g_critical("HI_MPI_RGN_Destroy [%d] failed with %#x\n", priv->RgnHandle[0], s32Ret);
        }
        g_free(priv->content[type]);
        priv->content[type] = NULL;
    }

    g_object_unref(priv->osd_font);
    
    return s32Ret;
}
static void ipcam_iosd_interface_init(IpcamIOSDInterface *iface)
{
    iface->start = ipcam_media_osd_start;
    iface->show = ipcam_media_osd_show;
    iface->set_pos = ipcam_media_osd_set_pos;
    iface->set_fontsize = ipcam_media_osd_set_fontsize;
    iface->set_color = ipcam_media_osd_set_color;
    iface->set_content = ipcam_media_osd_set_content;
    iface->stop = ipcam_media_osd_stop;
}
