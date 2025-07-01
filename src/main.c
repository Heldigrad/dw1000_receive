//**/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
// https://github.com/foldedtoad/dwm1001/tree/master
//**/

#include "C:\Users\agape\Documents\LICENTA\functions\devices.h"
#include "C:\Users\agape\Documents\LICENTA\functions\dw1000_ranging_functions.h"

// #include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\devices.h"
// #include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\dw1000_ranging_functions.h"

uint8_t Dev_id = 0x01;

int main(void)
{
    if (check_devices_ready())
    {
        LOG_ERR_IF_ENABLED("Devices not ready!");
        return 1;
    }
    gpio_pin_configure_dt(&reset_gpio, GPIO_OPEN_DRAIN | GPIO_OUTPUT);
    reset_devices();

    LOG_INF_IF_ENABLED("RESPONDER %0d", Dev_id);

    bip_init();
    bip_config();

    set_antenna_delay(Dev_id);

    double distance;
    uint64_t T2, T3, T6;
    uint8_t msg_id = 0;

    while (1)
    {
        dw1000_write_u8(SYS_CTRL, SYS_CTRL_TRXOFF);
        dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

        T2 = 0;
        T3 = 0;
        T6 = 0;

        while (1)
        {
            get_msg_from_init(Dev_id, &T2, &T3, &T6, &msg_id);

            dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR | SYS_STATUS_RX_OK);
            rx_soft_reset();
        }
    }
}