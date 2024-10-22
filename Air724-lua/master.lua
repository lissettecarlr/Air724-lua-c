require "mySocket"
--require "myDebug"
require "protocol"
module(..., package.seeall)

--local ip,prot,c = "47.108.200.115", "1883"     
-- local ip,port,c = "47.108.178.9'", "3000"

-- local pen_addr = "d8:0b:cb:61:02:e7"
local pen_addr =    "54:b7:e5:79:f4:49"
-- local pen_addr = "d8:0b:cb:61:4d:84"

btStatus = false

local function init()
    log.info("bt", "init")
    rtos.on(rtos.MSG_BLUETOOTH, function(msg)
        if msg.event == btcore.MSG_OPEN_CNF then
            sys.publish("BT_OPEN", msg.result) --蓝牙打开成功
        elseif msg.event == btcore.MSG_BLE_CONNECT_CNF then
            sys.publish("BT_CONNECT_IND", {["handle"] = msg.handle, ["result"] = msg.result}) --蓝牙连接成功
        elseif msg.event == btcore.MSG_BLE_DISCONNECT_CNF then
            log.info("bt", "蓝牙断开连接！！！") --蓝牙断开连接
            btStatus = false
            disconpck = protocol.contentDecode(pen_addr,false)
            mySocket.send(disconpck)

        elseif msg.event == btcore.MSG_BLE_DATA_IND then
            sys.publish("BT_DATA_IND", {["data"] = msg.data, ["uuid"] = msg.uuid, ["len"] = msg.len})  --接收到的数据内容
        elseif msg.event == btcore.MSG_BLE_SCAN_CNF then
            sys.publish("BT_SCAN_CNF", msg.result) --打开扫描成功
        elseif msg.event == btcore.MSG_BLE_SCAN_IND then
            sys.publish("BT_SCAN_IND", {["name"] = msg.name, ["addr_type"] = msg.addr_type, ["addr"] = msg.addr, ["manu_data"] = msg.manu_data, 
            ["raw_data"] = msg.raw_data, ["raw_len"] = msg.raw_len, ["rssi"] = msg.rssi})  --接收到扫描广播包数据
        elseif msg.event == btcore.MSG_BLE_FIND_CHARACTERISTIC_IND then
            sys.publish("BT_FIND_CHARACTERISTIC_IND", msg.result)  --发现服务包含的特征
        elseif msg.event == btcore.MSG_BLE_FIND_SERVICE_IND then
            log.info("bt", "find service uuid",msg.uuid)  --发现蓝牙包含的16bit uuid
            if msg.uuid == 0x1800 then          --根据想要的uuid修改
                sys.publish("BT_FIND_SERVICE_IND", msg.result)
            end
        elseif msg.event == btcore.MSG_BLE_FIND_CHARACTERISTIC_UUID_IND then
            uuid_c = msg.uuid
            log.info("bt", "find characteristic uuid",msg.uuid) --发现到服务内包含的特征uuid
        end
    end)
end
local function poweron()
    log.info("bt", "poweron")
    btcore.open(1) --打开蓝牙主模式
    _, result = sys.waitUntil("BT_OPEN", 5000) --等待蓝牙打开成功
end

local function connect()
    log.info("开始蓝牙连接", "对象MAC：" .. pen_addr)
    btcore.connect(pen_addr)
    return true
end

local function scan()
    log.info("bt", "scan")
    --btcore.setscanparam(1,48,6,0,0)--扫描参数设置（扫描类型,扫描间隔,扫描窗口,扫描过滤测量,本地地址类型）
    btcore.scan(1) --开启扫描
    _, result = sys.waitUntil("BT_SCAN_CNF", 50000) --等待扫描打开成功
    if result ~= 0 then
        return false
    end
    sys.timerStart(
        function()
            sys.publish("BT_SCAN_IND", nil)
        end,
        10000)  
    while true do
        _, bt_device = sys.waitUntil("BT_SCAN_IND") --等待扫描回复数据
        if not bt_device then
            -- 超时结束
            btcore.scan(0) --停止扫描
            return false
        else
            -- log.info("bt", "scan result")
            -- log.info("bt.scan_name", bt_device.name)  --蓝牙名称
			-- log.info("bt.rssi", bt_device.rssi)  --蓝牙信号强度
            -- log.info("bt.addr_type", bt_device.addr_type) --地址种类
            -- log.info("bt.scan_addr", bt_device.addr) --蓝牙地址
            -- if bt_device.manu_data ~= nil then
            --     log.info("bt.manu_data", string.toHex(bt_device.manu_data)) --厂商数据
            -- end
            -- log.info("bt.raw_len", bt_device.raw_len)
            -- if bt_device.raw_data ~= nil then
            --     log.info("bt.raw_data", string.toHex(bt_device.raw_data)) --广播包原始数据
            -- end

            --蓝牙连接   根据设备蓝牙广播数据协议解析广播原始数据(bt_device.raw_data)
            if(bt_device.addr == pen_addr) then
                --log.info("bt.pen.add",bt_device.addr)
                name = bt_device.name
                addr_type = bt_device.addr_type
                addr = bt_device.addr
                manu_data = bt_device.manu_data
                adv_data = bt_device.raw_data -- 广播包数据 根据蓝牙广播包协议解析
                log.info("bt.check_name",name)
                scanrsp_data = bt_device.raw_data
                btcore.scan(0)  --停止扫描
                btcore.connect(pen_addr)
                return true
            else
                log.info("bt.mac_find",bt_device.addr)
                log.info("bt.mac_pen",pen_addr)
            end

            -- if (bt_device.name == "smart1") then   --连接的蓝牙名称根据要连接的蓝牙设备修改
            --     name = bt_device.name
            --     addr_type = bt_device.addr_type
            --     addr = bt_device.addr
            --     manu_data = bt_device.manu_data
            --     adv_data = bt_device.raw_data -- 广播包数据 根据蓝牙广播包协议解析
            -- end

            if addr == bt_device.addr and bt_device.raw_data ~= adv_data then --接收到两包广播数据
                scanrsp_data = bt_device.raw_data --响应包数据 根据蓝牙广播包协议解析
                btcore.scan(0)  --停止扫描
                btcore.connect(bt_device.addr)
                log.info("bt.connect_name", name)
                log.info("bt.connect_addr_type", addr_type)
                log.info("bt.connect_addr", addr)
                if manu_data ~= nil then
                    log.info("bt.connect_manu_data", manu_data)
                end
                if adv_data ~= nil then
                    log.info("bt.connect_adv_data", adv_data)
                end
                if scanrsp_data ~= nil then
                    log.info("bt.connect_scanrsp_data", scanrsp_data)
                end
                return true
            end

        end
    end
    return true
end


local function data_trans()

    _, bt_connect = sys.waitUntil("BT_CONNECT_IND") --等待连接成功
    if bt_connect.result ~= 0 then
        log.info("bt","尝试连接蓝牙超时！！！")
        return false
    end
    log.info("bt", "蓝牙连接成功！！！")
    --修改蓝牙状态为已连接
    btStatus = true
    --链接成功
    conpck = protocol.contentDecode(pen_addr,true)
    mySocket.send(conpck)

    log.info("bt.connect_handle", bt_connect.handle)--蓝牙连接句柄
    log.info("bt", "find all service uuid")
    btcore.findservice()--发现所有16bit服务uuid
    _, result = sys.waitUntil("BT_FIND_SERVICE_IND") --等待发现uuid
    if not result then
        return false
    end

    btcore.findcharacteristic(0xFFF0)--服务uuid
    _, result = sys.waitUntil("BT_FIND_CHARACTERISTIC_IND") --等待发现服务包含的特征成功
    if not result then
        return false
    end
    btcore.opennotification(0xFFF1); --打开通知 对应特征uuid  

    --命令的UUID
    btcore.findcharacteristic(0xF100)--服务uuid
    _, result = sys.waitUntil("BT_FIND_CHARACTERISTIC_IND") --等待发现服务包含的特征成功
    if not result then
        return false
    end
    btcore.opennotification(0xF101); --打开通知 对应特征uuid 
      
    log.info("bt.rcv", "=============START RCV==============")
    
    local socketConnect=0
    while true do
        ret,bt_recv = sys.waitUntil("BT_DATA_IND",10000) --等待接收到数据
        if(ret == true) then --接收到事件
            local data = ""
            local uuid = ""
            while true do
                local recvuuid, recvdata, recvlen = btcore.recv(244)
                if recvlen == 0 then
                    break
                end
                uuid = recvuuid
                data = data .. recvdata
                log.info("——————————————————","收到数据")
                log.info("接收到的UUID", string.toHex(uuid))
                log.info("数据长度",string.format("%d",recvlen))
                -- log.info("数据长度复印",string.format("%d",recvlen))

                local testData = string.sub(recvdata, 1, 1)
                test2Data = string.toHex(testData)

                if(test2Data == 'C1')then --报告离线数据
                    offlineDataSizeStr = string.sub(recvdata, 3, 6)
                    log.info("数据类型：","离线数据长度-->" .. string.toHex(offlineDataSizeStr))
                elseif(test2Data == 'D1')then
                    log.info("数据类型：","压力数据")
                elseif(test2Data == 'E5')then  
                    log.info("数据类型：","笔型号")  
                elseif(test2Data == 'A9')then  
                    pow = string.toHex(string.sub(recvdata, 3,3))
                    sta = string.toHex(string.sub(recvdata, 4,4))
                    log.info("数据类型：","笔电量-->".. pow .."充电状态：" .. sta)  
                    penInfo = protocol.penInfoDecode(pen_addr,sta,pow)
                    mySocket.send(penInfo)

                elseif(test2Data == 'FE' or test2Data == 'FC')then
                    log.info("数据类型：","落笔点或移动点-->" .. string.toHex(data))
                    --local newPck = trackDecode(data,recvlen)
                    local newPck = protocol.trackDecode(pen_addr,data,recvlen)
                    log.info("新版封包：",string.toHex(newPck))
                    mySocket.send(newPck)
                elseif(test2Data == 'E1')then
                    log.info("数据类型：","点读码-->" .. string.toHex(data))
                else
                    log.info("未知数据：",string.toHex(data))         
                end
        
            end
        else -- 事件等待超时
            -- 判断是否蓝牙断开连接
            if btStatus == false then
                log.info("退出接收循环")
                break
            end
        end
    end
end


sys.taskInit(
    function()
        init()
        poweron()
        while true do
            connect()
            data_trans()
            sys.wait(1000)
        end
end)
