//*********************************************/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
// https://github.com/foldedtoad/dwm1001/tree/master
//*********************************************/

// #include "C:\Users\agape\Documents\LICENTA\functions\devices.h"
// #include "C:\Users\agape\Documents\LICENTA\functions\dw1000_ranging_functions.h"

#include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\devices.h"
#include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\dw1000_ranging_functions.h"

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

    bip_init();
    bip_config();

    set_rx_antenna_delay(RX_ANT_DLY);
    set_tx_antenna_delay(TX_ANT_DLY);

    uint64_t buffer, T2, T3, aux;
    dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

    dw1000_subwrite_u40(TX_TIME, 0x00, 0x00);
    dw1000_subwrite_u40(RX_TIME, 0x00, 0x00);

    for (int i = 0; i < 10; ++i)
    {
        LOG_INF("\n\n");

        if (receive(&buffer, &T2) == SUCCESS)
        {
            if (buffer == POLL_MSG)
            {
                /* Compute final message transmission time. See NOTE 7 below. */
                T3 = (T2 + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

                // set_delayed_trx_time(T3);

                /* Response TX timestamp is the transmission time we
                 * programmed plus the antenna delay.
                 */
                T3 = (((uint64_t)(T3 & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

                transmit(T2, 5, &aux);
                k_msleep(RX_SLEEP_TIME_MS);
                transmit(T3, 5, &aux);
            }
        }

        //  k_msleep(RX_SLEEP_TIME_MS);
    }

    return 0;
}