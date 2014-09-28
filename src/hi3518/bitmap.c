#include <hi_defines.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "stream_descriptor.h"

typedef struct _IpcamBitmapPrivate
{
    BITMAP_S data;
} IpcamBitmapPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(IpcamBitmap, ipcam_bitmap, G_TYPE_OBJECT)

static void ipcam_bitmap_finalize(GObject *object)
{
    IpcamBitmapPrivate *priv = ipcam_bitmap_get_instance_private(IPCAM_BITMAP(object));
    free(priv->data.pData);
    G_OBJECT_CLASS(ipcam_bitmap_parent_class)->finalize(object);
}
static void ipcam_bitmap_init(IpcamBitmap *self)
{
	IpcamBitmapPrivate *priv = ipcam_bitmap_get_instance_private(self);
    priv->data.enPixelFormat = PIXEL_FORMAT_RGB_1555;
    priv->data.u32Width = (IMAGE_WIDTH / 16 + 1) * 16;
    priv->data.u32Height = (IMAGE_HEIGHT / 16 + 1) * 16;
    priv->data.pData = malloc(2 * priv->data.u32Width * priv->data.u32Height);
    memset(priv->data.pData, 0, 2 * priv->data.u32Width * priv->data.u32Height);
}
static void ipcam_bitmap_class_init(IpcamBitmapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = &ipcam_bitmap_finalize;
}
BITMAP_S *ipcam_bitmap_get_data(IpcamBitmap *self)
{
    g_return_val_if_fail(IPCAM_IS_BITMAP(self), NULL);
    IpcamBitmapPrivate *priv = ipcam_bitmap_get_instance_private(self);
    return &priv->data;
}
void ipcam_bitmap_clear(IpcamBitmap *self, RECT_S *area)
{
    g_return_if_fail(IPCAM_IS_BITMAP(self));
    g_return_if_fail(area);

    IpcamBitmapPrivate *priv = ipcam_bitmap_get_instance_private(self);
    guint x = 0;
    guint y = 0;
    HI_U16 *u16Data;
    for (x = area->s32X; x < area->u32Width; x++)
    {
        for (y = area->s32Y; y < area->u32Height; y++)
        {
            u16Data = priv->data.pData + 2 * (x + y * area->u32Width);
            *u16Data = 0;
        }
    }
}
void ipcam_bitmap_bitblt(IpcamBitmap *self, BITMAP_S *src, POINT_S *pos)
{
    g_return_if_fail(IPCAM_IS_BITMAP(self));
    g_return_if_fail(src);
    g_return_if_fail(pos);

    IpcamBitmapPrivate *priv = ipcam_bitmap_get_instance_private(self);
    guint x = 0;
    guint y = 0;
    HI_U16 *u16Src;
    HI_U16 *u16Dst;
    for (x = 0; x < src->u32Width; x++)
    {
        for (y = 0; y < src->u32Height; y++)
        {
            u16Dst = priv->data.pData + 2 * ((x + pos->s32X) + (y + pos->s32Y) * priv->data.u32Width);
            u16Src = src->pData + 2 * (x + y * src->u32Width);
            *u16Dst = *u16Src;
        }
    }
}
