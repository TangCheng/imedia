#ifndef __MEDIA_OSD_INTERFACE_H__
#define __MEDIA_OSD_INTERFACE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define IPCAM_TYPE_IOSD (ipcam_iosd_get_type())
#define IPCAM_IOSD(obj) (G_TYPE_CHECK_INSTANCE_CASE((obj), IPCAM_TYPE_IOSD, IpcamIOSD))
#define IPCAM_IS_IOSD(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), IPCAM_TYPE_IOSD))
#define IPCAM_IOSD_GET_INTERFACE(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst), IPCAM_TYPE_IOSD, IpcamIOSDInterface))

typedef struct _IpcamIOSD IpcamIOSD;
typedef struct _IpcamIOSDInterface IpcamIOSDInterface;

struct _IpcamIOSDInterface
{
    GTypeInterface parent_iface;
    gint32 (*start)(IpcamIOSD *self);
    gint32 (*set_content)(IpcamIOSD *self, const gchar *content);
    gint32 (*stop)(IpcamIOSD *self);
};

GType ipcam_iosd_get_type(void);
gint32 ipcam_iosd_start(IpcamIOSD *self);
gint32 ipcam_iosd_set_content(IpcamIOSD *self, const gchar *content);
gint32 ipcam_iosd_stop(IpcamIOSD *self);

G_END_DECLS

#endif /* __MEDIA_OSD_INTERFACE_H__  */
