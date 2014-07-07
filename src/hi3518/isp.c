#include <hi_defines.h>
#include <hi_comm_isp.h>
#include <mpi_isp.h>
#include <mpi_ae.h>
#include <mpi_awb.h>
#include <mpi_af.h>
#include <hi_sns_ctrl.h>
#include "isp.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamIspPrivate
{
    pthread_t IspPid;
} IpcamIspPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamIsp, ipcam_isp, G_TYPE_OBJECT)

//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_isp_init(IpcamIsp *self)
{
	IpcamIspPrivate *priv = ipcam_isp_get_instance_private(self);
}
/*
static void ipcam_isp_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamIsp *self = IPCAM_ISP(object);
    IpcamIspPrivate *priv = ipcam_isp_get_instance_private(self);
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
static void ipcam_isp_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamIsp *self = IPCAM_ISP(object);
    IpcamIspPrivate *priv = ipcam_isp_get_instance_private(self);
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
static void ipcam_isp_class_init(IpcamIspClass *klass)
{
/*
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_isp_get_property;
    object_class->set_property = &ipcam_isp_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
}
static HI_S32 ipcam_isp_start(IpcamIsp *self)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IpcamIspPrivate *priv = ipcam_isp_get_instance_private(self);
    
    /******************************************
     step 1: configure sensor.
     note: you can jump over this step, if you do not use Hi3518 interal isp. 
    ******************************************/
    s32Ret = sensor_register_callback();
    if (s32Ret != HI_SUCCESS)
    {
        g_print("%s: sensor_register_callback failed with %#x!\n", \
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
        g_print("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 2. register awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 3. register af lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_AF_Register(&stLib);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return s32Ret;
    }

    /* 4. isp init */
    s32Ret = HI_MPI_ISP_Init();
    if (s32Ret != HI_SUCCESS)
    {
        g_print("%s: HI_MPI_ISP_Init failed!\n", __FUNCTION__);
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
        g_print("%s: HI_MPI_ISP_SetImageAttr failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    }

    stInputTiming.enWndMode = ISP_WIND_NONE;
    s32Ret = HI_MPI_ISP_SetInputTiming(&stInputTiming);
    if (s32Ret != HI_SUCCESS)
    {
        g_print("%s: HI_MPI_ISP_SetInputTiming failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    }

    if (0 != pthread_create(&priv->IspPid, 0, (void* (*)(void*))HI_MPI_ISP_Run, NULL))
    {
        g_print("%s: create isp running thread failed!\n", __FUNCTION__);
        return HI_FAILURE;
    }
}
static void ipcam_isp_stop(IpcamIsp *self)
{
    IpcamIspPrivate *priv = ipcam_isp_get_instance_private(self);
    HI_MPI_ISP_Exit();
    pthread_join(priv->IspPid, 0);
}
