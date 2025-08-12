PKG_CONFIG?=pkg-config
WAYLAND_PROTOCOLS=$(shell $(PKG_CONFIG) --variable=pkgdatadir wayland-protocols)
WAYLAND_SCANNER=$(shell $(PKG_CONFIG) --variable=wayland_scanner wayland-scanner)

PKGS="wlroots-0.18" wayland-server xkbcommon

CFLAGS+=$(shell $(PKG_CONFIG) --cflags $(PKGS))
CFLAGS+=--target=x86_64-linux-android33

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

input_service.o: TinywlInputService.cpp
	$(CXX) -g $(CFLAGS) -fPIC -I. -I./aidl_source_output_dir/debug/out -DWLR_USE_UNSTABLE -o $@ -c $<

buffer_presenter.o: buffer_presenter.cpp
	$(CXX) -g -Werror $(CFLAGS) -fPIC -I. -DWLR_USE_UNSTABLE -o $@ -c $<

find_cpp = $(firstword $(wildcard aidl_source_output_dir/**/$(basename $(notdir $1)).cpp))

%.o:
	$(CXX) -g $(CFLAGS) -fPIC -I. -I./aidl_source_output_dir/debug/out -DWLR_USE_UNSTABLE -o $@ -c  $(shell find aidl_source_output_dir -name "$(basename $(notdir $@)).cpp")

# Flatten to object file names (just base filename.o)
AIDL_OBJS := $(shell find aidl_source_output_dir -name '*.cpp' -exec basename {} .cpp \; | sed 's/$$/.o/')

libtinywl.so: tinywl.o buffer_utils.o buffer_presenter.o ahb_swapchain.o cros_gralloc_util.o input_service.o $(AIDL_OBJS)
	$(CC) $^ -g -Werror $(CFLAGS) $(LDFLAGS) $(LIBS) -fPIC -shared -o $@

clean:
	rm -f libtinywl.so tinywl.o buffer_utils.o buffer_presenter.o ahb_swapchain.o cros_gralloc_util.o input_service.o xdg-shell-protocol.h $(AIDL_OBJS)

.DEFAULT_GOAL=libtinywl.so
.PHONY: clean
