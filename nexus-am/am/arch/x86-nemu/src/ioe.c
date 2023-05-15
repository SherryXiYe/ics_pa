#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);        //从系统启动到IDE启动时经过的毫秒数
}

//返回经过的毫秒数
unsigned long _uptime() {
  unsigned long useTime_ms=inl(RTC_PORT) - boot_time;
  return useTime_ms;
}

uint32_t* const fb = (uint32_t *)0x40000;

//结构体_scree用于指示屏幕的大小
_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

//用于将 pixels 指定的矩形像素绘制到屏幕中以(x, y)和(x+w, y+h)两点连线为对角线的矩形区域
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  // int i;
  // for (i = 0; i < _screen.width * _screen.height; i++) {
  //   fb[i] = i;
  // }
  int temp=w;
  if(w>_screen.width-x){
    temp=_screen.width-x;
  }
  int cp_bytes = sizeof(uint32_t)*temp;
  for(int j=0; j<h && y+j<_screen.height;j++){
    memcpy(&fb[(y+j)*_screen.width+x],pixels,cp_bytes);
    pixels+=w;
  }
}

//用于将之前的绘制内容同步到屏幕上(在 NEMU 中绘制内容总是会同步到屏幕上,因而无需实现此 API)
void _draw_sync() {
}

// 用于返回按键的键盘码,若无按键,则返回_KEY_NONE
int _read_key() {
  if (inb(0x64)) 
    return inl(0x60);
  return _KEY_NONE;
}

void getScreen(int* width,int* height){
  *width=_screen.width;
  *height=_screen.height;
}