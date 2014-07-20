#include "media_osd_interface.h"

G_DEFINE_INTERFACE(IpcamIOSD, ipcam_iosd, 0);

static void
ipcam_iosd_default_init(IpcamIOSDInterface *iface)
{

}

gint32 ipcam_iosd_start(IpcamIOSD *self)
{
    g_return_val_if_fail(IPCAM_IS_IOSD(self), FALSE);
    return IPCAM_IOSD_GET_INTERFACE(self)->start(self);
}

gint32 ipcam_iosd_set_content(IpcamIOSD *self, const gchar *content)
{
    g_return_val_if_fail(IPCAM_IS_IOSD(self), FALSE);
    return IPCAM_IOSD_GET_INTERFACE(self)->set_content(self, content);
}

gint32 ipcam_iosd_stop(IpcamIOSD *self)
{
    g_return_val_if_fail(IPCAM_IS_IOSD(self), FALSE);
    return IPCAM_IOSD_GET_INTERFACE(self)->start(self);
}
