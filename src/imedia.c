#include "imedia.h"

/*
enum
{
    PROP_0,
    PROP_XX,
    N_PROPERTIES
};
*/

typedef struct _IpcamIMediaPrivate
{
    gchar *xx;
} IpcamIMediaPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamIMedia, ipcam_imedia, IPCAM_BASE_APP_TYPE)

//static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void ipcam_imedia_before(IpcamIMedia *imedia);
static void ipcam_imedia_in_loop(IpcamIMedia *imedia);

static void ipcam_imedia_finalize(GObject *object)
{
    G_OBJECT_CLASS(ipcam_imedia_parent_class)->finalize(object);
}
static void ipcam_imedia_init(IpcamIMedia *self)
{
	IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(self);
    priv->xx = NULL;
}
/*
static void ipcam_imedia_get_property(GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
    IpcamIMedia *self = IPCAM_IMEDIA(object);
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(self);
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
static void ipcam_imedia_set_property(GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
    IpcamIMedia *self = IPCAM_IMEDIA(object);
    IpcamIMediaPrivate *priv = ipcam_imedia_get_instance_private(self);
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
static void ipcam_imedia_class_init(IpcamIMediaClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = &ipcam_imedia_finalize;
/*
    object_class->get_property = &ipcam_imedia_get_property;
    object_class->set_property = &ipcam_imedia_set_property;

    obj_properties[PROP_XX] =
        g_param_spec_string("xx",
                            "xxx",
                            "xxx.",
                            NULL, // default value
                            G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
*/
    IpcamBaseServiceClass *base_service_class = IPCAM_BASE_SERVICE_CLASS(klass);
    base_service_class->before = &ipcam_imedia_before;
    base_service_class->in_loop = &ipcam_imedia_in_loop;
}
static void ipcam_imedia_before(IpcamIMedia *imedia)
{
}
static void ipcam_imedia_in_loop(IpcamIMedia *imedia)
{
}
