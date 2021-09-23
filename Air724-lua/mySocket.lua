require "socket"
module(...,package.seeall)


local tcpServerIp = "47.108.178.9"
local tcpServerPort = "3001"

local ready = false

--- socket连接是否处于激活状态
-- @return 激活状态返回true，非激活状态返回false
-- @usage socketTask.isReady()
function isReady()
    return ready
end


----------------------------------------------------------------------------------------

--- socket客户端数据接收处理
-- @param socketClient，socket客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage socketInMsg.proc(socketClient)
function socketInMsgproc(socketClient)
    local result,data
    if(isReady()==false)then
        return false
    end
    while true do
        result,data = socketClient:recv(60000,"APP_SOCKET_SEND_DATA")
        --接收到数据
        if result then
            log.info("socket接收到数据：",data)
                
            --TODO：根据需求自行处理data
            
        else
            break
        end
    end
	
    return result or data=="timeout" or data=="APP_SOCKET_SEND_DATA"
end

----------------------------------------------------------------------------------------

--数据发送的消息队列
local msgQueue = {}

local function insertMsg(data,user)
    table.insert(msgQueue,{data=data,user=user})
    sys.publish("APP_SOCKET_SEND_DATA")
end

-- 心跳包
-- local heartPck="heart data\r\n"
-- function setHeartPck(newStr)
--     heartPck = newStr
-- end
-- local function sndHeartCb(result)
--     log.info("socketOutMsg.sndHeartCb",result)
--     if result then sys.timerStart(sndHeart,60000) end
-- end
-- function sndHeart()
--     log.info("heart!")
--     -- insertMsg(heartPck,{cb=sndHeartCb})
-- end

-- local function sndLocCb(result)
--     log.info("socketOutMsg.sndLocCb",result)
--     if result then sys.timerStart(sndLoc,20000) end
-- end

-- function sndLoc()
--     insertMsg("location data\r\n",{cb=sndLocCb})
-- end

function sendCB()
    log.info("socket send cb 发送回调")
end

function send(data)
    log.info("tset send","socket send")
    insertMsg(data,{cb=sendCB})
end

--- 初始化“socket客户端数据发送”
-- @return 无
-- @usage socketOutMsg.init()
function socketOutMsgInit()
    sndHeart()
    --sndLoc()
end

--- 去初始化“socket客户端数据发送”
-- @return 无
-- @usage socketOutMsg.unInit()

-- function socketOutMsgUnInit()
--     sys.timerStop(sndHeart)
--     sys.timerStop(sndLoc)
--     while #msgQueue>0 do
--         local outMsg = table.remove(msgQueue,1)
--         if outMsg.user and outMsg.user.cb then outMsg.user.cb(false,outMsg.user.para) end
--     end
-- end

--- socket客户端数据发送处理
-- @param socketClient，socket客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage socketOutMsg.proc(socketClient)
function socketOutMsgProc(socketClient)
    while #msgQueue>0 do
        local outMsg = table.remove(msgQueue,1) --取出数据
        local result = socketClient:send(outMsg.data)--发送
        -- if outMsg.user and outMsg.user.cb then 
        --     outMsg.user.cb(result,outMsg.user.para) --触发回调
        -- end
        if not result then 
            return false
        end
    end
    return true
end
----------------------------------------------------------------------------------
local gol_hanld
--启动socket客户端任务

sys.taskInit(
    function()
        local retryConnectCnt = 0
        while true do
            if not socket.isReady() then
                retryConnectCnt = 0
                --等待网络环境准备就绪，超时时间是5分钟
                sys.waitUntil("IP_READY_IND",300000)
            end

            if socket.isReady() then
                --创建一个socket tcp客户端
                local socketClient = socket.tcp()
                gol_hanld = socketClient
                --阻塞执行socket connect动作，直至成功
                if socketClient:connect(tcpServerIp,tcpServerPort) then
                    log.info("TCP连接成功")
                    retryConnectCnt = 0
                    ready = true

                    --send("smartpen connect")
                    -- socketOutMsgInit()
                    --循环处理接收和发送的数据
                    while true do
                        --暂时不需要接收
                        sys.wait(50)
                        --if not socketInMsgproc(socketClient) then log.error("socketTask.socketInMsg.proc error") break end
                        if not socketOutMsgProc(socketClient) then 
                            log.error("socketTask.socketOutMsg proc error 发送失败") 
                            --break 
                    
                        end
                    end
                    -- socketOutMsgUnInit()

                    ready = false
                else
                    retryConnectCnt = retryConnectCnt+1
                    log.info("TCP连接失败",retryConnectCnt)
                end
                --断开socket连接
                socketClient:close()
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


----------------------------------------------------------------------------------------