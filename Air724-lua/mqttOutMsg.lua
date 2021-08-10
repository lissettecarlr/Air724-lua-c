--- 模块功能：MQTT客户端数据发送处理
-- @author openLuat
-- @module mqtt.mqttOutMsg
-- @license MIT
-- @copyright openLuat
-- @release 2018.03.28


module(...,package.seeall)

--数据发送的消息队列
local msgQueue = {}

local function insertMsg(topic,payload,qos,user)
    table.insert(msgQueue,{t=topic,p=payload,q=qos,user=user})
    sys.publish("APP_SOCKET_SEND_DATA")
end

-- local function pubQos0TestCb(result)
--     log.info("mqttOutMsg.pubQos0TestCb",result)
--     if result then sys.timerStart(pubQos0Test,10000) end
-- end

-- function pubQos0Test()
--     insertMsg("/qos0topic","qos0data",0,{cb=pubQos0TestCb})
-- end

local function pubQos1TestCb(result)
    log.info("mqttOutMsg.pubQos1TestCb",result)
    if result then sys.timerStart(pubQos1Test,20000) end
end

-- 测试用报警信息
local torigin =
{
    msg = "”PbWaring”",
    mac = "11:22:33:44:55:66",
    DisconnectBT = 0,
    LowPower = 0,
    SimNotIN = 0,
    SimNotNet =0,
}
local jsondata = json.encode(torigin)

function pubQos1Test()
    log.info("testJson.encode",jsondata)
    insertMsg("penload",jsondata,1,{cb=pubQos1TestCb})
end

--- 初始化“MQTT客户端数据发送”
-- @return 无
-- @usage mqttOutMsg.init()
function init()
    --pubQos0Test()
    pubQos1Test()
end

--- 去初始化“MQTT客户端数据发送”
-- @return 无
-- @usage mqttOutMsg.unInit()
function unInit()
    sys.timerStop(pubQos0Test)
    sys.timerStop(pubQos1Test)
    while #msgQueue>0 do
        local outMsg = table.remove(msgQueue,1)
        if outMsg.user and outMsg.user.cb then outMsg.user.cb(false,outMsg.user.para) end
    end
end


--- MQTT客户端数据发送处理
-- @param mqttClient，MQTT客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage mqttOutMsg.proc(mqttClient)
function proc(mqttClient)
    while #msgQueue>0 do
        local outMsg = table.remove(msgQueue,1)
        local result = mqttClient:publish(outMsg.t,outMsg.p,outMsg.q)
        if outMsg.user and outMsg.user.cb then outMsg.user.cb(result,outMsg.user.para) end
        if not result then return end
    end
    return true
end
