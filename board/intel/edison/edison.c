#include <common.h>
#include <asm/cache.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <intel_scu_ipc.h>
#include <watchdog.h>

#define IPC_WATCHDOG 0xF8

enum {
    SCU_WATCHDOG_START = 0,
    SCU_WATCHDOG_STOP,
    SCU_WATCHDOG_KEEPALIVE,
    SCU_WATCHDOG_SET_ACTION_ON_TIMEOUT
};

int board_usb_init(int index, enum usb_init_type init)
{
    return usb_gadget_init_udc();
}

void hw_watchdog_reset(void)
{
    /*
     *if (intel_scu_ipc_simple_command(IPC_WATCHDOG, SCU_WATCHDOG_KEEPALIVE))
     *    error("%s failed\n", __func__);
     */
}

int watchdog_disable(void)
{
    return (intel_scu_ipc_simple_command(IPC_WATCHDOG, SCU_WATCHDOG_STOP));
}

int watchdog_init(void)
{
    return (intel_scu_ipc_simple_command(IPC_WATCHDOG, SCU_WATCHDOG_START));
}
