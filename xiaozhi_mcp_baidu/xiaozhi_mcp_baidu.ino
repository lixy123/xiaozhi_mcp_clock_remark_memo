#include <WiFi.h>
#include <WebSocketMCP.h>
#include <LinkedList.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RTClib.h>
#include "Baidu.h"
//#include "wen_ap.h"

RTC_Millis rtc;
long lastNtpUpdate;
bool rtc_ok = false;

// NTP客户端配置
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 8 * 3600, 60000); // 东八区(UTC+8)
char daysOfTheWeek[7][12] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};

// 缓冲区管理
#define MAX_INPUT_LENGTH 1024
char inputBuffer[MAX_INPUT_LENGTH];
int inputBufferIndex = 0;
bool newCommandAvailable = false;

// 连接状态
bool wifiConnected = false;
bool mcpConnected = false;



// 创建WebSocketMCP实例
WebSocketMCP mcpClient;

unsigned long  esp32_root_time;
//计时器，多久后提醒
StaticJsonDocument<512> doc_remind;

//闹钟， 指定时分秒提醒
StaticJsonDocument<512> doc_clock;

LinkedList<String> list_note = LinkedList<String>();


/********** 函数声明 ***********/
//void setupWifi();
void onMcpOutput(const String &message);
void onMcpError(const String &error);
void onMcpConnectionChange(bool connected);
void processSerialCommands();
void registerMcpTools();



String Get_softrtc_time(int flag)
{
  DateTime now = rtc.now();
  char buf[50];
  buf[0] = '\0';
  if (flag == 0)
  {
    sprintf(buf, "%02d,%02d,%02d,%02d,%02d,%02d", now.year(), now.month() , now.day(), now.hour(), now.minute(), now.second());
  }
  if (flag == 1)
  {
    sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  }
  else if (flag == 2)
  {
    sprintf(buf, "%02d,%02d,%02d,%02d,%02d", now.year(), now.month() , now.day(), now.hour(), now.minute());

  }
  else if (flag == 3)
  {
    sprintf(buf, "%02d月%02d日%s",  now.month() , now.day(), daysOfTheWeek[now.dayOfTheWeek()]);
  }
  else if (flag == 4)
  {
    sprintf(buf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  }
  else if (flag == 5)
  {
    sprintf(buf, "%02d-%02d-%02d %02d:%02d", now.year(), now.month() , now.day(), now.hour(), now.minute());

  }
  else if (flag == 6)
  {
    sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month() , now.day(), now.hour(), now.minute(), now.second());

  }
  return String(buf);
}

/**
   MCP输出回调函数(stdout替代)
*/
void onMcpOutput(const String &message) {
  Serial.print("[MCP输出] ");
  Serial.println(message);
}

/**
   MCP错误回调函数(stderr替代)
*/
void onMcpError(const String &error) {
  Serial.print("[MCP错误] ");
  Serial.println(error);
}

void Task_mytask(void *pvParameters) {
  String timeStr5;
  String timeStr6;
  int last_alarm1_cnt = 0;
  int last_alarm2_cnt = 0;
  while (1) 
  {
    timeStr5 = Get_softrtc_time(5);
    timeStr6 = Get_softrtc_time(6);

    //计时提醒
    if (doc_remind["status"].as<bool>() == true )
    {
      int set_time = doc_remind["hours"].as<int>() * 3600 + doc_remind["minutes"].as<int>() * 60;
      int elapsed_time = int(millis() / 1000) -  doc_remind["note_time"].as<int>();

      //时间溢出处理
      if (elapsed_time < 0)
        doc_remind["note_time"] = 0;

      //秒数
      if (int(millis() / 1000) -  doc_remind["note_time"].as<int>() > set_time)
      {
        Serial.println("\nplay remind:" + doc_remind["note"].as<String>() );

        if (doc_remind["note"].as<String>().length() > 0)
          baiduTTS_Send("提醒时间到:" + doc_remind["note"].as<String>());
        else
          baiduTTS_Send("提醒时间到!");

        doc_remind["status"] = false;
      }
      else
      {
        Serial.printf("计时秒数:%d，过去秒数:%d\n", set_time, elapsed_time );
      }
    }

    //闹钟提醒
    if (doc_clock["status"].as<bool>() == true )
    {
      //确保时钟是准的
      if (rtc_ok)
      {
        char buf[50];
        buf[0] = '\0';
        sprintf(buf, "%02d-%02d-%02d %02d:%02d", doc_clock["year"].as<int>() ,
                doc_clock["month"].as<int>(), doc_clock["day"].as<int>(),
                doc_clock["hour"].as<int>(), doc_clock["minute"].as<int>());

        Serial.println("闹钟时间:" + String(buf) + ", 当前时间:" + timeStr6 );
        if (String(buf) == timeStr5)
        {
          Serial.println("\nplay clock:" + doc_clock["note"].as<String>() );

          if (doc_clock["note"].as<String>().length() > 0)
            baiduTTS_Send("闹钟时间到:" + doc_clock["note"].as<String>());
          else
            baiduTTS_Send("闹钟时间到!");

          doc_clock["status"] = false;
        }
      }
    }


    //预置闹钟1提醒
    if (alarm1_en != "" )
    {
      //时间匹配上
      if (timeStr5.indexOf(alarm1_time) != -1  )
      {
        if (last_alarm1_cnt == 0)
        {
          if (alarm1_msg.length() > 0)
            baiduTTS_Send(alarm1_msg);
          else
            baiduTTS_Send("闹钟时间到!");
          last_alarm1_cnt = 1;
        }
      }
      else  //下一分钟，清除此闹钟标记位
        last_alarm1_cnt = 0;
    }

    //预置闹钟2提醒
    if (alarm2_en != "" )
    {
      //时间匹配上
      if (timeStr5.indexOf(alarm2_time) != -1)
      {
        if (last_alarm2_cnt == 0)
        {
          if (alarm2_msg.length() > 0)
            baiduTTS_Send(alarm2_msg);
          else
            baiduTTS_Send("闹钟时间到!");
          last_alarm2_cnt = 1;
        }
      }
      else  //下一分钟，清除此闹钟标记位
        last_alarm2_cnt = 0;
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS); // 延时5秒
  }
}



// 注册MCP工具
void registerMcpTools() {



  //###################################  计时器提醒 (简化，只一条)   #########################################

  mcpClient.registerTool(
    "remind_me_time_later",
    "设定提醒",
    "{\"type\":\"object\",\"properties\":{\"hours\":{\"type\":\"integer\"},\"minutes\":{\"type\":\"integer\"},\
        \"note\":{\"type\":\"string\"}}, \
        \"required\":[\"hours\",\"minutes\",\"note\"]}",
  [](const String & args) {

    Serial.println("remind_me_time_later:" + args);

    String result = "完毕";
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }
    int p_hours = doc["hours"].as<int>();
    int p_minutes = doc["minutes"].as<int>();
    String note = doc["note"].as<String>();

    doc_remind["hours"] = p_hours;
    doc_remind["minutes"] = p_minutes;
    doc_remind["note"] = note;
    //启动时的秒数
    doc_remind["note_time"] =  int(millis() / 1000);
    doc_remind["status"] = true;

    Serial.printf("hours=%d, minutes=%d, note=%s, note_time=%d, status=%d\n",
                  doc_remind["hours"].as<int>(), doc_remind["minutes"].as<int>(), doc_remind["note"].as<String>(), doc_remind["note_time"].as<int>(),
                  doc_remind["status"].as<bool>() );


    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );

  mcpClient.registerTool(
    "remind_me_read",
    "查询提醒",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("remind_me_read:" + args);
    String result = "";
    if ( doc_remind["status"].as<bool>() == true)
    {
      result =  doc_remind["note"].as<String>();
    }

    Serial.printf("hours=%d, minutes=%d, note=%s, note_time=%d, status=%d\n",
                  doc_remind["hours"].as<int>(), doc_remind["minutes"].as<int>(),  doc_remind["note"].as<String>(), doc_remind["note_time"].as<int>(),
                  doc_remind["status"].as<bool>() );

    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );


  mcpClient.registerTool(
    "remind_me_delete",
    "删除提醒",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("remind_me_delete:" + args);
    String result = "完毕";
    doc_remind["status"] = false;

    Serial.printf("hours=%d, minutes=%d, note=%s, note_time=%d, status=%d\n",
                  doc_remind["hours"].as<int>(), doc_remind["minutes"].as<int>(), doc_remind["note"].as<String>(), doc_remind["note_time"].as<int>(),
                  doc_remind["status"].as<bool>() );

    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );
  //###################################  闹钟提醒  (简化，只一条)     #########################################


  mcpClient.registerTool(
    "alarm_clock_set",
    "设定闹钟",
    "{\"type\":\"object\",\"properties\":{\"year\":{\"type\":\"integer\"},\"month\":{\"type\":\"integer\"},\"day\":{\"type\":\"integer\"},\
        \"hour\":{\"type\":\"integer\"},\"minute\":{\"type\":\"integer\"},\
        \"note\":{\"type\":\"string\"}}, \
        \"required\":[\"year\",\"month\",\"day\",\"hour\",\"minute\",\"note\"]}",
  [](const String & args) {

    Serial.println("alarm_clock_set:" + args);

    String result = "完毕";
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }

    doc_clock["year"] =  doc["year"].as<int>();
    doc_clock["month"] = doc["month"].as<int>();
    doc_clock["day"] = doc["day"].as<int>();
    doc_clock["hour"] = doc["hour"].as<int>();
    doc_clock["minute"] = doc["minute"].as<int>();
    doc_clock["note"] = doc["note"].as<String>();
    doc_clock["status"] = true;

    Serial.printf("year=%d, month=%d, day=%d, hours=%d, minutes=%d, note=%s,  status=%d\n",
                  doc_clock["year"].as<int>(), doc_clock["month"].as<int>(), doc_clock["day"].as<int>()   ,
                  doc_clock["hour"].as<int>(), doc_clock["minute"].as<int>(), doc_clock["note"].as<String>(),
                  doc_clock["status"].as<bool>() );
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );



  mcpClient.registerTool(
    "alarm_clock_read",
    "查询闹钟",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("alarm_clock_read:" + args);
    String result = "";
    if ( doc_clock["status"].as<bool>() == true)
    {
      result =  doc_clock["note"].as<String>();
    }

    Serial.println(Get_softrtc_time(5));

    Serial.printf("year=%d, month=%d, day=%d, hours=%d, minutes=%d, note=%s,  status=%d\n",
                  doc_clock["year"].as<int>(), doc_clock["month"].as<int>(), doc_clock["day"].as<int>()   ,
                  doc_clock["hour"].as<int>(), doc_clock["minute"].as<int>(), doc_clock["note"].as<String>(),
                  doc_clock["status"].as<bool>() );
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );


  mcpClient.registerTool(
    "alarm_clock_delete",
    "删除闹钟",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("alarm_clock_delete:" + args);
    String result = "完毕";
    doc_clock["status"] = false;

    Serial.printf("year=%d, month=%d, day=%d, hours=%d, minutes=%d, note=%s,  status=%d\n",
                  doc_clock["year"].as<int>(), doc_clock["month"].as<int>(), doc_clock["day"].as<int>()   ,
                  doc_clock["hour"].as<int>(), doc_clock["minute"].as<int>(), doc_clock["note"].as<String>(),
                  doc_clock["status"].as<bool>() );

    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );


  //###################################  备忘录 (多条)   #########################################
  //实现一条简单，实现多条复杂
  //因为AI无法即时判断你有几条备忘录， 在问出内容并在尝试修改第N条备忘录时才提醒你没有第N条，显得很愚蠢

  mcpClient.registerTool(
    "memo_set",
    "记录备忘录",
    "{\"type\":\"object\",\"properties\":{\"note\":{\"type\":\"string\"}}, \
        \"required\":[\"note\"]}",
  [](const String & args) {

    Serial.println("memo_set:" + args);

    String result = "完毕";
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }
    String note = doc["note"].as<String>();
    list_note.add(note);
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );

  mcpClient.registerTool(
    "memo_delete",
    "删除备忘录",
    "{\"type\":\"object\",\"properties\":{\"no\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":1000}}, \
       \"required\":[\"no\"]}",
  [](const String & args) {

    Serial.println("memo_delete:" + args);

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }
    int no = doc["no"].as<int>();
    String result = "";
    if (no <= list_note.size())
    {
      result = list_note[no - 1];
      list_note.remove(no - 1);
    }
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );


  mcpClient.registerTool(
    "memo_modify",
    "修改备忘录",
    "{\"type\":\"object\",\"properties\":{\"no\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":1000}, \
       \"note\":{\"type\":\"string\"}}, \
       \"required\":[\"no\",\"note\"]}",
  [](const String & args) {
    Serial.println("memo_modify:" + args);
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }
    int no = doc["no"].as<int>();
    String note = doc["note"].as<String>();
    String result = "完毕";

    if (no <= list_note.size())
      list_note[no - 1] = note;
    else
      result = "没有第" + String(no) + "条备忘录";
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }                                  );

  mcpClient.registerTool(
    "memo_read",
    "查询备忘录",
    "{\"type\":\"object\",\"properties\":{\"no\":{\"type\":\"integer\",\"minimum\":1,\"maximum\":1000}}, \
       \"required\":[\"no\"]}",
  [](const String & args) {
    Serial.println("memo_read:" + args);

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, args);

    if (error) {
      // 返回错误响应
      WebSocketMCP::ToolResponse response("{\"success\":false,\"error\":\"无效的参数格式\"}", true);
      return response;
    }
    int no = doc["no"].as<int>();
    String result = "";
    if (no <= list_note.size())
      result = list_note[no - 1];

    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );


  mcpClient.registerTool(
    "memo_clear",
    "清空备忘录",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("memo_clear:" + args);
    String result = "";
    if ( list_note.size() > 0)
    {
      list_note.clear();
      result = "完毕";
    }
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + result + "\"}");
  }
  );

  mcpClient.registerTool(
    "memo_length",
    "查询备忘录数量",
    "{\"type\":\"object\",\"properties\":{}}",
  [](const String & args) {
    Serial.println("memo_length:" + args);
    int result = list_note.size();
    return WebSocketMCP::ToolResponse(String("{\"success\":true,\"result\":\"") + String(result) + "\"}");
  }
  );

  Serial.println("[MCP] 注册完成");
}


/**
   MCP连接状态变化回调函数
*/
void onMcpConnectionChange(bool connected) {
  mcpConnected = connected;
  if (connected) {
    Serial.println("[MCP] 已连接到MCP服务器");
    // 连接成功后注册工具
    registerMcpTools();
  } else {
    Serial.println("[MCP] 与MCP服务器断开连接");
  }
}


void update_ntp(bool force)
{
  long now = long( millis() / 1000);
  //每小时同步一次
  if (now - lastNtpUpdate >= 60 * 60 || force)
  {
    if ( timeClient.update())
    {
      unsigned long epochTime = timeClient.getEpochTime(); // 获取 epoch 时间（秒）
      rtc.adjust(DateTime(epochTime)); // 调整 RTC 时间到网络时间
      //Serial.println("RTC time synchronized with NTP time.");
      Serial.println("synchronize ntp time:" + timeClient.getFormattedTime()); // 显示格式化的时间，例如："23:59:59"（根据实际情况调整）
      rtc_ok = true;
      Serial.println(Get_softrtc_time(5));
    }
    lastNtpUpdate = int(millis() / 1000);
  }

  //处理时间溢出
  if ( (millis() / 1000) < lastNtpUpdate)
    lastNtpUpdate = millis() / 1000;

}



void loop() {
 //处理配网模式必要逻辑
  ap_serve();

  if (apstate)
    return;
    
  if (wifiConnected)
  {
    //百度百度令牌每30天失效
    //每5天，重新获取百度令牌
    if  ( (millis() - esp32_root_time) >  3600 * 24 * 5 * 1000)
    {
      get_access_token();
      esp32_root_time = millis();
    }

    update_ntp(false);
  }

  // 处理MCP客户端事件
  mcpClient.loop();
}


void setup() {

  Serial.begin(115200);

    //必须加上，判断是否进入配网模式
  ap_init();

  //非AP配网模式，正常流程处理...
  if (apstate)
  {
    Serial.println("AP 配网模式...");
    return;
  }
  wifiConnected = true;
   
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);
  esp32_root_time = millis();

  //初始化提醒变量
  doc_remind["hours"] = -1;
  doc_remind["minutes"] = -1;
  doc_remind["note"] = "";
  doc_remind["note_time"] = 0;
  doc_remind["status"] = false;

  //初始化闹钟变量
  doc_clock["year"] = -1;
  doc_clock["month"] = -1;
  doc_clock["day"] = -1;
  doc_clock["hour"] = -1;
  doc_clock["minutes"] = -1;
  doc_clock["note"] = "";
  doc_clock["status"] = false;

  //到时间后，处理提醒

  xTaskCreatePinnedToCore(
    Task_mytask,
    "Task_my",
    10000,
    NULL,
    1,
    NULL,
    0
  );

  I2S_Init(I2S_MODE_TX, 16000, I2S_BITS_PER_SAMPLE_16BIT );

  // 连接WiFi
  //setupWifi();

  if (wifiConnected)
    get_access_token();

  //NTP校对时间

  Serial.println(Get_softrtc_time(5));

  // NTP同步至本地时间
  timeClient.begin();

  update_ntp(true);
  Serial.println(Get_softrtc_time(5));
  lastNtpUpdate = int(millis() / 1000);


  // 初始化MCP客户端
  if (mcpClient.begin(mcpEndpoint.c_str(), onMcpConnectionChange)) {
    Serial.println("[ESP32 MCP客户端] 初始化成功，尝试连接到MCP服务器...");
  } else {
    Serial.println("[ESP32 MCP客户端] 初始化失败!");
  }

   Serial.println("start...");
}
