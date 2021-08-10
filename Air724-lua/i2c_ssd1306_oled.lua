--- 模块功能：SSD 1306驱动芯片 I2C屏幕显示测试
-- @author CX
-- @module ui.mono_i2c_ssd1306
-- @license MIT
-- @copyright CX
-- @release 2020.02.23
module(..., package.seeall)
require "bit"

------------------------------------------------------------------------------------
-------------------------------用户配置区域------------------------------------------
------------------------------------------------------------------------------------
--local fontfile_ascii = "/ldata/ASCII_lib.bin" --字库文件的地址，2G模块
--local fontfile_gb2312 = "/ldata/GB2312_lib.bin" --字库文件的地址，2G模块

local fontfile_ascii = "/lua/ASCII_lib.bin" --字库文件的地址，4G模块
local fontfile_gb2312 = "/lua/GB2312_lib.bin" --字库文件的地址，4G模块

local iicspeed = i2c.SLOW --iic速度，100K
--local iicspeed = i2c.FAST --iic速度，400K。在720sl上点不亮，2g模块可以使用高速iic

local i2cid = 2 --iicid，需要根据接线而定，默认iic1
local i2cslaveaddr = 0x3C --从机地址，如果是八位地址，需要向右移动一位。我这个模块说明书给的地址是0x78，右移动一位就是0x3c

------------------------------------------------------------------------------------
-------------------------------函数说明---------------------------------------------
------------------------------------------------------------------------------------
--[[
* @description: 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
* @param {type} 无
* @return: 无
function OLED_Clear()
]]
--[[
* @description: 初始化OLED，
* @param {type} :无
* @return: 无
function OLED_Init()
]]
--[[
* @description: 在指定位置显示一个字符,包括部分字符
* @param {type} :-----x:0~127
----------------------y:0~6
----------------------0第一行，2第二行，4第三行，6第四行
----------------------Show_char:要显示的字符
----------------------isChinese:是中文就填入1，英文就忽略
* @return: 无
]]
--[[
* @description: 在指定坐标起始处显示字符串
* @param {type} :-----x:0~127
----------------------y:0~6
----------------------0第一行，2第二行，4第三行，6第四行
----------------------Show_char:要显示的字符串，支持中文
* @return: 无
]]
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-------------------------------下面都不要管了----------------------------------------
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
local OLED_CMD = 0
local OLED_DATA = 1
local SIZE = 16
local Max_Column = 128
local Max_Row = 64
local X_WIDTH = 128
local Y_WIDTH = 64

local function OLED_Write_Command(OLED_Byte)
    i2c.send(i2cid, i2cslaveaddr, {0x00, OLED_Byte})
end

local function OLED_Write_Data(OLED_Byte)
    i2c.send(i2cid, i2cslaveaddr, {0x40, OLED_Byte})
end

local function OLED_WR_Byte(OLED_Byte, OLED_Type)
    if OLED_Type == OLED_DATA then
        OLED_Write_Data(OLED_Byte)
    else
        OLED_Write_Command(OLED_Byte)
    end
end

--[[
* @description: 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
* @param {type} 无
* @return: 无
]]
function OLED_Clear()
    local N_Page, N_row = 0, 0
    for N_Page = 1, 8 do
        OLED_WR_Byte(0xb0 + N_Page - 1, OLED_CMD)
        OLED_WR_Byte(0x00, OLED_CMD)
        OLED_WR_Byte(0x10, OLED_CMD)
        for N_row = 1, 128 do
            OLED_WR_Byte(0x00, OLED_DATA)
        end
    end
end


local function OLED_Set_Pos(x, y)
    OLED_WR_Byte(0xb0 + y, OLED_CMD)
    OLED_WR_Byte(bit.band(x, 0x0f), OLED_CMD)
    OLED_WR_Byte(bit.bor(bit.rshift(bit.band(x, 0xf0), 4), 0x10), OLED_CMD)
end

--[[
* @description: 初始化OLED，
* @param {type} :无
* @return: 无
]]
function OLED_Init()
    if i2c.setup(i2cid, iicspeed) ~= iicspeed then
        print("testI2c.init fail")
        return
    end
    OLED_WR_Byte(0xAE, OLED_CMD)
    OLED_WR_Byte(0x00, OLED_CMD)
    OLED_WR_Byte(0x10, OLED_CMD)
    OLED_WR_Byte(0x40, OLED_CMD)
    OLED_WR_Byte(0xB0, OLED_CMD)
    OLED_WR_Byte(0x81, OLED_CMD)
    OLED_WR_Byte(0xFF, OLED_CMD)
    OLED_WR_Byte(0xA1, OLED_CMD)
    OLED_WR_Byte(0xA6, OLED_CMD)
    OLED_WR_Byte(0xA8, OLED_CMD)
    OLED_WR_Byte(0x3F, OLED_CMD)
    OLED_WR_Byte(0xC8, OLED_CMD)
    OLED_WR_Byte(0xD3, OLED_CMD)
    OLED_WR_Byte(0x00, OLED_CMD)
    OLED_WR_Byte(0xD5, OLED_CMD)
    OLED_WR_Byte(0x80, OLED_CMD)
    OLED_WR_Byte(0xD9, OLED_CMD)
    OLED_WR_Byte(0xF1, OLED_CMD)
    OLED_WR_Byte(0xDA, OLED_CMD)
    OLED_WR_Byte(0x12, OLED_CMD)
    OLED_WR_Byte(0xDB, OLED_CMD)
    OLED_WR_Byte(0x40, OLED_CMD)
    OLED_WR_Byte(0x8D, OLED_CMD)
    OLED_WR_Byte(0x14, OLED_CMD)
    OLED_WR_Byte(0xAF, OLED_CMD)
    OLED_Clear()
    OLED_Set_Pos(0, 0)
end


--[[
* @description: 在指定位置显示一个字符,包括部分字符
* @param {type} :-----x:0~127
----------------------y:0~6
----------------------0第一行，2第二行，4第三行，6第四行
----------------------Show_char:要显示的字符
----------------------isChinese:是中文就填入1，英文就忽略
* @return: 无
]]
function OLED_ShowChar(x, y, Show_char, isChinese)
    local c, i = 0, 0
    if x > Max_Column - 1 then
        x = 0
        y = y + 2
    end
    local fontfile, error = nil, nil
    if (isChinese ~= 1) then
        fontfile, error = io.open(fontfile_ascii, "r")
        c = string.byte(Show_char) - 0x20
    else
        --log.info("OLED_ShowChar", "loadseq:", Show_char)
        fontfile, error = io.open(fontfile_gb2312, "r")
        c = Show_char
    end
    if fontfile == nil then
        --log.info("OLED_ShowChar", "fontfile-error:", error)
        return
    end
    fontfile:seek("set", 32 * c)
    local F8X16 = fontfile:read(32)
    io.close(fontfile)
    --log.info("ssd1306", "fontdata:", F8X16:toHex())
    if SIZE == 16 then
        OLED_Set_Pos(x, y)
        for i = 1, 8 do
            if (isChinese ~= 1) then
                --log.info("ssd1306", "left:", string.char(string.byte(F8X16, i * 2 - 1)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2 - 1), OLED_DATA)
            else
                --log.info("ssd1306", "left:", string.char(string.byte(F8X16, i * 2 - 1)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2 - 1), OLED_DATA)
            end
        end
        OLED_Set_Pos(x, y + 1)
        for i = 1, 8 do
            if (isChinese ~= 1) then
                --log.info("ssd1306", "left:", string.char(string.byte(F8X16, i * 2)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2), OLED_DATA)
            else
                --log.info("ssd1306", "left:", string.char(string.byte(F8X16, i * 2)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2), OLED_DATA)
            
            end
        end
        OLED_Set_Pos(x + 8, y)
        for i = 1, 8 do
            if (isChinese == 1) then
                --log.info("ssd1306", "right:", string.char(string.byte(F8X16, i * 2 + 15)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2 + 15), OLED_DATA)
            end
        end
        OLED_Set_Pos(x + 8, y + 1)
        for i = 1, 8 do
            if (isChinese == 1) then
                --log.info("ssd1306", "right:", string.char(string.byte(F8X16, i * 2 + 16)):toHex())
                OLED_WR_Byte(string.byte(F8X16, i * 2 + 16), OLED_DATA)
            end
        end
    end
end


--[[
* @description: 在指定坐标起始处显示字符串
* @param {type} :-----x:0~127
----------------------y:0~6
----------------------0第一行，2第二行，4第三行，6第四行
----------------------Show_char:要显示的字符串，支持中文
* @return: 无
]]
function OLED_ShowString(x, y, ShowStr)
    local len = #ShowStr
    local N_Char = 1
    while N_Char <= len do
        local char_l = string.char(string.byte(ShowStr, N_Char))
        local char_r = ""
        if (N_Char < len) then
            char_r = string.char(string.byte(ShowStr, N_Char + 1))
        else
            char_r = " "
        end
        --log.info("OLED_ShowChar", len, char_l:toHex() .. " " .. char_r:toHex())
        if (char_l:byte() < 0xA1) then
            OLED_ShowChar(x, y, char_l, nil)
        else
            local seq = (char_l:byte() - 0xA1) * 94 + char_r:byte() - 0xA0 - 1
           -- log.info("OLED_ShowChar", "Seq is ", seq)
            OLED_ShowChar(x, y, seq, 1)
            N_Char = N_Char + 1
            x = x + 8
        end
        x = x + 8
        if x >= 128 then
            x = 0
            y = y + 2
        end
        N_Char = N_Char + 1
    end
end
