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
