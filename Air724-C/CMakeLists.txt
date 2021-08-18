if(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${flash_ldscript} main.c debug.c ble.c fifo.c atUart.c AtProtocol_6266.c)
elseif(CONFIG_APPIMG_LOAD_FILE)
add_appimg(${target} ${file_ldscript} main.c debug.c ble.c fifo.c atUart.c AtProtocol_6266.c)
endif()



