AM_SRCS := native/trm.c \
           native/ioe.c \
           native/cte.c \
           native/trap.S \
           native/vme.c \
           native/mpe.c \
           native/platform.c \
           native/ioe/input.c \
           native/ioe/timer.c \
           native/ioe/gpu.c \
           native/ioe/audio.c \
           native/ioe/disk.c \

CFLAGS  += -fpie
CFLAGS  +=  -Og -ggdb3
# CFLAGS += $(if $(CONFIG_CC_DEBUG),-Og -ggdb3,)
# CFLAGS += $(if $(CONFIG_CC_ASAN),-fsanitize=address,)

ASFLAGS += -fpie -pie
comma = ,
LDFLAGS_CXX = $(addprefix -Wl$(comma), $(LDFLAGS))

image:
	@echo + LD "->" $(IMAGE_REL)
	@g++ -pie -o $(IMAGE) -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX) -lSDL2 -ldl

run: image
	$(IMAGE)

gdb: image
	# gdb $(IMAGE)
	gdb -ex "handle SIGUSR1 SIGUSR2 SIGSEGV noprint nostop" $(IMAGE)
