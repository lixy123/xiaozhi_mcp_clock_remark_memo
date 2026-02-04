
const char configPage[] PROGMEM = R"=====(
<!Doctype html>
<html >
<head>
  <meta charset="UTF-8">
</head>
<body>
        <h1>参数设置</h1>
        <form action="/save" id="myForm" method="POST">
            WIFI_SSID: <input type="text" name="ssid"  value="<<ssid>>">
           
            WIFI_Password: <input type="password" name="pwd"  value="<<pwd>>">
            <br>
            mcpEndpoint: <input type="text" name="mcpEndpoint"  style="width:600px" value="<<mcpEndpoint>>">
            <br> 
            baidu api key: <input type="text" name="baidu_api_key"  style="width:200px"  value="<<baidu_api_key>>">          
            baidu secret key: <input type="text" name="baidu_secret_key" style="width:200px"   value="<<baidu_secret_key>>">
            <br> 
            reset_pin: <input type="text" name="reset_pin"  style="width:50px"   value="<<reset_pin>>">            
            led_pin: <input type="text" name="led_pin"  style="width:50px"   value="<<led_pin>>">
            <br>            
            i2s_out_bclk:<input type="text" name="i2s_out_bclk"    style="width:50px"  value="<<i2s_out_bclk>>">
            i2s_out_lrc:<input type="text" name="i2s_out_lrc"  style="width:50px"  value="<<i2s_out_lrc>>">                   
            i2s_out_dout:<input type="text" name="i2s_out_dout"   style="width:50px"   value="<<i2s_out_dout>>">           
            <br>
            闹钟1:<input type="checkbox" name="alarm1_en" <<alarm1_en>>>  
            闹钟1-时间:<input type="text" name="alarm1_time"    style="width:50px"  value="<<alarm1_time>>">
            闹钟1-消息:<input type="text" name="alarm1_msg"  style="width:200px"  value="<<alarm1_msg>>">
            <br>
            闹钟2:<input type="checkbox" name="alarm2_en"  <<alarm2_en>>>           
            闹钟2-时间:<input type="text" name="alarm2_time"   style="width:50px"   value="<<alarm2_time>>">           
            闹钟2-消息:<input type="text" name="alarm2_msg"  style="width:200px"  value="<<alarm2_msg>>">
            <br>
            <input type="submit" value="保存">
            <br>
            <<span>>
        </form>
        
<script>

// 获取表单元素并添加事件监听器，监听submit事件
document.getElementById('myForm').addEventListener('submit', function (event) {
    if (!validateRequired()) {
        //alert("参数检查未通过!")
        event.preventDefault() // 阻止表单提交
    }
})

// 校验参数
function validateRequired() {
    // 获取表单中文本的输入元素
    var ssid = document.getElementsByName('ssid')[0]
    var mcpEndpoint = document.getElementsByName('mcpEndpoint')[0]
    var baidu_api_key = document.getElementsByName('baidu_api_key')[0]
    var baidu_secret_key = document.getElementsByName('baidu_secret_key')[0]
    var reset_pin = document.getElementsByName('reset_pin')[0]
    var led_pin = document.getElementsByName('led_pin')[0]
    var i2s_out_bclk = document.getElementsByName('i2s_out_bclk')[0]
    var i2s_out_lrc = document.getElementsByName('i2s_out_lrc')[0]
    var i2s_out_dout = document.getElementsByName('i2s_out_dout')[0]

    if (ssid.value.trim() === "") {
        alert("ssid不能为空！")
        return false
    }
 
    if (mcpEndpoint.value.trim() === "") {
        alert("mcpEndpoint不能为空！")
        return false
    }

    if (baidu_api_key.value.trim() === "") {
        alert("baidu_api_key不能为空！")
        return false
    }
    if (baidu_secret_key.value.trim() === "") {
        alert("baidu_secret_key不能为空！")
        return false
    }
    if (reset_pin.value.trim() === "") {
        alert("reset_pin不能为空, 可以用-1！")
        return false
    }
    if (led_pin.value.trim() === "") {
        alert("led_pin不能为空, 可以用-1")
        return false
    }

  if (isNaN(reset_pin.value.trim())) {
        alert("reset_pin不是数值！")
        return false
    }
    if (isNaN(led_pin.value.trim())) {
        alert("led_pin不是数值！")
        return false
    }
    
    if (i2s_out_bclk.value.trim() === "") {
        alert("i2s_out_bclk不能为空！")
        return false
    }
    if (i2s_out_lrc.value.trim() === "") {
        alert("i2s_out_lrc不能为空！")
        return false
    }
    if (i2s_out_dout.value.trim() === "") {
        alert("i2s_out_dout不能为空！")
        return false
    }
  
  if (isNaN(i2s_out_bclk.value.trim())) {
        alert("i2s_out_bclk不是数值！")
        return false
    }
    if (isNaN(i2s_out_lrc.value.trim())) {
        alert("i2s_out_lrc不是数值！")
        return false
    }
    if (isNaN(i2s_out_dout.value.trim())) {
        alert("i2s_out_dout不是数值！")
        return false
    }
  
    return true
    console.log("验证通过！")
}
</script>
        
</body>
</html>

)=====";
