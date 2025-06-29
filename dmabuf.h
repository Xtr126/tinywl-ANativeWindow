#include <wlr/types/wlr_buffer.h>

struct wlr_dmabuf_buffer {
	struct wlr_buffer base;
	struct wlr_dmabuf_attributes dmabuf;
	bool saved;
};

/**
 * Wraps a DMA-BUF into a wlr_buffer. The DMA-BUF may be accessed until
 * dmabuf_buffer_drop() is called.
 */
struct wlr_dmabuf_buffer *dmabuf_buffer_create(
	struct wlr_dmabuf_attributes *dmabuf);
/**
 * Drops ownership of the buffer (see wlr_buffer_drop() for more details) and
 * takes a reference to the DMA-BUF (by dup'ing its file descriptors) if a
 * consumer still has the buffer locked.
 */
bool dmabuf_buffer_drop(struct wlr_dmabuf_buffer *buffer);

