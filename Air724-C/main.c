#include "iot_debug.h"
#include "iot_os.h"
#include "stdbool.h"
#include "debug.h"
#include "stdlib.h"
#include "stdio.h"
#include "iot_pmd.h"

HANDLE main_task_handle=NULL;
extern void bleStart(void);
extern void socket_init(void);

static void main_task(PVOID pParameter)
{

    while(true)
    {
        iot_os_sleep(10000);
        app_debug_print("------------");
    }
    iot_os_delete_task(main_task_handle);
}

int appimg_enter(void *param)
{
#if CONFIG_APP_DEBUG == 1
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
#endif // CONFIG_APP_DEBUG
    iot_pmd_exit_deepsleep();
    app_debug_init();
    main_task_handle = iot_os_create_task(main_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "main");
    //bleStart();
    socket_init();
    return 0;
}
void appimg_exit(void)
{
    app_debug_print("exit");
}
