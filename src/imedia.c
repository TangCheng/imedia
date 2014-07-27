#include <time.h>
#include "imedia.h"
#include "media_sys_ctrl_interface.h"
#include "media_video_interface.h"
#include "media_osd_interface.h"
#ifdef HI3518
#include "hi3518/media_sys_ctrl.h"
#include "hi3518/media_video.h"
#include "hi3518/media_osd.h"
#endif

typedef struct _IpcamIMediaPrivate
{
    IpcamIMediaSysCtrl *sys_ctrl;
    IpcamIVideo *video;
    IpcamIOSD *osd;
    time_t last_time;
} IpcamIMediaPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamIMedia, ipcam_imedia, IPCAM_BASE_APP_TYPE)

static void ipcam_imedia_before(IpcamIMedia *imedia);
static void ipcam_imedia_in_loop(IpcamIMedia *imedia);

static void ipcam_imedia_finalize(GObject *object)
{
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(IPCAM_IMEDIA(object));
    g_clear_object(&priv->sys_ctrl);
    g_clear_object(&priv->video);
    g_clear_object(&priv->osd);
    G_OBJECT_CLASS(ipcam_imedia_parent_class)->finalize(object);
}
static void ipcam_imedia_init(IpcamIMedia *self)
{
	IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(self);
    priv->sys_ctrl = g_object_new(IPCAM_MEDIA_SYS_CTRL_TYPE, NULL);
    priv->video = g_object_new(IPCAM_MEDIA_VIDEO_TYPE, NULL);
    priv->osd = g_object_new(IPCAM_MEDIA_OSD_TYPE, NULL);
    time(&priv->last_time);
}
static void ipcam_imedia_class_init(IpcamIMediaClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = &ipcam_imedia_finalize;

    IpcamBaseServiceClass *base_service_class = IPCAM_BASE_SERVICE_CLASS(klass);
    base_service_class->before = &ipcam_imedia_before;
    base_service_class->in_loop = &ipcam_imedia_in_loop;
}
static void ipcam_imedia_before(IpcamIMedia *imedia)
{
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(imedia);
    IpcamOSDParameter parameters[IPCAM_OSD_TYPE_LAST];
    memset(parameters, 0, sizeof(IpcamOSDParameter) * IPCAM_OSD_TYPE_LAST);
    
    ipcam_imedia_sys_ctrl_init(priv->sys_ctrl);
    ipcam_ivideo_start(priv->video);
    ipcam_iosd_start(priv->osd, parameters);
}
static void ipcam_imedia_in_loop(IpcamIMedia *imedia)
{
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(imedia);
    time_t now;
    gchar timeBuf[24] = {0};

    time(&now);
    if (priv->last_time != now)
    {
        priv->last_time = now;
        strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S ", localtime(&now));
        ipcam_iosd_set_content(priv->osd, IPCAM_OSD_TYPE_DATETIME, timeBuf);
    }
}
