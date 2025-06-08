//**/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
// https://github.com/foldedtoad/dwm1001/tree/master
//**/

#include "C:\Users\agape\Documents\LICENTA\functions\devices.h"
#include "C:\Users\agape\Documents\LICENTA\functions\dw1000_ranging_functions.h"

int main(void)
{
    if (check_devices_ready())
    {
        LOG_ERR("Devices not ready!");
        return 1;
    }
    gpio_pin_configure_dt(&reset_gpio, GPIO_OPEN_DRAIN | GPIO_OUTPUT);
    reset_devices();

    LOG_INF("RX");

    uint8_t Dev_id = 0x01;

    bip_init();
    bip_config();

    set_rx_antenna_delay(RX_ANT_DLY);
    set_tx_antenna_delay(TX_ANT_DLY);

    // set_rx_timeout(RESP_RX_TIMEOUT_UUS * 10000);

    double distance;
    uint64_t T1, T2, T3, T4;
    dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

    uint8_t Msg_id = 0;

    while (1)
    {
        dw1000_write_u8(SYS_CTRL, SYS_CTRL_TRXOFF);
        dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

        T1 = 0;
        T2 = 0;
        T3 = 0;
        T4 = 0;
        do
        {
            get_msg_from_init(&T1, &T2, &T3, &T4);
        } while (T1 == 0 || T2 == 0 || T3 == 0 || T4 == 0);

        distance = compute_distance(T1, T2, T3, T4);
        LOG_INF("Distance = %0f", distance);
    }
}