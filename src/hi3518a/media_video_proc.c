#include <hi_comm_isp.h>
#include <hi_comm_vi.h>
#include <hi_comm_vo.h>
#include <hi_comm_venc.h>
#include <hi_comm_vpss.h>
#include <hi_comm_vdec.h>
#include <hi_comm_vda.h>
#include <hi_comm_region.h>
#include <hi_comm_adec.h>
#include <hi_comm_aenc.h>
#include <hi_comm_ai.h>
#include <hi_comm_ao.h>
#include <hi_comm_aio.h>
#include <hi_comm_isp.h>
#include <hi_defines.h>

#include <mpi_sys.h>
#include <mpi_vb.h>
#include <mpi_vi.h>
#include <mpi_vo.h>
#include <mpi_venc.h>
#include <mpi_vpss.h>
#include <mpi_vdec.h>
#include <mpi_vda.h>
#include <mpi_region.h>
#include <mpi_adec.h>
#include <mpi_aenc.h>
#include <mpi_ai.h>
#include <mpi_ao.h>
#include <mpi_isp.h>
#include <mpi_ae.h>
#include <mpi_awb.h>
#include <mpi_af.h>
#include <hi_sns_ctrl.h>
#include <shm_queue.h>
#include <shm_rr_queue.h>
#include "media_video_proc.h"

enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};

typedef struct _IpcamMediaVideoProcPrivate
{
    gchar *xx;
    IpcamShmRRQueue *video_pool;
	pthread_t gs_IspPid;
} IpcamMediaVideoProcPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamMediaVideoProc, ipcam_media_video_proc, G_TYPE_OBJECT)

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_media_video_proc_finalize(GObject *object)
{
    IpcamMediaVideoProcPrivate *priv = ipcam_media_video_proc_get_instance_private(self);
    g_clear_object(&priv->video_pool);
    G_OBJECT_CLASS(ipcam_media_video_proc_parent_class)->finalize(object);
}
static void ipcam_media_video_proc_init(IpcamMediaVideoProc *self)
{
	IpcamMediaVideoProcPrivate *priv = ipcam_media_video_proc_get_instance_private(self);
    priv->xx = NULL;
    priv->video_pool = g_object_new(IPCAM_SHM_RR_QUEUE_TYPE,
                                    "block-num", 10,
                                    "pool-size", 1024 * 1024,
                                    "mode", OP_MODE_WRITE,
                                    "priority", WRITE_PRIO,
                                    NULL);
    ipcam_shm_rr_queue_open(queue, "/data/configuration.sqlite3", 0);
}
static void ipcam_media_video_proc_get_property(GObject    *object,
                                                guint       property_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
    IpcamMediaVideoProc *self = IPCAM_MEDIA_VIDEO_PROC(object);
    IpcamMediaVideoProcPrivate *priv = ipcam_media_video_proc_get_instance_private(self);
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
static void ipcam_media_video_proc_set_property(GObject      *object,
                                                guint         property_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
    IpcamMediaVideoProc *self = IPCAM_MEDIA_VIDEO_PROC(object);
    IpcamMediaVideoProcPrivate *priv = ipcam_media_video_proc_get_instance_private(self);
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
static void ipcam_media_video_proc_class_init(IpcamMediaVideoProcClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_media_video_proc_get_property;
    object_class->set_property = &ipcam_media_video_proc_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}
HI_S32 ipcam_media_vidoe_proc_start_video_input_unit(IpcamMediaVideoProc *media_proc)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;

    /******************************************
     step 1: configure sensor.
     note: you can jump over this step, if you do not use Hi3518 interal isp. 
    ******************************************/
    s32Ret = sensor_register_callback();
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: sensor_register_callback failed with %#x!\n", \
               __FUNCTION__, s32Ret);
        return s32Ret;
    }
    sensor_init();

	ISP_IMAGE_ATTR_S stImageAttr;
    ISP_INPUT_TIMING_S stInputTiming;

    ALG_LIB_S stLib;

    /* 1. register ae lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 2. register awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 3. register af lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_AF_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 4. isp init */
    s32Ret = HI_MPI_ISP_Init();
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_ISP_Init failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 5. isp set image attributes */
    /* note : different sensor, different ISP_IMAGE_ATTR_S define.
              if the sensor you used is different, you can change
              ISP_IMAGE_ATTR_S definition */

            stImageAttr.enBayer      = BAYER_GRBG;
            stImageAttr.u16FrameRate = 30;
            stImageAttr.u16Width     = 1280;
            stImageAttr.u16Height    = 720;

    s32Ret = HI_MPI_ISP_SetImageAttr(&stImageAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_ISP_SetImageAttr failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    }

            stInputTiming.enWndMode = ISP_WIND_NONE;
    s32Ret = HI_MPI_ISP_SetInputTiming(&stInputTiming);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: HI_MPI_ISP_SetInputTiming failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    }

    if (0 != pthread_create(&gs_IspPid, 0, (void* (*)(void*))HI_MPI_ISP_Run, NULL))
    {
        printf("%s: create isp running thread failed!\n", __FUNCTION__);
        return HI_FAILURE;
    }

    /******************************************************
     step 3 : config & start vicap dev
    ******************************************************/
    for (i = 0; i < u32DevNum; i++)
    {
        ViDev = i;
        VI_DEV_ATTR_S stViDevAttr =
            {
                VI_MODE_DIGITAL_CAMERA,
                VI_WORK_MODE_1Multiplex,
                {0xFF000000, 0x00},
                VI_SCAN_PROGRESSIVE,
                {-1, -1, -1, -1},
                VI_INPUT_DATA_UYVY,
                {
                    VI_VSYNC_FIELD,
                    VI_VSYNC_NEG_HIGH,
                    VI_HSYNC_VALID_SINGNAL,
                    VI_HSYNC_NEG_HIGH,
                    VI_VSYNC_VALID_SINGAL,
                    VI_VSYNC_VALID_NEG_HIGH,
                    {
                        4,
                        IMAGE_WIDTH,
                        544,
                        4,
                        IMAGE_HEIGHT,
                        20,
                        0,
                        0,
                        0
                    }
                },
                VI_PATH_ISP,
                VI_DATA_TYPE_YUV,
                HI_FALSE
            };
        
        s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VI_SetDevAttrEx [%d] failed with %#x!\n", ViDev, s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_EnableDev(ViDev);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VI_EnableDev [%d] failed with %#x!\n", ViDev, s32Ret);
            return HI_FAILURE;
        }
    }
    
    /******************************************************
     * Step 4: config & start vicap chn (max 1) 
     ******************************************************/
    for (i = 0; i < u32ChnNum; i++)
    {
        ViChn = i;
        VI_CHN_ATTR_S stChnAttr =
            {
                {0, 0, IMAGE_WIDTH, IMAGE_HEIGHT},
                {IMAGE_WIDTH, IMAGE_HEIGHT},
                VI_CAPSEL_BOTH,
                PIXEL_FORMAT_YUV_SEMIPLANAR_422,
                HI_FALSE,
                HI_FALSE,
                HI_FALSE,
                -1,
                -1
            };

        s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_EnableChn(ViChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return s32Ret;
}
    
HI_S32 ipcam_media_vidoe_proc_stop_video_input_unit(IpcamMediaVideoProc *media_proc)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;

    /*** Stop VI Chn ***/
    for(i = 0; i < u32ChnNum; i++)
    {
        /* Stop vi phy-chn */
        ViChn = i;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
    }

    /*** Stop VI Dev ***/
    for(i = 0; i < u32DevNum; i++)
    {
        ViDev = i;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }

	HI_MPI_ISP_Exit();
    pthread_join(gs_IspPid, 0);

    return HI_SUCCESS;
}

HI_S32 ipcam_media_vidoe_proc_start_video_process_subsystem_unit(IpcamMediaVideoProc *media_proc)
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
        printf("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
        
    stVpssParam.u32MotionThresh = 0;
        
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
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
        printf("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    /*** start vpss group ***/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID ipcam_media_vidoe_proc_stop_video_process_subsystem_unit(IpcamMediaVideoProc *media_proc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return;
    }
    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return;
    }
    
    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
        return;
    }
}

HI_S32 ipcam_media_vidoe_proc_start_video_encode_unit(IpcamMediaVideoProc *media_proc)
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
        printf("HI_MPI_VENC_CreateGroup err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_CreateChn err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_RegisterChn(VeGroup, VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_RegisterChn err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_StartRecvPic err 0x%x\n",s32Ret);
        return HI_FAILURE;
    }
    // omit other code here.
    return HI_SUCCESS;
}

HI_S32 ipcam_media_vidoe_proc_stop_video_encode_unit(IpcamMediaVideoProc *media_proc)
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
        printf("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  UnRegist Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_UnRegisterChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 3:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 4:  Distroy Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyGroup(VeGrp);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",\
               VeGrp, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 ipcam_media_vidoe_proc_vpss_bind_vi(IpcamMediaVideoProc *media_proc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
    }

    return s32Ret;
}

HI_S32 ipcam_media_vidoe_proc_venc_bind_vpss(IpcamMediaVideoProc *media_proc)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("failed with %#x!\n", s32Ret);
    }

    return s32Ret;
}

#if 0
HI_VOID* ipcam_media_vidoe_proc_video_stream_proc(void *param)
{
    HI_S32 i;
    volatile ThreadParam *pThreadParam;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd;
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    
    pThreadParam = (ThreadParam *)param;
    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    /* Set Venc Fd. */
    VencFd = HI_MPI_VENC_GetFd(0);
    if (VencFd < 0)
    {
        printf("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd);
        return NULL;
    }
    if (maxfd <= VencFd)
    {
        maxfd = VencFd;
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == *(pThreadParam->pbThreadStart))
    {
        FD_ZERO(&read_fds);
        FD_SET(VencFd, &read_fds);

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            printf("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            printf("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            if (FD_ISSET(VencFd, &read_fds))
            {
                /*******************************************************
                 step 2.1 : query how many packs in one-frame stream.
                *******************************************************/
                memset(&stStream, 0, sizeof(stStream));
                s32Ret = HI_MPI_VENC_Query(0, &stStat);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                    break;
                }

                /*******************************************************
                 step 2.2 : malloc corresponding number of pack nodes.
                *******************************************************/
                stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                if (NULL == stStream.pstPack)
                {
                    printf("malloc stream pack failed!\n");
                    break;
                }
                    
                /*******************************************************
                 step 2.3 : call mpi to get one-frame stream
                *******************************************************/
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = HI_MPI_VENC_GetStream(0, &stStream, HI_TRUE);
                if (HI_SUCCESS != s32Ret)
                {
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    printf("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                    break;
                }

                /*******************************************************
                 step 2.4 : save frame to file
                *******************************************************/
                HI_U64 length = 0;
                for (i = 0; i < stStream.u32PackCount; i++)
                {
                    length += stStream.pstPack[i].u32Len[0];
                    length += stStream.pstPack[i].u32Len[1];
                }
                if (length > 0)
                {
                    DataPackage *data = (DataPackage *)malloc(sizeof(DataPackage));
                    data->len = length;
                    data->next = NULL;
                    data->addr = (HI_U8 *)malloc(length + 10);
                    data->pts = stStream.pstPack[0].u64PTS;
                    HI_U64 pos = 0;
                    for (i = 0; i < stStream.u32PackCount; i++)
                    {
                        memcpy(data->addr + pos, stStream.pstPack[i].pu8Addr[0], stStream.pstPack[i].u32Len[0]);
                        pos += stStream.pstPack[i].u32Len[0];
                        if (stStream.pstPack[i].u32Len[1] > 0)
                        {
                            memcpy(data->addr + pos, stStream.pstPack[i].pu8Addr[1], stStream.pstPack[i].u32Len[1]);
                            pos += stStream.pstPack[i].u32Len[1];
                        }
                    }
                    if (pthread_mutex_lock(&mtx) == 0)
                    {
                        DataPackage *p = &dataHead;
                        while (p->next)
                        {
                            p = p->next;
                        }
                        p->next = data;
                        pthread_mutex_unlock(&mtx);
                    }
                    fprintf(stderr, "0x%x\n", (HI_U32 *)data->addr);
                    pThreadParam->env->taskScheduler().triggerEvent(H264LiveStreamSource::eventTriggerId, pThreadParam->ourSource);
                }
                
                /*******************************************************
                 step 2.5 : release stream
                *******************************************************/
                s32Ret = HI_MPI_VENC_ReleaseStream(0, &stStream);
                if (HI_SUCCESS != s32Ret)
                {
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    break;
                }
                /*******************************************************
                 step 2.6 : free pack nodes
                *******************************************************/
                free(stStream.pstPack);
                stStream.pstPack = NULL;
            }
        }
    }

    /*******************************************************
     * step 3 : close save-file
     *******************************************************/
    return NULL;
}
#endif

HI_S32 ipcam_media_vidoe_proc_start_video_stream_proc(IpcamMediaVideoProc *media_proc)
{
    HI_S32 s32Ret;
    s32Ret = start_video_input_unit();
    if (HI_SUCCESS == s32Ret)
    {
        s32Ret = start_video_process_subsystem_unit();
        if (HI_SUCCESS == s32Ret)
        {
            vpss_bind_vi();
            s32Ret = start_video_encode_unit();
            if (HI_SUCCESS == s32Ret)
            {
                s32Ret = venc_bind_vpss();
                if (HI_SUCCESS == s32Ret)
                { 
                    //pthread_create(&vencPid, 0, video_stream_proc, (void *)&threadParam);
                    vencFd = HI_MPI_VENC_GetFd(0);
                    envir().taskScheduler().turnOnBackgroundReadHandling(vencFd,
                                                                         (TaskScheduler::BackgroundHandlerProc*)&deliverFrame0, this);
                    if (vencFd < 0)
                    {
                        printf("HI_MPI_VENC_GetFd failed with %#x!\n", vencFd);
                        return HI_FAILURE;
                    }
                }
            }
        }
        
    }
    return s32Ret;
}

HI_VOID ipcam_media_vidoe_proc_stop_video_stream_proc(IpcamMediaVideoProc *media_proc)
{
    envir().taskScheduler().turnOffBackgroundReadHandling(vencFd);
    stop_video_encode_unit();
    stop_video_process_subsystem_unit();
    stop_video_input_unit();

    return;
}
#if 0
{
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0;

    /*******************************************************
     step 2.1 : query how many packs in one-frame stream.
    *******************************************************/
    memset(&stStream, 0, sizeof(stStream));
    s32Ret = HI_MPI_VENC_Query(0, &stStat);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
        return;
    }

    /*******************************************************
                 step 2.2 : malloc corresponding number of pack nodes.
    *******************************************************/
    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    if (NULL == stStream.pstPack)
    {
        printf("malloc stream pack failed!\n");
        return;
    }
                    
    /*******************************************************
                 step 2.3 : call mpi to get one-frame stream
    *******************************************************/
    stStream.u32PackCount = stStat.u32CurPacks;
    s32Ret = HI_MPI_VENC_GetStream(0, &stStream, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        free(stStream.pstPack);
        stStream.pstPack = NULL;
        printf("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
        return;
    }

    /*******************************************************
                 step 2.4 : save frame to file
    *******************************************************/
    unsigned newFrameSize = 0; //%%% TO BE WRITTEN %%%
    HI_U8 *p = NULL;
    for (i = 0; i < stStream.u32PackCount; i++)
    {
        newFrameSize += (stStream.pstPack[i].u32Len[0]);
        p = stStream.pstPack[i].pu8Addr[0];
        if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
        {
            newFrameSize -= 4;
            //envir() << "packet " << i << " pu8Addr[0][5] is " << p[4] << " length is " << newFrameSize << "\n";
        }
        if (stStream.pstPack[i].u32Len[1] > 0)
        {
            newFrameSize += (stStream.pstPack[i].u32Len[1]);
            p = stStream.pstPack[i].pu8Addr[1];
            if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
            {
                newFrameSize -= 4;
                envir() << "pu8Addr[1][5] is " << p[4] << "\n";
            }
        }
    }
    if (newFrameSize > 0)
    {
        //gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
        fPresentationTime.tv_sec = stStream.pstPack[0].u64PTS / 1000000;
        fPresentationTime.tv_usec = stStream.pstPack[0].u64PTS % 1000000;
        // Deliver the data here:
        if (newFrameSize > fMaxSize) {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = newFrameSize - fMaxSize;
        } else {
            fFrameSize = newFrameSize;
        }
        // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
        HI_U64 pos = 0;
        HI_U64 left = 0;
        for (i = 0; i < stStream.u32PackCount; i++)
        {
            left = (fFrameSize - pos);
            p = stStream.pstPack[i].pu8Addr[0];
            if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
            {
                left = MIN(left, stStream.pstPack[i].u32Len[0] - 4);
                left = left < (stStream.pstPack[i].u32Len[0] - 4) ? left : (stStream.pstPack[i].u32Len[0] - 4);
                memcpy(fTo + pos, p + 4, left);
            }
            else
            {
                left = MIN(left, stStream.pstPack[i].u32Len[0]);
                memcpy(fTo + pos, p, left);
            }
            pos += left;
            if (pos >= fFrameSize) break;
            if (stStream.pstPack[i].u32Len[1] > 0)
            {
                left = (fFrameSize - pos);
                p = stStream.pstPack[i].pu8Addr[1];
                if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
                {
                    left = MIN(left, stStream.pstPack[i].u32Len[1] - 4);
                    memcpy(fTo + pos, p + 4, left);
                }
                else
                {
                    left = MIN(left, stStream.pstPack[i].u32Len[1]);
                    memcpy(fTo + pos, p, left);
                }
                pos += left;
                if (pos >= fFrameSize) break;
            }
        }
    }
                
    /*******************************************************
                 step 2.5 : release stream
    *******************************************************/
    s32Ret = HI_MPI_VENC_ReleaseStream(0, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        free(stStream.pstPack);
        stStream.pstPack = NULL;
        return;
    }
    /*******************************************************
                 step 2.6 : free pack nodes
    *******************************************************/
    free(stStream.pstPack);
    stStream.pstPack = NULL;

    // After delivering the data, inform the reader that it is now available:
    if (newFrameSize > 0)
        FramedSource::afterGetting(this);
}
#endif
