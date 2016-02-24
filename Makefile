
CC=gcc
CFLAGS=-Wall -g2 -O $(shell pkg-config --cflags xcb) -DHAVE_XCB=1
LDFLAGS=
LIBS=-lvulkan $(shell pkg-config --libs xcb) -lpthread -lm

OBJS=vulkanplay.o vkapi.o plat.o plat_xcb.o shader_vert_spv.o shader_frag_spv.o
SOURCES=$(patsubst %.o,%.c,$(OBJS))

LAYERS = VK_LAYER_LUNARG_param_checker \
	 VK_LAYER_LUNARG_object_tracker \
	 VK_LAYER_LUNARG_draw_state \
	 VK_LAYER_LUNARG_swapchain \
	 VK_LAYER_LUNARG_api_dump
	 #VK_LAYER_LUNARG_mem_tracker \

.PHONY: clean depend

vulkanplay: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

depend:
	rm .deps
	$(MAKE) .deps

.deps: $(SOURCES) Makefile
	$(CC) -MM -MP $(CFLAGS) $(SOURCES) > .deps

clean:
	-rm -f *.o vulkanplay *_spv.c

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%_vert.spv: %.vert
	glslangValidator -V $< -o $@

%_frag.spv: %.frag
	glslangValidator -V $< -o $@

%_spv.c: %.spv
	( echo "const " && xxd -i $< ) > $@

.PHONY: run
run: vulkanplay
	export VK_INSTANCE_LAYERS=$(subst $(eval) ,:,$(LAYERS)) ; \
	export VK_DEVICE_LAYERS=$(subst $(eval) ,:,$(LAYERS)) ; \
	./vulkanplay

-include .deps
