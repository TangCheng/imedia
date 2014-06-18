#include "media_sys_ctrl_interface.h"

G_DEFINE_INTERFACE(IpcamIMediaSysCtrl, ipcam_imedia_sys_ctrl, 0);

static void
ipcam_imedia_sys_ctrl(gpointer g_class)
{
    
}

void ipcam_imedia_sys_ctrl_init(IpcamIMediaSysCtrl *self)
{
    g_return_if_fail(IPCAM_IS_IMEDIA_SYS_CTRL(self));
    IPCAM_IMEDIA_GET_INTERFACE(self)->init(self);
}
