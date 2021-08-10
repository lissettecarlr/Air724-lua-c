--- 模块功能：MQTT客户端数据接收处理
-- @author openLuat
-- @module mqtt.mqttInMsg
-- @license MIT
-- @copyright openLuat
-- @release 2018.03.28

module(...,package.seeall)

local cmd
local setMac
local wifiName
local wifiPw
--- MQTT客户端数据接收处理
-- @param mqttClient，MQTT客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage mqttInMsg.proc(mqttClient)
function proc(mqttClient)
    local result,data
    while true do
        result,data = mqttClient:receive(60000,"APP_SOCKET_SEND_DATA")
        --接收到数据
        if result then
            --log.info("mqtt接收到消息",data.topic,string.toHex(data.payload))
            log.info("mqtt接收到消息",data.topic,data.payload)

            local tjsondata,result,errinfo = json.decode(data.payload)
            if result then
                --下行JSON命令
                --log.info("json消息",tjsondata)
                cmd = tjsondata["cmd"]
                log.info("命令的值为：",cmd)
                --修改笔地址
                if (cmd=="SetPenMac") then
                    setMac = tjsondata["mac"]
                --修改WIFI地址
                elseif(cmd =="SetWiFi")then
                    wifiName = tjsondata["name"]
                    wifiPw = tjsondataP["password"]
                end
            --TODO：根据需求自行处理data.payload
            else
                log.info("json","解析错误")
            end
        else
            break
        end
    end
	
    return result or data=="timeout" or data=="APP_SOCKET_SEND_DATA"
end
