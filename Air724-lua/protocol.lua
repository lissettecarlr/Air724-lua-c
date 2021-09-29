module(...,package.seeall)

function str2hex(inputStr)
	--判断输入类型	
	if (type(inputStr)~="string") then
	    return nil,"str2hex invalid input type"
	end
	--滤掉分隔符
	inputStr=inputStr:gsub("[%s%p]",""):upper()
	--检查内容是否合法
	if(inputStr:find("[^0-9A-Fa-f]")~=nil) then
	    return nil,"str2hex invalid input content"
	end
	--检查字符串长度
	if(inputStr:len()%2~=0) then
	    return nil,"str2hex invalid input lenth"
	end
	--拼接字符串
	local index=1
	local ret=""
	for index=1,inputStr:len(),2 do
	    ret=ret..string.char(tonumber(inputStr:sub(index,index+1),16))
	end
	return ret
end

-- local newPck = heart_decode()
-- mySocket.send(newPck)
function heartDecode(mac)
    local head="F0AA0105"  
    --秒级时间戳
    local time = string.format("%08x",os.time()) 
    --local time="01020304"
    local mac = string.gsub(mac, ':',"")
    local pcklen = string.format("%04x",16)
    local pck=""
    pck = head .. pcklen ..mac .. time
    return str2hex(pck)
    -- return pck
end

-- 旧版本的轨迹封包
function oldTrackDecode(mac,data,len)
    local data_new
    local mac_addr = string.gsub(mac, ':',"")  --去除冒号
    local now_time = os.time()

    log.info("bt.mac_addr----------",mac_addr)
    strlen = string.format("%04x",len)
    -- 封装了AP基础包   
    data_new = str2hex("f0aa" .. "01" .. "ffff" .. mac_addr .. strlen)
    data_new = data_new   .. data
    return data_new
end


local PckNumber = 0
-- [首部]+[协议版本 协议类型]+[包长]+[MAC]+[秒级时间戳]+[包编号]+[原AP封装包]
function trackDecode(mac,data,len)
    -- F0AA0101 001E 112233445566 010203040506 0001 A1A2A3A4A5A6A7A8A9A0
    local head="F0AA0101"  
    --毫秒级时间戳
    --local time="010203040506"
    local timeS = string.format("%08x",os.time()) 
    local timeMs = string.format("%04x",getMS()) 
    log.info("毫秒时间戳--->",timeS..timeMs)

    local mac = string.gsub(mac, ':',"")
    local pcklen = string.format("%04x",len+6+6+6+2)
    PckNumber = PckNumber+1 
    local pckNo = string.format("%04x",PckNumber) 
    local temp = string.toHex(data)
    local pck=""
    pck = head .. pcklen ..mac .. timeS .. timeMs .. pckNo .. temp
    return str2hex(pck)
    -- return pck
end


--连接状态包
function contentDecode(mac,sta)
    local mac = string.gsub(mac, ':',"")
    local time = string.format("%08x",os.time()) 

    local head=""
    if(sta == true)then
        head = "F0AA01070010"
    else
        head = "F0AA01080010"
    end

    local pck=""
    pck = head ..mac .. time
    return str2hex(pck)   
end    
    
-- 笔信息包
function penInfoDecode(mac,sta,pow)

    local head="F0AA010A000E"
    local mac = string.gsub(mac, ':',"")

    -- local s = string.format("%02x",sta) 
    -- local p = string.format("%02x",pow) 
    local pck=""
    pck = head ..mac .. sta .. pow
    return str2hex(pck) 

end
-- 毫秒计时器

local msCount=0
local LastOsTime=0

function  millisecondTimerCount()
    if(LastOsTime == 0)then
        LastOsTime = os.time()
    end 
    nowOsTime = os.time()
    if(LastOsTime ~= nowOsTime)then
        msCount=0
        LastOsTime = nowOsTime
    end
    msCount = msCount +1 
end

function getMS()
    return msCount
end

function msTimerStart()
    sys.timerLoopStart(millisecondTimerCount,1)
end