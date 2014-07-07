#include <hi_defines.h>
#include <hi_comm_venc.h>
#include <mpi_venc.h>
#include "video_encode.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamVideoEncodePrivate
{
    gchar *xx;
} IpcamVideoEncodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamVideoEncode, ipcam_video_encode, G_TYPE_OBJECT)

//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };
#define IMAGE_WIDTH          1280
#define IMAGE_HEIGHT         720

static void ipcam_video_encode_init(IpcamVideoEncode *self)
{
	IpcamVideoEncodePrivate *priv = ipcam_video_encode_get_instance_private(self);
    priv->xx = NULL;
}
/*
static void ipcam_video_encode_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamVideoEncode *self = IPCAM_VIDEO_ENCODE(object);
    IpcamVideoEncodePrivate *priv = ipcam_video_encode_get_instance_private(self);
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
static void ipcam_video_encode_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamVideoEncode *self = IPCAM_VIDEO_ENCODE(object);
    IpcamVideoEncodePrivate *priv = ipcam_video_encode_get_instance_private(self);
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
static void ipcam_video_encode_class_init(IpcamVideoEncodeClass *klass)
{
/*
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_video_encode_get_property;
    object_class->set_property = &ipcam_video_encode_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
}

static HI_S32 ipcam_video_encode_start(IpcamVideoEncode *self)
{
    HI_S32 s32Ret;
    VENC_GRP VeGroup = 0;
    VENC_CHN VeChn = 0;
    VENC_CHN_ATTR_S stAttr;
    /* set h264 chnnel video encode attribute */
    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));
    stAttr.stVeAttr.enType = PT_H264;
    stAttr.stVeAttr.stAttrH264e.u32PicWidth = IMAGE_WIDTH;
    stAttr.stVeAttr.stAttrH264e.u32PicHeight = IMAGE_HEIGHT;
    stAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = IMAGE_WIDTH;
    stAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = IMAGE_HEIGHT;
    stAttr.stVeAttr.stAttrH264e.u32Profile = 0;
    stAttr.stVeAttr.stAttrH264e.u32BufSize  = IMAGE_WIDTH * IMAGE_HEIGHT * 2;/*stream buffer size*/
    stAttr.stVeAttr.stAttrH264e.u32Profile  = 0;/*0: baseline; 1:MP; 2:HP   ? */
    stAttr.stVeAttr.stAttrH264e.bByFrame = HI_FALSE;/*get stream mode is slice mode or frame mode?*/
    stAttr.stVeAttr.stAttrH264e.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
    stAttr.stVeAttr.stAttrH264e.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
    stAttr.stVeAttr.stAttrH264e.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
    stAttr.stVeAttr.stAttrH264e.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
    // omit other video encode assignments here.
    /* set h264 chnnel rate control attribute */
    stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR ;
    stAttr.stRcAttr.stAttrH264Cbr.u32BitRate = 2 * 1024;
    stAttr.stRcAttr.stAttrH264Cbr.fr32TargetFrmRate = 30;
    stAttr.stRcAttr.stAttrH264Cbr.u32ViFrmRate = 30;
    stAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
    stAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    stAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 1;
    // omit other rate control assignments here.
    s32Ret = HI_MPI_VENC_CreateGroup(VeGroup);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_CreateGroup err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_CreateChn err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_RegisterChn(VeGroup, VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_RegisterChn err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("HI_MPI_VENC_StartRecvPic err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    // omit other code here.
    return HI_SUCCESS;
}
static HI_S32 ipcam_video_encode_stop(IpcamVideoEncode *self)
{
    HI_S32 s32Ret;
    VENC_GRP VeGrp = 0;
    VENC_CHN VeChn = 0;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  UnRegist Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_UnRegisterChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 3:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 4:  Distroy Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyGroup(VeGrp);
    if (HI_SUCCESS != s32Ret)
    {
        g_print("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",\
               VeGrp, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

