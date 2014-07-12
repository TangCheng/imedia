#include "imedia.h"
#include "media_sys_ctrl_interface.h"
#include "media_video_interface.h"
#ifdef HI3518
#include "hi3518/media_sys_ctrl.h"
#include "hi3518/media_video.h"
#endif

typedef struct _IpcamIMediaPrivate
{
    IpcamIMediaSysCtrl *sys_ctrl;
    IpcamIVideo *video;
} IpcamIMediaPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamIMedia, ipcam_imedia, IPCAM_BASE_APP_TYPE)

static void ipcam_imedia_before(IpcamIMedia *imedia);
static void ipcam_imedia_in_loop(IpcamIMedia *imedia);

static void ipcam_imedia_finalize(GObject *object)
{
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(IPCAM_IMEDIA(object));
    g_clear_object(&priv->sys_ctrl);
    g_clear_object(&priv->video);
    G_OBJECT_CLASS(ipcam_imedia_parent_class)->finalize(object);
}
static void ipcam_imedia_init(IpcamIMedia *self)
{
	IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(self);
    priv->sys_ctrl = g_object_new(IPCAM_MEDIA_SYS_CTRL_TYPE, NULL);
    priv->video = g_object_new(IPCAM_MEDIA_VIDEO_TYPE, NULL);
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
    ipcam_imedia_sys_ctrl_init(priv->sys_ctrl);
    ipcam_ivideo_start(priv->video);
}
static void ipcam_imedia_in_loop(IpcamIMedia *imedia)
{
}
