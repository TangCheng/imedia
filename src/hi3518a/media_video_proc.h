#ifndef __MEDIA_VIDEO_PROC_H__
#define __MEDIA_VIDEO_PROC_H__

#include <glib.h>
#include <glib-object.h>

#define IPCAM_MEDIA_VIDEO_PROC_TYPE (ipcam_media_video_proc_get_type())
#define IPCAM_MEDIA_VIDEO_PROC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IPCAM_MEDIA_VIDEO_PROC_TYPE, IpcamMediaVideoProc))
#define IPCAM_MEDIA_VIDEO_PROC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), IPCAM_MEDIA_VIDEO_PROC_TYPE, IpcamMediaVideoProcClass))
#define IPCAM_IS_MEDIA_VIDEO_PROC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), IPCAM_MEDIA_VIDEO_PROC_TYPE))
#define IPCAM_IS_MEDIA_VIDEO_PROC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), IPCAM_MEDIA_VIDEO_PROC_TYPE))
#define IPCAM_MEDIA_VIDEO_PROC_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), IPCAM_MEDIA_VIDEO_PROC_TYPE, IpcamMediaVideoProcClass))

typedef struct _IpcamMediaVideoProc IpcamMediaVideoProc;
typedef struct _IpcamMediaVideoProcClass IpcamMediaVideoProcClass;

struct _IpcamMediaVideoProc
{
    GObject parent;
};

struct _IpcamMediaVideoProcClass
{
    GObjectClass parent_class;
};

GType ipcam_media_video_proc_get_type(void);

#endif /* __MEDIA_VIDEO_PROC_H__ */
