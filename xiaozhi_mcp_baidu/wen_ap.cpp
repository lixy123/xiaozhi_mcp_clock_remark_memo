#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h> //dns 当启用dns,在win11查看连接AP时，会出现《打开浏览器并连接》
#include <ESPmDNS.h>   //内部域名 (需和dnsserver结合用， 用处不大！)
#include <ESPAsyncWebServer.h>
#include "web_page.h"
#include "wen_ap.h"

//https://github.com/ESP32Async/ESPAsyncWebServer/wiki
bool apstate = true;  //配置网络参数的AP模式

String param_flag = ""; //是否配置过参数
String ssid = ""; //wifi名称
String pwd = "";  //WiFi密码
String mcpEndpoint = "";  //小智mcp地址
String baidu_api_key = "";  //百度秘钥
String baidu_secret_key = "";  //百度秘钥
String reset_pin = "";  //重置参数引脚，必须有
String led_pin = "";  //AP模式下LED亮灯状态， -1 表示不用
String alarm1_en = ""; //启用
String alarm1_time = ""; //时间
String alarm1_msg = ""; //发声
String alarm2_en = ""; //
String alarm2_time = ""; //
String alarm2_msg = ""; //
String i2s_out_bclk;
String i2s_out_lrc;
String i2s_out_dout;

Preferences preferences;
AsyncWebServer web(80);
DNSServer dnsServer;

#define AP_SSID "ESP32-"  //名称
#define AP_PASSWORD ""  //密码

IPAddress apIP(192, 168, 4, 1); //路由地址

//esp32复位信号
bool root_now = false;
long root_time ;

void set_led(bool high_low);
void pin_reset();
void www_configPage(AsyncWebServerRequest * request);
String param_to_html(String html, String info_str);

//led指示灯 开/关, 显示状态表示是否处于AP模式
void set_led(bool high_low)
{
  if (led_pin == "-1")
    return;
  else
    digitalWrite(led_pin.toInt(), high_low);
}


// web服务
void ap_serve() {
  // 判断 是否打开监听 AP 模式
  if (apstate) {
    dnsServer.processNextRequest(); //监听并处理dns请求

    if (root_now)
    {
      //收到信号3秒后重启esp32
      if ((millis() - root_time) > 3000)
      {
        //连接成功，重启设备
        Serial.println("重启设备");
        Serial.flush();
        ESP.restart();              //重启复位esp32
        Serial.println("已重启设备.");
      }
    }
  }
  else
    pin_reset();
}



String param_to_html(String html, String info_str)
{
  html.replace("<<span>>", info_str);
  html.replace("<<ssid>>", ssid);
  html.replace("<<pwd>>", pwd);
  html.replace("<<mcpEndpoint>>", mcpEndpoint);
  html.replace("<<baidu_api_key>>", baidu_api_key);
  html.replace("<<baidu_secret_key>>", baidu_secret_key);
  html.replace("<<reset_pin>>", reset_pin);
  html.replace("<<led_pin>>", led_pin);

  html.replace("<<alarm1_en>>", alarm1_en);
  html.replace("<<alarm1_time>>", alarm1_time);
  html.replace("<<alarm1_msg>>", alarm1_msg);

  html.replace("<<alarm2_en>>", alarm2_en);
  html.replace("<<alarm2_time>>", alarm2_time);
  html.replace("<<alarm2_msg>>", alarm2_msg);

  html.replace("<<i2s_out_bclk>>", i2s_out_bclk);
  html.replace("<<i2s_out_lrc>>", i2s_out_lrc);
  html.replace("<<i2s_out_dout>>", i2s_out_dout);
  return html;
}

//  首页回调函数
void www_configPage(AsyncWebServerRequest * request) {
  //Serial.println("www_configPage");
  String html = configPage;
  html= param_to_html(html, "");
  request->send(200,  "text/html", html );
}


//  保存WiFi信息
void www_save(AsyncWebServerRequest * request) {
  Serial.println(">>> www_save");

  //注意：
  //AsyncWebServerRequest控件只认name,会跳过ID!!!
  //网页控件要用 name, 而不是id,否则params(), hasParam()函数获取不到任何值!!!

  //List all parameters
  /*
  int params = request->params();
  for (int i = 0; i < params; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) {
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
  */

  //当post,能获取到参数
  /*
    if(request->hasArg("ssid"))
    {
    String arg1 = request->arg("ssid");
    }
  */


  //注意：post方式下，hasParam,getParam函数必须加参数true,否则获取不到值
  //如果用post方式下 hasArg, arg方式不需要多加参数
  if (request->hasParam("ssid", true)) {
    ssid = request->getParam("ssid", true)->value();
  }

  if (request->hasParam("pwd", true)) {
    pwd = request->getParam("pwd", true)->value();
  }

  if (request->hasParam("mcpEndpoint", true)) {
    mcpEndpoint = request->getParam("mcpEndpoint", true)->value();
  }


  if (request->hasParam("baidu_api_key", true)) {
    baidu_api_key = request->getParam("baidu_api_key", true)->value();
  }

  if (request->hasParam("baidu_secret_key", true)) {
    baidu_secret_key = request->getParam("baidu_secret_key", true)->value();
  }

  if (request->hasParam("reset_pin", true)) {
    reset_pin = request->getParam("reset_pin", true)->value();
  }

  if (request->hasParam("led_pin", true)) {
    led_pin = request->getParam("led_pin", true)->value();
  }

  //如果不选中，hasParam无值
  //如果选中，  hasParam有值，getParam返回 on，存入：checked
  alarm1_en = "";
  if (request->hasParam("alarm1_en", true)) {
    alarm1_en = "checked";
  }

  if (request->hasParam("alarm1_time", true)) {
    alarm1_time = request->getParam("alarm1_time", true)->value();
  }

  if (request->hasParam("alarm1_msg", true)) {
    alarm1_msg = request->getParam("alarm1_msg", true)->value();
  }

  alarm2_en = "";
  if (request->hasParam("alarm2_en", true)) {
    alarm2_en = "checked";
  }

  if (request->hasParam("alarm2_time", true)) {
    alarm2_time = request->getParam("alarm2_time", true)->value();
  }

  if (request->hasParam("alarm2_msg", true)) {
    alarm2_msg = request->getParam("alarm2_msg", true)->value();
  }

  if (request->hasParam("i2s_out_bclk", true)) {
    i2s_out_bclk = request->getParam("i2s_out_bclk", true)->value();
  }

  if (request->hasParam("i2s_out_lrc", true)) {
    i2s_out_lrc = request->getParam("i2s_out_lrc", true)->value();
  }

  if (request->hasParam("i2s_out_dout", true)) {
    i2s_out_dout = request->getParam("i2s_out_dout", true)->value();
  }
  Serial.println("传入参数:");
  Serial.println("ssid=<" + ssid + ">");
  Serial.println("pwd=<" + pwd + ">");
  Serial.println("mcpEndpoint=<" + mcpEndpoint + ">");
  Serial.println("baidu_api_key=<" + baidu_api_key + ">");
  Serial.println("baidu_secret_key=<" + baidu_secret_key + ">");
  Serial.println("reset_pin=<" + reset_pin + ">");
  Serial.println("led_pin=<" + led_pin + ">");

  Serial.println("alarm1_en=<" + alarm1_en + ">");
  Serial.println("alarm1_time=<" + alarm1_time + ">");
  Serial.println("alarm1_msg=<" + alarm1_msg + ">");
  Serial.println("alarm2_en=<" + alarm2_en + ">");
  Serial.println("alarm2_time=<" + alarm2_time + ">");
  Serial.println("alarm2_msg=<" + alarm2_msg + ">");
  Serial.println("i2s_out_bclk=<" + i2s_out_bclk + ">");
  Serial.println("i2s_out_lrc=<" + i2s_out_lrc + ">");
  Serial.println("i2s_out_dout=<" + i2s_out_dout + ">");
  /*
    //如果用post方式，因为反馈时间过长，网页会失去连接，必须用ajax方式！
    //连接AP
    WiFi.begin(param_ssid, param_pwd);

    //15秒
    for (size_t i = 0; i < 15; i++) {

      //如果连接上AP
      if (WiFi.status() == WL_CONNECTED) {
        //连接成功，修改
        ssid =  param_ssid ;
        pwd = param_pwd;

        // 保存WiFi信息
        preferences.begin("wen", false); // 读写模式
        preferences.putString("ssid", ssid);
        preferences.putString("pwd", pwd);
        preferences.end();


        set_led(HIGH);

        String html = configPage;
        html.replace("<<span>>", " <span style=\"color:blue\">密码连接成功!</span>");
        html.replace("<<ssid>>", param_ssid);
        request->send(200,  "text/html", html );

        //返回保存成功页面
        //  request->send(200, "text/html", "<meta charset='UTF-8'>SSID：" + param_ssid + "<br />password:" + param_pwd +
        //               "<br /> 密码连接成功, 请手动关闭此页面。");

        //root_time = millis();
        //root_now = true;

        return;
      } else {
        delay(1000);
      }
    }

    //返回保存成功页面
    //request->send(200, "text/html", "<meta charset='UTF-8'>SSID：" + param_ssid + "<br />password:" + param_pwd +
    //             "<br /> 密码连接失败，请手动关闭此页面。");
    String html = configPage;
    html.replace("<<span>>", " <span style=\"color:blue\">密码连接失败!</span>");
    html.replace("<<ssid>>", param_ssid);
    request->send(200,  "text/html", html );
  */

  // 保存WiFi信息
  preferences.begin("wen", false); // 读写模式
  preferences.putString("ssid", ssid);
  preferences.putString("pwd", pwd);
  preferences.putString("mcpEndpoint", mcpEndpoint);
  preferences.putString("api_key", baidu_api_key);
  preferences.putString("secret_key", baidu_secret_key);
  preferences.putString("reset_pin", reset_pin);
  preferences.putString("led_pin", led_pin);

  preferences.putString("alarm1_en", alarm1_en);
  preferences.putString("alarm1_time", alarm1_time);
  preferences.putString("alarm1_msg", alarm1_msg);

  preferences.putString("alarm2_en", alarm2_en);
  preferences.putString("alarm2_time", alarm2_time);
  preferences.putString("alarm2_msg", alarm2_msg);

  preferences.putString("i2s_out_bclk", i2s_out_bclk);
  preferences.putString("i2s_out_lrc", i2s_out_lrc);
  preferences.putString("i2s_out_dout", i2s_out_dout);

  preferences.putString("param_flag", "1");
  preferences.end();

  String html = configPage;
  html= param_to_html(html, "<span style=\"color:blue\">参数写入完成！ESP32将重启!</span>" );
  request->send(200,  "text/html", html );

  //如果立即重启，会显示不出网页刷新提示!
  root_time = millis();
  root_now = true;
}

/*
  void onPageNotFounds(AsyncWebServerRequest * request) {
  Serial.println("onPageNotFounds");
  request->send(404, "text/plain", "Not found");
  }
*/

//  web服务初始化
void ap_web_init() {

  // 给本机设置内部DNS域名
  //AP模式才用得着,
  //浏览器输入 http://esp32 或 http://esp32.local均能路入主页
  //ping esp32 或 ping esp32.local 均相当于地址  192.168.4.1
  //给设备设定域名esp32,完整的域名是esp32.local
  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS服务启动。");
  }

  web.on("/", HTTP_GET, www_configPage);
  web.on("/save", HTTP_POST, www_save);
  //不管是什么网页，一率视为主页，这样就能对带参数的跳转页起作用！
  //例如：
  //http://www.msftconnecttest.com/redirect
  web.onNotFound(www_configPage);
  //web.onNotFound(onPageNotFounds);

  // Start web server
  web.begin();

  //在本机IP启用DNS服务，53端口
  dnsServer.start((byte)53, "*", apIP);
}


void pin_reset()
{
  //清空记忆连接并重启配网模式
  if (reset_pin != "-1")
  {
    if (!digitalRead(reset_pin.toInt())) //长按5秒清除网络配置信息
    {
      delay(5000);
      if (!digitalRead(reset_pin.toInt()))
      {
        Serial.println("\n按键已长按5秒,正在清空网络连保存接信息.");

        // 保存WiFi信息
        preferences.begin("wen", false); // 读写模式
        preferences.remove("param_flag");
        preferences.end();

        Serial.println("重启设备");
        Serial.flush();
        ESP.restart();              //重启复位esp32
        //下句,串口是收不到的
        Serial.println("已重启设备.");
      }
    }
  }
}

// 配网初始化
void ap_init() {

  // 判断是否存在了WiFi信息，存在则直接连接
  Serial.println("read params: ");
  preferences.begin("wen", true); // 只读模式

  //KEY:最大长度为 15 个字符
  param_flag = preferences.getString("param_flag");
  ssid = preferences.getString("ssid");
  pwd = preferences.getString("pwd");
  mcpEndpoint = preferences.getString("mcpEndpoint");

  //默认
  baidu_api_key = preferences.getString("api_key");
  baidu_secret_key = preferences.getString("secret_key");

  //默认：40引脚
  reset_pin = preferences.getString("reset_pin", "40");
  //默认：没有LED
  led_pin = preferences.getString("led_pin", "-1");

  //默认引脚
  i2s_out_bclk = preferences.getString("i2s_out_bclk", "15");
  i2s_out_lrc = preferences.getString("i2s_out_lrc", "16");
  i2s_out_dout = preferences.getString("i2s_out_dout", "7");

  alarm1_en = preferences.getString("alarm1_en");
  alarm1_time = preferences.getString("alarm1_time");
  alarm1_msg = preferences.getString("alarm1_msg");

  alarm2_en = preferences.getString("alarm2_en");
  alarm2_time = preferences.getString("alarm2_time");
  alarm2_msg = preferences.getString("alarm2_msg");

  param_flag = preferences.getString("param_flag");

  Serial.println("ssid: " + ssid);
  Serial.println("pwd: " + pwd);
  Serial.println("mcpEndpoint: " + mcpEndpoint);
  Serial.println("baidu_api_key: " + baidu_api_key);
  Serial.println("baidu_secret_key: " + baidu_secret_key);
  Serial.println("reset_pin: " + reset_pin);
  Serial.println("led_pin: " + led_pin);

  Serial.println("i2s_out_bclk: " + i2s_out_bclk);
  Serial.println("i2s_out_lrc: " + i2s_out_lrc);
  Serial.println("i2s_out_dout: " + i2s_out_dout);

  Serial.println("alarm1_en: " + alarm1_en);
  Serial.println("alarm1_time: " + alarm1_time);
  Serial.println("alarm1_msg: " + alarm1_msg);
  Serial.println("alarm2_en: " + alarm2_en);
  Serial.println("alarm2_time: " + alarm2_time);
  Serial.println("alarm2_msg: " + alarm2_msg);
  Serial.println("param_flag: " + param_flag);
  preferences.end();

  //测试用,如启动不了开启此句
  // param_flag="";


  //重置参数引脚
  if (reset_pin != "-1")
    pinMode(reset_pin.toInt(), INPUT_PULLUP);

  //LED引脚
  if (led_pin != "-1")
    pinMode(led_pin.toInt(), OUTPUT);

  //如果参数有效标志 / ssid / mcpEndpoint  任一为空，
  //进入AP配网模式
  if (param_flag == "1" &&  ssid != "" && mcpEndpoint != "") {
    Serial.print("wifi连接...");
    //进行连接
    WiFi.mode(WIFI_STA);   //如果不修改模式调用WiFi.begin(ssid, pwd),则自动变成 WIFI_STA 或 STA_AP模式
    WiFi.begin(ssid, pwd);
    // 连接15秒，避免停电路由器开机慢而导致无法连接的问题
    for (size_t i = 0; i < 15; i++) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("历史信息联网成功");
        //WiFi.setAutoReconnect(true); //显性开启自动重连，默认是开启的

        Serial.println("boot Connected");
        Serial.println("My Local IP is : ");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.gatewayIP().toString());
        Serial.println(WiFi.subnetMask().toString());
        Serial.println(WiFi.dnsIP().toString());

        apstate = false;
        return;
      } else {
        Serial.print(".");
        delay(1000);
      }
    }
  }



  // 如果连接失败, 打开AP
  if (apstate) {
    Serial.print("进入AP配网模式");
    set_led(HIGH);

    uint8_t mac[6];
    //读取mac地址
    WiFi.macAddress(mac);
    String mac_str = String(mac[0], HEX) + String(mac[1], HEX) + String(mac[2], HEX) + String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);

    //WiFi.mode(WIFI_AP_STA); //使用STA+AP 模式
    WiFi.mode(WIFI_AP); //使用STA 模式
    // WiFi.persistent(false); //关闭持久化存储
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //配置AP
    //开启AP
    WiFi.softAP(AP_SSID + mac_str, AP_PASSWORD);
    Serial.print("softAP");

    //启动web服务
    ap_web_init();
  }
  else
    set_led(LOW);
}
