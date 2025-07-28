PKG_CONFIG?=pkg-config
WAYLAND_PROTOCOLS=$(shell $(PKG_CONFIG) --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell $(PKG_CONFIG) --variable=wayland_scanner wayland-scanner)

PKGS="wlroots-0.18" wayland-server xkbcommon

CFLAGS+=$(shell $(PKG_CONFIG) --cflags $(PKGS))
CFLAGS+=--target=x86_64-linux-android30

LIBS=$(shell $(PKG_CONFIG) --libs $(PKGS))
LIBS+=-lsync -lnativewindow -lcutils -landroid
# wayland-scanner is a tool which generates C headers and rigging for Wayland
# protocols, which are specified in XML. wlroots requires you to rig these up
# to your build system yourself and provide them in the include path.
xdg-shell-protocol.h:
	$(WAYLAND_SCANNER) server-header \
		$(WAYLAND_PROTOCOLS)/stable/xdg-shell/xdg-shell.xml $@

tinywl.o: tinywl.c xdg-shell-protocol.h
	$(CC) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

buffer_utils.o: buffer_utils.c
	$(CC) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

ahb_swapchain.o: ahb_swapchain.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<


buffer_manager.o: buffer_manager.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

dmabuf.o: dmabuf.c
	$(CC) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

libtinywl.so: tinywl.o buffer_utils.o dmabuf.o buffer_manager.o ahb_swapchain.o
	$(CC) $^ -g -Werror $(CFLAGS) $(LDFLAGS) $(LIBS) -fPIC -shared -o $@

clean:
	rm -f libtinywl.so tinywl.o buffer_utils.o dmabuf.o buffer_manager.o ahb_swapchain.o xdg-shell-protocol.h

.DEFAULT_GOAL=libtinywl.so
.PHONY: clean
