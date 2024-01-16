#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
int width = 0, height = 0;

void __am_gpu_init() {
  // int i;
  height = inl(VGACTL_ADDR) & 0xFFFF;
  width = inl(VGACTL_ADDR) >> 16;
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < width * height; i ++) {
    // fb[i] = i;
  // }
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;
  int w = ctl->w, h = ctl->h;
  int x_t = 0, y_t = 0, cnt = 0;
  if(w == 0 || h == 0) return;

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pixels = ctl->pixels;
  // 从xy为最低点rici
  for(int i = 0; i < h; i ++) {
    for(int j = 0; j < w; j ++) {
      x_t = x + j;
      y_t = y + i;
      fb[y_t * width + x_t] = pixels[cnt++];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
