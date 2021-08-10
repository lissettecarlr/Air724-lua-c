module(...,package.seeall)


local blCounter=0;
local blErrorConuter=0;
local TCPCounter=0;
local TCPErrorCounter=0;

local startDebug=0;

-- 计数统计
function Counter(newCounter)
    if(startDebug == 0)then
        startDebug =1
        blCounter = newCounter
        return false
    end
    if(blCounter < newCounter) then
        local  dif1 = newCounter - blCounter
        if(dif1 > 1 ) then
            blErrorConuter = blErrorConuter + dif1
            log.info("[1]丢失包新增--------：",dif1)
        end
    elseif(blCounter > newCounter) then
        local  dif2 = newCounter+ 247 - blCounter
        if(dif2 >1) then
            blErrorConuter = blErrorConuter + dif2
            log.info("[2]丢失包新增-------：",dif2)
        end
    else
        blErrorConuter = blErrorConuter + 255 

    end
    blCounter = newCounter
    return true
end

function getError()
    log.info("已经丢失包：",blErrorConuter)
end


local flag=0
-- 拆包获取计数值
function decode(data,lenth)
    -- 转化为字符串
    local strData = string.toHex(data)
    -- 循环获取对应数组
    local ct = lenth /10;
    for pos=0,ct-1,1 do
        local head  = string.sub(strData,pos*20+1,pos*20+2)
        log.info("头包是:",head)
        --落笔包没有序号，仅计数判断笔移动和抬起包
        if (head == "FC") then
            local pckCt  = string.sub(strData,pos*20+19,pos*20+20)
            --log.info("计数包",pckCt)
            local pckCounter = tonumber(pckCt,16)
            log.info("编号:",pckCounter);
            Counter(pckCounter)
        elseif (head == 'FE') then
            if(flag == 0)then
                flag =1
            else
                local downCt = tonumber((string.sub(strData,pos*20+11,pos*20+12)),16)
                log.info("落笔计数:",downCt)
                Counter(downCt)
                flag =0
            end
        end
        --读出序号
    end
    
end

