#include <hi_type.h>
#include <hi_defines.h>
#include <hi_comm_region.h>
#include <mpi_region.h>
#include <stdlib.h>
#include <string.h>
#include "osd_font/osd_font.h"
#include "media_osd_interface.h"
#include "media_osd.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamMediaOsdPrivate
{
    IpcamOsdFont *osd_font;
} IpcamMediaOsdPrivate;

static void ipcam_iosd_interface_init(IpcamIOSDInterface *iface);

G_DEFINE_TYPE_WITH_CODE(IpcamMediaOsd, ipcam_media_osd, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(IPCAM_TYPE_IOSD,
                                              ipcam_iosd_interface_init));

//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_media_osd_init(IpcamMediaOsd *self)
{
}
/*
static void ipcam_media_osd_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamMediaOsd *self = IPCAM_MEDIA_OSD(object);
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    switch(property_id)
    {
    case PROP_XX:
        {
            g_value_set_string(value, priv->xx);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}
static void ipcam_media_osd_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamMediaOsd *self = IPCAM_MEDIA_OSD(object);
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    switch(property_id)
    {
    case PROP_XX:
        {
            g_free(priv->xx);
            priv->xx = g_value_dup_string(value);
        }
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}
*/
static void ipcam_media_osd_class_init(IpcamMediaOsdClass *klass)
{
    g_type_class_add_private(klass, sizeof(IpcamMediaOsdPrivate));
/*
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_media_osd_get_property;
    object_class->set_property = &ipcam_media_osd_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
}
#define START_POINT_X_OFFSET 16
#define START_POINT_Y_OFFSET 16
static gint32 ipcam_media_osd_start(IpcamMediaOsd *self)
{
    HI_S32 s32Ret = HI_FAILURE;
    RGN_HANDLE RgnHandle;
    VENC_GRP VencGrp;
    RGN_ATTR_S stRgnAttr;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);

    priv->osd_font = ipcam_osd_font_new();
    g_object_ref(priv->osd_font);

    stRgnAttr.enType = OVERLAY_RGN;
    stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
    stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 512;
    stRgnAttr.unAttr.stOverlay.stSize.u32Height = 32;
    stRgnAttr.unAttr.stOverlay.u32BgColor = 0x7FFF;

    RgnHandle = 0;

    s32Ret = HI_MPI_RGN_Create(RgnHandle, &stRgnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        g_critical("HI_MPI_RGN_Create (%d) failed with %#x!\n", RgnHandle, s32Ret);
        return s32Ret;
    }

    VencGrp = 0;
    stChn.enModId = HI_ID_GROUP;
    stChn.s32DevId = VencGrp;
    stChn.s32ChnId = 0;

    memset(&stChnAttr,0,sizeof(stChnAttr));
    stChnAttr.bShow = HI_TRUE;
    stChnAttr.enType = OVERLAY_RGN;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = START_POINT_X_OFFSET;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = START_POINT_Y_OFFSET;
    stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
    stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
    stChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;

    stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
    stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 48;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_TRUE;

    s32Ret = HI_MPI_RGN_AttachToChn(RgnHandle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        g_critical("HI_MPI_RGN_AttachToChn (%d) failed with %#x!\n", RgnHandle, s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}
static gint32 ipcam_media_osd_set_content(IpcamMediaOsd *self, const gchar *content)
{
    HI_S32 s32Ret = HI_FAILURE;
    gboolean bRet = FALSE;
    RGN_HANDLE RgnHandle;
    BITMAP_S stBitmap;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);
    g_return_val_if_fail((NULL != priv->osd_font), s32Ret);

    RgnHandle = 0;
    stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
    bRet = ipcam_osd_font_render_text(priv->osd_font,
                                      content,
                                      &stBitmap.pData,
                                      &stBitmap.u32Width,
                                      &stBitmap.u32Height);
    if (bRet)
    {
        s32Ret = HI_MPI_RGN_SetBitMap(RgnHandle, &stBitmap);
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
    RGN_HANDLE RgnHandle;
    VENC_GRP VencGrp;
    MPP_CHN_S stChn;
    IpcamMediaOsdPrivate *priv = IPCAM_MEDIA_OSD_GET_PRIVATE(self);

    RgnHandle = 0;
    VencGrp = 0;
    stChn.enModId = HI_ID_GROUP;
    stChn.s32DevId = VencGrp;
    stChn.s32ChnId = 0;

    s32Ret = HI_MPI_RGN_DetachFrmChn(RgnHandle, &stChn);
    if(HI_SUCCESS != s32Ret)
    {
        g_critical("HI_MPI_RGN_DetachFrmChn (%d) failed with %#x!\n", RgnHandle, s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_RGN_Destroy(RgnHandle);
    if (HI_SUCCESS != s32Ret)
    {
        g_critical("HI_MPI_RGN_Destroy [%d] failed with %#x\n", RgnHandle, s32Ret);
    }

    g_object_unref(priv->osd_font);
    
    return s32Ret;
}
static void ipcam_iosd_interface_init(IpcamIOSDInterface *iface)
{
    iface->start = ipcam_media_osd_start;
    iface->set_content = ipcam_media_osd_set_content;
    iface->stop = ipcam_media_osd_stop;
}
