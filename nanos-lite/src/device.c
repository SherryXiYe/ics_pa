#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool down = false;
  if (key & 0x8000) {
    key &= 0xfff;
    down = true;
  }
  if (key!=_KEY_NONE) 
    sprintf(buf, "%s %s\n", down ? "kd" : "ku", keyname[key]);
  else
    sprintf(buf, "t %d\n", (uint32_t)_uptime());

  if(key==_KEY_F12 && down){    //按下F12，切换游戏
    extern void switch_current_game();
    switch_current_game();
    Log("switch current game");
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

extern void getScreen(int* width,int* height);

void fb_write(const void *buf, off_t offset, size_t len) {
  assert(offset%4==0&&len%4==0);    //像素以4B为单位
  int width=0,height=0;
  getScreen(&width,&height);

  int index,x1,y1,y2;
  index=offset/4;   //像素的索引
  x1=index%width;
  y1=index/width;   //行

  index=(offset+len)/4;
  y2=index/width;

  assert(y2>=y1);
  if(y2==y1){       //只在一行上绘制
    _draw_rect(buf,x1,y1,len/4,1);  //目标，左上角的x，y，长度，行数
    return;
  }
  int tempx=width-x1;
  if(y2-y1==1){       //在两行上绘制
    _draw_rect(buf,x1,y1,tempx,1);
    _draw_rect(buf+4*tempx,0,y2,len/4-tempx,1);
    return;
  }
  //三行以上
  _draw_rect(buf,x1,y1,tempx,1);
  int tempy=y2-y1-1;
  _draw_rect(buf+4*tempx,0,y1+1,width,tempy);
  _draw_rect(buf+4*tempx+4*width*tempy,0,y2,len/4-tempx-tempy*width,1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width=0,height=0;
  getScreen(&width,&height);
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d\n",width,height);
}
