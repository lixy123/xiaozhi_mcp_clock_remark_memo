#include <Arduino.h>

extern bool apstate;  //是否AP配网模式
extern String ssid;   //wifi账号
extern String pwd;    //wifi密码
extern String mcpEndpoint; //小智URL
extern String baidu_api_key;  //百度秘钥
extern String baidu_secret_key; //百度秘钥
extern String reset_pin;  //重置参数引脚，必须有
extern String led_pin;    //AP模式下LED亮灯状态， -1 表示不用
extern String alarm1_en; //闹钟1
extern String alarm1_time; //
extern String alarm1_msg; //
extern String alarm2_en; //闹钟2
extern String alarm2_time; //
extern String alarm2_msg; //
extern String i2s_out_bclk;
extern String i2s_out_lrc;
extern String i2s_out_dout; 

void ap_serve();
void ap_init();
