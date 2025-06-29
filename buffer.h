#include <android/hardware_buffer.h>
#include <android/native_window.h>
#include <drm_fourcc.h>
#include <wlr/render/dmabuf.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>

uint32_t android_to_drm_format(uint32_t android_format);

bool AHardwareBuffer_getDmabufAttributes(AHardwareBuffer *ahb, 
                           struct wlr_dmabuf_attributes *attribs);


/* timeout in msecs */
int sync_wait(int fd, int timeout);

void ANativeWindow_render_scene(ANativeWindow *window, struct wlr_output *output, struct wlr_scene_output * scene_output);