//#include <hi_comm_region.h>
#include <hi_type.h>
#include <hi_defines.h>
#include <hi_comm_sys.h>
#include <hi_comm_venc.h>
//#include <mpi_region.h>
#include <mpi_sys.h>
#include <mpi_venc.h>
#include <shm_queue.h>
#include <shm_rr_queue.h>
#include "stream_descriptor.h"
#include "media_video_interface.h"
#include "media_video.h"
#include "isp.h"
#include "video_input.h"
#include "video_encode.h"
#include "video_process_subsystem.h"

enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};

typedef struct _IpcamMediaVideoPrivate
{
    gchar *xx;
    IpcamShmRRQueue *video_pool;
    IpcamIsp *isp;
    IpcamVideoInput *vi;
    IpcamVideoProcessSubsystem *vpss;
    IpcamVideoEncode *venc;
    gboolean livestream_flag;
    GThread *livestream;
} IpcamMediaVideoPrivate;

static void ipcam_ivideo_interface_init(IpcamIVideoInterface *iface);
static gpointer ipcam_media_video_livestream(gpointer data);

G_DEFINE_TYPE_WITH_CODE(IpcamMediaVideo, ipcam_media_video, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(IPCAM_TYPE_IVIDEO,
                                              ipcam_ivideo_interface_init));

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_media_video_finalize(GObject *object)
{
    IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(IPCAM_MEDIA_VIDEO(object));
    ipcam_shm_rr_queue_close(priv->video_pool);
    g_clear_object(&priv->video_pool);
    g_clear_object(&priv->isp);
    g_clear_object(&priv->vi);
    g_clear_object(&priv->vpss);
    g_clear_object(&priv->venc);
    G_OBJECT_CLASS(ipcam_media_video_parent_class)->finalize(object);
}
static void ipcam_media_video_init(IpcamMediaVideo *self)
{
	IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(self);
    priv->xx = NULL;
    priv->video_pool = g_object_new(IPCAM_SHM_RR_QUEUE_TYPE,
                                    "block-num", 10,
                                    "pool-size", 1024 * 1024,
                                    "mode", OP_MODE_WRITE,
                                    "priority", WRITE_PRIO,
                                    NULL);
    ipcam_shm_rr_queue_open(priv->video_pool, "/data/configuration.sqlite3", 0);
    priv->isp = g_object_new(IPCAM_ISP_TYPE, NULL);
    priv->vi = g_object_new(IPCAM_VIDEO_INPUT_TYPE, NULL);
    priv->vpss = g_object_new(IPCAM_VIDEO_PROCESS_SUBSYSTEM_TYPE, NULL);
    priv->venc = g_object_new(IPCAM_VIDEO_ENCODE_TYPE, NULL);
    priv->livestream_flag = TRUE;
}
static void ipcam_media_video_get_property(GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
    IpcamMediaVideo *self = IPCAM_MEDIA_VIDEO(object);
    IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(self);
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
static void ipcam_media_video_set_property(GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
    IpcamMediaVideo *self = IPCAM_MEDIA_VIDEO(object);
    IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(self);
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
static void ipcam_media_video_class_init(IpcamMediaVideoClass *klass)
{
    g_type_class_add_private(klass, sizeof(IpcamMediaVideoPrivate));
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = &ipcam_media_video_finalize;
    object_class->get_property = &ipcam_media_video_get_property;
    object_class->set_property = &ipcam_media_video_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}
static gint32 ipcam_media_video_vpss_bind_vi(IpcamMediaVideo *self)
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
        g_critical("%s: HI_MPI_SYS_Bind failed with %#x!\n", __FUNCTION__, s32Ret);
    }

    return s32Ret;
}
static gint32 ipcam_media_video_vpss_unbind_vi(IpcamMediaVideo *self)
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

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        g_critical("%s: HI_MPI_SYS_UnBind failed with %#x!\n", __FUNCTION__, s32Ret);
    }

    return s32Ret;
}
static gint32 ipcam_media_video_venc_bind_vpss(IpcamMediaVideo *self)
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
        g_critical("%s: HI_MPI_SYS_Bind failed with %#x!\n", __FUNCTION__, s32Ret);
    }

    return s32Ret;
}
static gint32 ipcam_media_video_venc_unbind_vpss(IpcamMediaVideo *self)
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

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        g_critical("%s: HI_MPI_SYS_UnBind failed with %#x!\n", __FUNCTION__, s32Ret);
    }

    return s32Ret;
}
static gint32 ipcam_media_video_start_livestream(IpcamMediaVideo *self)
{
    IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(self);

    ipcam_isp_start(priv->isp);
    ipcam_video_input_start(priv->vi);
    ipcam_video_process_subsystem_start(priv->vpss);
    ipcam_video_encode_start(priv->venc);

    ipcam_media_video_vpss_bind_vi(self);
    ipcam_media_video_venc_bind_vpss(self);

    priv->livestream_flag = TRUE;
    priv->livestream = g_thread_new("livestream",
                                    ipcam_media_video_livestream,
                                    priv);

    return HI_SUCCESS;
}
static gint32 ipcam_media_video_stop_livestream(IpcamMediaVideo *self)
{
    IpcamMediaVideoPrivate *priv = ipcam_media_video_get_instance_private(self);

    priv->livestream_flag = FALSE;
    g_thread_join(priv->livestream);
    
    ipcam_media_video_venc_unbind_vpss(self);
    ipcam_media_video_vpss_unbind_vi(self);

    ipcam_video_encode_stop(priv->venc);
    ipcam_video_process_subsystem_stop(priv->vpss);
    ipcam_video_input_stop(priv->vi);
    ipcam_isp_stop(priv->isp);

    return HI_SUCCESS;
}
static gpointer ipcam_media_video_livestream(gpointer data)
{
    volatile IpcamMediaVideoPrivate *priv = (IpcamMediaVideoPrivate *)data;
    HI_S32 i = 0;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd;
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    
    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    /* Set Venc Fd. */
    VencFd = HI_MPI_VENC_GetFd(0);
    if (VencFd < 0)
    {
        g_critical("HI_MPI_VENC_GetFd failed with %#x!\n", VencFd);
    }
    if (maxfd <= VencFd)
    {
        maxfd = VencFd;
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (TRUE == priv->livestream_flag)
    {
        FD_ZERO(&read_fds);
        FD_SET(VencFd, &read_fds);

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            g_critical("select failed with %#x!\n", s32Ret);
            break;
        }
        else if (s32Ret == 0)
        {
            //g_warning("get venc stream time out, continue!\n");
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
                    g_print("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", 0, s32Ret);
                    break;
                }

                /*******************************************************
                 step 2.2 : malloc corresponding number of pack nodes.
                *******************************************************/
                stStream.pstPack = g_new(VENC_PACK_S, stStat.u32CurPacks);
                if (NULL == stStream.pstPack)
                {
                    g_critical("malloc stream pack failed!\n");
                    break;
                }
                    
                /*******************************************************
                 step 2.3 : call mpi to get one-frame stream
                *******************************************************/
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = HI_MPI_VENC_GetStream(0, &stStream, HI_TRUE);
                if (HI_SUCCESS != s32Ret)
                {
                    g_free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    g_critical("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                    break;
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
                            g_print("pu8Addr[1][5] is %#x\n", p[4]);
                        }
                    }
                }
                if (newFrameSize > 0)
                {
                    //gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g.,
                                                              //from an encoder - then use that instead.
                    VideoStreamData *vsd = (VideoStreamData *)g_malloc(sizeof(VideoStreamData) + newFrameSize);
                    vsd->pts.tv_sec = stStream.pstPack[0].u64PTS / 1000000;
                    vsd->pts.tv_usec = stStream.pstPack[0].u64PTS % 1000000;
                    // Deliver the data here:
                    vsd->len = newFrameSize;
                    // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer),
                    // then set "fDurationInMicroseconds" here.
                    HI_U64 pos = 0;
                    HI_U64 left = 0;
                    for (i = 0; i < stStream.u32PackCount; i++)
                    {
                        left = (vsd->len - pos);
                        p = stStream.pstPack[i].pu8Addr[0];
                        if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
                        {
                            left = MIN(left, stStream.pstPack[i].u32Len[0] - 4);
                            left = left < (stStream.pstPack[i].u32Len[0] - 4) ? left : (stStream.pstPack[i].u32Len[0] - 4);
                            memcpy(vsd->data + pos, p + 4, left);
                        }
                        else
                        {
                            left = MIN(left, stStream.pstPack[i].u32Len[0]);
                            memcpy(vsd->data + pos, p, left);
                        }
                        pos += left;
                        if (pos >= vsd->len) break;
                        if (stStream.pstPack[i].u32Len[1] > 0)
                        {
                            left = (vsd->len - pos);
                            p = stStream.pstPack[i].pu8Addr[1];
                            if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01)
                            {
                                left = MIN(left, stStream.pstPack[i].u32Len[1] - 4);
                                memcpy(vsd->data + pos, p + 4, left);
                            }
                            else
                            {
                                left = MIN(left, stStream.pstPack[i].u32Len[1]);
                                memcpy(vsd->data + pos, p, left);
                            }
                            pos += left;
                            if (pos >= vsd->len) break;
                        }
                    }
                    ipcam_shm_rr_queue_write(priv->video_pool, vsd, sizeof(VideoStreamData) + newFrameSize);
                    g_free(vsd);
                }
                
                /*******************************************************
                 step 2.5 : release stream
                *******************************************************/
                s32Ret = HI_MPI_VENC_ReleaseStream(0, &stStream);
                if (HI_SUCCESS != s32Ret)
                {
                    g_free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    break;
                }
                /*******************************************************
                 step 2.6 : free pack nodes
                *******************************************************/
                g_free(stStream.pstPack);
                stStream.pstPack = NULL;
            }
        }
    }

    /*******************************************************
     * step 3 : close save-file
     *******************************************************/
    g_thread_exit(0);
    return NULL;
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
        g_print("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
        return;
    }

    /*******************************************************
                 step 2.2 : malloc corresponding number of pack nodes.
    *******************************************************/
    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    if (NULL == stStream.pstPack)
    {
        g_print("malloc stream pack failed!\n");
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
        g_print("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
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
static void ipcam_ivideo_interface_init(IpcamIVideoInterface *iface)
{
    iface->start = ipcam_media_video_start_livestream;
    iface->stop = ipcam_media_video_stop_livestream;
}
