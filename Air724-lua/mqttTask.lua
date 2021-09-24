-- 该文件用于建立MQTT连接
-- 实现智能笔盒的配置协议


module(...,package.seeall)

require"misc"
require"mqtt"
-- require"mqttOutMsg"
-- require"mqttInMsg"

local ready = false

local mqttServerHost = 'ali.kala.love'
local mqttServerPort = 1883
local mqttServerUser = 'fff'
local mqttServerPasswort = 'fff'
local subTopic = '/penbox/AABBCCDDEEFF/CMD'
local popTopic = '/penbox/AABBCCDDEEFF/ACK'
local ackNormal = "{\"type\": 2,\"msg\":\"AckNormal\"}"
local ackPeninfo = "{\"type\": 2,\"msg\":\"AckPbInfo\", \
                    \"PenboxMac\":\"11:22:33:44:55:66\", \
                    \"PenMac\":[{\"mac\":\"11:22:33:44:55:66\"},{\"mac\":\"66:55:44:33:22:11\"}],\
                    \"version\":1 \
                    \"Runtime\":60, \
                    \"Cat1RSSI\":20, \
                    \"WifiRSSI\":20, \
                    \"BleRSSI\":20,\
                    \"OfflineDataSwitch\":\"OFF\",\
                    \"WifiMac\":\"11:22:33:44:55:66\",\
                    \"WifiApMac\":\"11:22:33:44:55:00\"}"
function isReady()
    return ready
end

--启动MQTT客户端任务
sys.taskInit(
    function()
        -- 标志位，用于表示重复连接次数
        local retryConnectCnt = 0
        -- 循环 如果没有网络则进入等待
        while true do
            if not socket.isReady() then
                retryConnectCnt = 0
                --等待网络环境准备就绪，超时时间是5分钟
                sys.waitUntil("IP_READY_IND",300000)
            end
            
            -- 如果网络状态正常则开始建立mqtt连接
            if socket.isReady() then
                local imei = misc.getImei()
                --创建一个MQTT客户端
                local mqttClient = mqtt.client(imei,600,mqttServerUser,mqttServerPasswort)
                --local mqttClient = mqtt.client(imei,600)
                --阻塞执行MQTT CONNECT动作，直至成功
                if mqttClient:connect(mqttServerHost,mqttServerPort,"tcp") then
                    retryConnectCnt = 0
                    ready = true
                    log.info("MQTT连接成功，开始订阅")
                    --订阅主题
                    if mqttClient:subscribe({[subTopic]=0, ["/penbox/test"]=1}) then
                        -- 接收订阅循环
                        while true do
                            local res,data = mqttClient:receive(60000)
                            if res then
                                log.info("MQTT接收到消息",data.topic,data.payload or "nil")
                                local jsData,jsRes,jsErrinfo=json.decode(data.payload)
                                -- 1为命令 3为透传
                                if(jsData['type'] ==1)then
                                    cmd = jsData['cmd']
                                    log.info("MQTT 下行命令",cmd)
                                    if(cmd == 'test')then
                                        log.info("MQTT 下行测试成功！")
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'SetPenMac')then
                                        -- "pen":[{"mac":"11:22:33:44:55:66"},{"mac":"66:55:44:33:22:11"}]}                                    
                                        -- 官网文档这个json的API没法解析这个
                                        log.info("setPenMac:",jsData["pen"])
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == "SetWiFi")then
                                        log.info("setWifi:",jsData["name"],tjsondata["password"])
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'GetPdInfo')then
                                        mqttClient:publish(popTopic,ackPeninfo,0)
                                    elseif(cmd == 'GetLog')then
                                        -- mqttClient:publish(popTopic,ackNormal,0)
                                        mqttClient:publish(popTopic,'log test',0)
                                    elseif(cmd == 'DownloadFirmware')then
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'UpdateFirmware')then
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'SetTime')then
                                        log.info("设置时间：",jsData['time'])
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'CloseWifi')then
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'SetText')then
                                        log.info("设置显示文本：",jsData['text'])
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    elseif(cmd == 'OfflineDataSwitch')then
                                        log.info("设置离线数据存储开关：",jsData['switch'])
                                        mqttClient:publish(popTopic,ackNormal,0)
                                    else 
                                        log.info("MQTT 命令解析失败") 
                                    end 
                                elseif(jsData['type'] == 3)then
                                    log.info("MQTT 透传数据类型",jsData['data'])
                                else
                                    log.info("MQTT 数据类型位置")        
                                end                                               
                                -- if(mqttClient:publish("/penbox/AABBCCDDEEFF/ACK","test msg!",0))then
                                --     log.info("发送成功")
                                -- end
                            else -- 如果未接收到数据
                                log.info("mqtt退出接收",data)
                                if(data ~= "timeout") then
                                    log.info("mqtt接收错误，尝试重新建立连接",data)
                                    break
                                end
                            end
                        end    
                    end
                    ready = false
                else
                    retryConnectCnt = retryConnectCnt+1
                end
                --断开MQTT连接
                mqttClient:disconnect()
                -- if retryConnectCnt>=5 then link.shut() retryConnectCnt=0 end
                sys.wait(5000)
            else
                --进入飞行模式，20秒之后，退出飞行模式
                net.switchFly(true)
                sys.wait(20000)
                net.switchFly(false)
            end
        end
    end
)
