PKG_CONFIG?=pkg-config
WAYLAND_PROTOCOLS=$(shell $(PKG_CONFIG) --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell $(PKG_CONFIG) --variable=wayland_scanner wayland-scanner)

PKGS="wlroots-0.18" wayland-server xkbcommon

CFLAGS+=$(shell $(PKG_CONFIG) --cflags $(PKGS))
CFLAGS+=--target=x86_64-linux-android35

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

buffer_utils.o: buffer_utils.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

ahb_swapchain.o: ahb_swapchain.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

cros_gralloc_util.o: cros_gralloc_util.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<


buffer_presenter.o: buffer_presenter.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

libtinywl.so: tinywl.o buffer_utils.o buffer_presenter.o ahb_swapchain.o cros_gralloc_util.o
	$(CC) $^ -g -Werror $(CFLAGS) $(LDFLAGS) $(LIBS) -fPIC -shared -o $@

clean:
	rm -f libtinywl.so tinywl.o buffer_utils.o buffer_presenter.o ahb_swapchain.o cros_gralloc_util.o xdg-shell-protocol.h

.DEFAULT_GOAL=libtinywl.so
.PHONY: clean
