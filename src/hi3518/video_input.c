#include <hi_defines.h>
#include <hi_comm_vi.h>
#include <mpi_vi.h>
#include "video_input.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamVideoInputPrivate
{
    gchar *xx;
} IpcamVideoInputPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamVideoInput, ipcam_video_input, G_TYPE_OBJECT)
#define IMAGE_WIDTH          1280
#define IMAGE_HEIGHT         720
//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_video_input_init(IpcamVideoInput *self)
{
	IpcamVideoInputPrivate *priv = ipcam_video_input_get_instance_private(self);
    priv->xx = NULL;
}
/*
static void ipcam_video_input_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamVideoInput *self = IPCAM_VIDEO_INPUT(object);
    IpcamVideoInputPrivate *priv = ipcam_video_input_get_instance_private(self);
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
static void ipcam_video_input_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamVideoInput *self = IPCAM_VIDEO_INPUT(object);
    IpcamVideoInputPrivate *priv = ipcam_video_input_get_instance_private(self);
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
static void ipcam_video_input_class_init(IpcamVideoInputClass *klass)
{
/*
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = &ipcam_video_input_get_property;
    object_class->set_property = &ipcam_video_input_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
}

static HI_S32 ipcam_video_input_start(IpcamVideoInput *self)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;

    

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
            g_print("HI_MPI_VI_SetDevAttrEx [%d] failed with %#x!\n", ViDev, s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_EnableDev(ViDev);
        if (s32Ret != HI_SUCCESS)
        {
            g_print("HI_MPI_VI_EnableDev [%d] failed with %#x!\n", ViDev, s32Ret);
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
            g_print("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_EnableChn(ViChn);
        if (s32Ret != HI_SUCCESS)
        {
            g_print("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return s32Ret;
}
static HI_S32 ipcam_video_input_stop(IpcamVideoInput *self)
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
            g_print("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
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
            g_print("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}
