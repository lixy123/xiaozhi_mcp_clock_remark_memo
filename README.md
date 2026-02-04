功能：
   在ESP32S3运行的MCP服务，给小智扩展计时提醒，闹钟提醒，备忘录三大功能。

<b>一.硬件说明</b><br>
1.ESP32S3<br/>
  Flash大小>=4MB , 无需PSRAM <br/>
2.MAX98357A<br/>
MAX98357A   ==>ESP32S3引脚<br/>
I2S_OUT_BCLK     15<br/>
I2S_OUT_LRC      16<br/>
I2S_OUT_DOUT     7<br/>
3.喇叭<br/>
连接到MAX98357A<br/>
淘宝上用 "小智AI语音对话机器人 MINI ESP32-S3-N16R8开发板" 可查到. <br/>
<img src= 'https://raw.githubusercontent.com/lixy123/xiaozhi_mcp_clock_remark_memo/refs/heads/main/pic/2.jpg' /> <br/>

用ESP32， ESP32C3板也可运行。<br/>
小智AI所用硬件也是ESP32S3,ESP32C3等芯片。本程序是烧录在另一个ESP32S3芯片上运行的，不是烧录在小智所用硬件上的，不要弄混淆了。

<b>二.源码说明</b><br>
1.源码环境：<br>
A.开发软件: <br>
arduino: 1.8.19

B. lib库及版本：<br>
ESP32 2.0.15 https://espressif.github.io/arduino-esp32/package_esp32_index.json<br>
WebSocketMCP 1.0.0 https://github.com/toddpan/xiaozhi-esp32-mcp  <br>
LinkedList 1.3.3 https://github.com/ivanseidel/LinkedList<br>
NTPClient 3.2.0  https://github.com/arduino-libraries/NTPClient<br>
RTClib 2.1.4  https://github.com/adafruit/RTClib<br>
ArduinoJson 7.3.1 https://github.com/bblanchon/ArduinoJson.git<br>
UrlEncode   1.0.1   https://github.com/plageoj/urlencode<br>

2.烧录<br>
方式1: 源码烧录<br>
A.打开arduino, 打开源码，导入上面的lib库，如果版本不一致，有一定机率编译有问题，<br>
B.试编译.<br>
C.插入ESP32S3等<br>
D.选择端口<br>
E.点击烧录按钮。 (注：有些ESP32S3板需要先按boot+reset两键进入烧录模式，有些可不用)<br>
F.烧录完后重新上电 (注：有些板烧录完毕能自动重启，不必重新上电)<br>

方式2:  固件烧录 （无需准备源码环境，最省事的方法)<br>
A.下载 flash_download_tool软件<br>
B.下载编译好的固件: target.bin （已上传至：Releases) <br>
C. target.bin烧录到地址 0x0<br>
注： flash_download_tool 使用技巧参考：<br>
https://my.feishu.cn/wiki/Zpz4wXBtdimBrLk25WdcXzxcnNS<br>

编译好的固件烧录界面:<br/>
<img src= 'https://raw.githubusercontent.com/lixy123/xiaozhi_mcp_clock_remark_memo/refs/heads/main/pic/1.jpg' /> <br/>

<b>三. 配置</b><br>
上电后有两种需要配置相关参数：<br>
<b>>>>小智 后台配置：</b><br>
1.登陆小智 网站https://xiaozhi.me/<br>
2.配置角色介绍<br>
  进入 >控制台>对应的智能体>配置角色><br>
  在角色介绍中加入如下文字<br>
  
 当用户提到几小时几分钟后提醒我做某事时，我会调用remind_me_time_later的MCP服务。<br>
当用户要求查询提醒时，我会调用remind_me_read的MCP服务。<br>
当用户要求删除提醒时，我会调用remind_me_delete的MCP服务。<br>

当用户提到设置闹钟时，我会调用alarm_clock_set的MCP服务。<br>
当用户要求查询闹钟时，我会调用alarm_clock_read的MCP服务。<br>
当用户要求删除闹钟时，我会调用alarm_clock_delete的MCP服务。<br>

当用户提到帮我记一下时，我会调用memo_set的MCP服务。<br>
当用户提到查询备忘录时，我会调用memo_read的MCP服务。<br>
当用户提到删除备忘录时，我会调用memo_delete的MCP服务。<br>
当用户提到清空备忘录时，我会调用memo_clear的MCP服务。<br>
当用户提到有几条备忘录时，我会调用memo_length的MCP服务。<br>
当用户提到修改备忘录时，我会调用memo_modify的MCP服务。<br>

  注：配置完需要重启小智生效。以后不要重新查看MCP地址，得新进入后MCP地址就会变化，需要重新配置ESP32<br>
3.拷出MCP地址<br>
  进入>控制台>对应的智能体>配置角色>MCP设置>获取MCP接入点<br>
  拷出MCP接入点URL地址,下一步要用.<br>

<b>>>>ESP32配置：</b><br>
1. 首次运行，ESP32会创建一个AP.<br>
2.用手机或电脑连接此AP<br>
3.浏览器输入网址 http://192.168.4.1<br>
4.填入配置参数<br>

参数说明：<br>
WIFI_SSID    所连接的路由器 AP账号<br>
WIFI_PASS   所连接的路由器 AP密码<br>
mcpEndpoint  上一步得到的小智MCP地址<br>
baidu_api_key        百度云的TTS语音服务KEY, 如果第一次接触，可能需要约1小时左右申请完成<br>
baidu_secret_key<br>
i2s_out_bclk         ESP32开发板连接数字功放（MAX98357A）的3个引脚, 配置错误则无法发出语音<br>
i2s_out_lrc<br>
i2s_out_dout<br>
led_pin              ESP32 LED灯引脚，AP配置模式会亮灯<br>
reset_pin            ESP32 重置参数引脚，按住超5秒，下次上电后进入AP设置模式<br>
闹钟1                每天固定时间点的闹钟，到点会语音提醒，格式 **:** ， 例 07:10 ，每日7点10分语音提醒<br>
闹钟2<br>
注：以上配置最费时间的是百度云参数获得.<br>

<b>四.使用</b><br>
1.唤醒小智后，对着小智说:"3分钟后提醒我去喝水"。 此时运行在ESP32的MCP服务记住信息，到点会发出语音：去喝水。<br>
2.对着小智说:"提醒我明天10点半开会"。  此时运行在ESP32的MCP服务记住信息，到点会发出语音：去开会。<br>
3.对着小智说:"帮我记一下，找张三谈件事"。 此时运行在ESP32的MCP服务记住备忘录信息。<br>
4.对着小智说:"查询备忘录"。 小智会读出上一步MCP服务记忆的信息.<br>

<b>五.后续改进计划</b><br>
1.只实现了一个最简化的闹钟，只能一条，且不能循环。可增强。<br>
2.只实现了一个最简化的定时器，只能一条，可增强<br>
注：功能越复杂，和小智语音交互就越复杂。功能简单反而使用简单。<br>


