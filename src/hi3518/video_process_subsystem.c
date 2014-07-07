#include <hi_defines.h>
#include <hi_comm_vpss.h>
#include <mpi_vpss.h>
#include "video_process_subsystem.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamVideoProcessSubsystemPrivate
{
    gchar *xx;
} IpcamVideoProcessSubsystemPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamVideoProcessSubsystem, ipcam_video_process_subsystem, G_TYPE_OBJECT)
#define IMAGE_WIDTH          1280
#define IMAGE_HEIGHT         720
//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_video_process_subsystem_init(IpcamVideoProcessSubsystem *self)
{
	IpcamVideoProcessSubsystemPrivate *priv = ipcam_video_process_subsystem_get_instance_private(self);
    priv->xx = NULL;
}
/*
static void ipcam_video_process_subsystem_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamVideoProcessSubsystem *self = IPCAM_VIDEO_PROCESS_SUBSYSTEM(object);
    IpcamVideoProcessSubsystemPrivate *priv = ipcam_video_process_subsystem_get_instance_private(self);
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
static void ipcam_video_process_subsystem_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamVideoProcessSubsystem *self = IPCAM_VIDEO_PROCESS_SUBSYSTEM(object);
    IpcamVideoProcessSubsystemPrivate *priv = ipcam_video_process_subsystem_get_instance_private(self);
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
static void ipcam_video_process_subsystem_class_init(IpcamVideoProcessSubsystemClass *klass)
{
/*
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_video_process_subsystem_get_property;
    object_class->set_property = &ipcam_video_process_subsystem_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
}

static HI_S32 ipcam_video_process_subsystem_start(IpcamVideoProcessSubsystem *self)
{
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN_ATTR_S stChnAttr;
    VPSS_GRP_PARAM_S stVpssParam;
    HI_S32 s32Ret;

    /*** Set Vpss Grp Attr ***/

    stGrpAttr.u32MaxW = IMAGE_WIDTH;
    stGrpAttr.u32MaxH = IMAGE_HEIGHT;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_FALSE;
    stGrpAttr.bNrEn = HI_FALSE;
    stGrpAttr.bHistEn = HI_FALSE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_422;

    /*** create vpss group ***/
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
        
    stVpssParam.u32MotionThresh = 0;
        
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /*** enable vpss chn, with frame ***/
    /* Set Vpss Chn attr */
    stChnAttr.bSpEn = HI_FALSE;
    stChnAttr.bFrameEn = HI_FALSE;
    stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_LEFT] = 0;
    stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_RIGHT] = 0;
    stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_BOTTOM] = 0;
    stChnAttr.stFrame.u32Color[VPSS_FRAME_WORK_TOP] = 0;
    stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_LEFT] = 0;
    stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_RIGHT] = 0;
    stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_TOP] = 0;
    stChnAttr.stFrame.u32Width[VPSS_FRAME_WORK_BOTTOM] = 0;
            
    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    /*** start vpss group ***/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
static HI_S32 ipcam_video_process_subsystem_stop(IpcamVideoProcessSubsystem *self)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}
