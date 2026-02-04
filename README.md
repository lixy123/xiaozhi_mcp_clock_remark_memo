功能：
        在ESP32运行的MCP服务，给小智扩展计时提醒，闹钟提醒，备忘录三大功能。

一.硬件说明
1.ESP32， ESP32S3 . >=4MB Flash， 无需PSRAM
小智运行的硬件也是ESP32S3,ESP32C3等芯片。本程序是烧录在另一个ESP32或ESP32S3芯片上运行的，不要弄混淆了。

二.源码说明
1.源码环境：
A.开发软件: 
arduino: 1.8.19

B. lib库及版本：
ESP32 2.0.15 https://espressif.github.io/arduino-esp32/package_esp32_index.json
WebSocketMCP 1.0.0 https://github.com/toddpan/xiaozhi-esp32-mcp  
LinkedList 1.3.3 https://github.com/ivanseidel/LinkedList
NTPClient 3.2.0  https://github.com/arduino-libraries/NTPClient
RTClib 2.1.4  https://github.com/adafruit/RTClib
ArduinoJson 7.3.1 https://github.com/bblanchon/ArduinoJson.git
UrlEncode   1.0.1   https://github.com/plageoj/urlencode

2.烧录
方式1: 源码烧录
A.打开arduino, 打开源码，导入上面的lib库，如果版本不一致，有一定机率编译有问题，
B.试编译.
C.插入ESP32或ESP32S3等
D.选择端口
E.点击烧录按钮。 (注：有些esp32板需要先按boot+reset两键进入烧录模式，有些可不用)
F.烧录完后重新上电 (注：有些板烧录完毕能自动重启，不必重新上电)

方式2:  固件烧录 （无需准备源码环境，最省事的方法)
A.下载 flash_download_tool软件
B.下载编译好的固件: target.bin
C. target.bin烧录到地址 0x0
注： flash_download_tool 使用技巧参考：
https://my.feishu.cn/wiki/Zpz4wXBtdimBrLk25WdcXzxcnNS

三. 配置
上电后有两种需要配置相关参数：
>>>小智 后台配置：
1.登陆小智 网站https://xiaozhi.me/
2.配置角色介绍
  进入 >控制台>对应的智能体>配置角色>
  在角色介绍中加入如下文字
  
 当用户提到几小时几分钟后提醒我做某事时，我会调用remind_me_time_later的MCP服务。
当用户要求查询提醒时，我会调用remind_me_read的MCP服务。
当用户要求删除提醒时，我会调用remind_me_delete的MCP服务。

当用户提到设置闹钟时，我会调用alarm_clock_set的MCP服务。
当用户要求查询闹钟时，我会调用alarm_clock_read的MCP服务。
当用户要求删除闹钟时，我会调用alarm_clock_delete的MCP服务。

当用户提到帮我记一下时，我会调用memo_set的MCP服务。
当用户提到查询备忘录时，我会调用memo_read的MCP服务。
当用户提到删除备忘录时，我会调用memo_delete的MCP服务。
当用户提到清空备忘录时，我会调用memo_clear的MCP服务。
当用户提到有几条备忘录时，我会调用memo_length的MCP服务。
当用户提到修改备忘录时，我会调用memo_modify的MCP服务。

  注：配置完需要重启小智生效。以后不要重新查看MCP地址，得新进入后MCP地址就会变化，需要重新配置ESP32
3.拷出MCP地址
  进入>控制台>对应的智能体>配置角色>MCP设置>获取MCP接入点
  拷出MCP接入点URL地址,下一步要用.

>>>ESP32配置：
1. 首次运行，ESP32会创建一个AP.
2.用手机或电脑连接此AP
3.浏览器输入网址 http://192.168.4.1
4.填入配置参数

参数说明：
WIFI_SSID    所连接的路由器 AP账号
WIFI_PASS   所连接的路由器 AP密码
mcpEndpoint  上一步得到的小智MCP地址
baidu_api_key        百度云的TTS语音服务KEY, 如果第一次接触，可能需要约1小时左右申请完成
baidu_secret_key
i2s_out_bclk         ESP32开发板连接数字功放（MAX98357A）的3个引脚, 配置错误则无法发出语音
i2s_out_lrc
i2s_out_dout
led_pin              ESP32 LED灯引脚，AP配置模式会亮灯
reset_pin            ESP32 重置参数引脚，按住超5秒，下次上电后进入AP设置模式
闹钟1                每天固定时间点的闹钟，到点会语音提醒，格式 **:** ， 例 07:10 ，每日7点10分语音提醒
闹钟2
注：以上配置最费时间的是百度云参数获得.

四.使用
1.唤醒小智后，对着小智说:"3分钟后提醒我去喝水"。 此时运行在ESP32的MCP服务记住信息，到点会发出语音：去喝水。
2.对着小智说:"提醒我明天10点半开会"。  此时运行在ESP32的MCP服务记住信息，到点会发出语音：去开会。
3.对着小智说:"帮我记一下，找张三谈件事"。 此时运行在ESP32的MCP服务记住备忘录信息。
4.对着小智说:"查询备忘录"。 小智会读出上一步MCP服务记忆的信息.

五.后续改进计划
1.只实现了一个最简化的闹钟，只能一条，且不能循环。可增强。
2.只实现了一个最简化的定时器，只能一条，可增强
注：功能越复杂，和小智语音交互就越复杂。功能简单反而使用简单。


