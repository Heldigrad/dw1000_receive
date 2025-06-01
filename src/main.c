//*********************************************/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
// https://github.com/foldedtoad/dwm1001/tree/master
//*********************************************/

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

    bip_init();
    bip_config();

    set_rx_antenna_delay(RX_ANT_DLY);
    set_tx_antenna_delay(TX_ANT_DLY);

    uint64_t buffer, T2, T3, aux;
    dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

    int ok = 1;

    dw1000_subwrite_u40(TX_TIME, 0x00, 0x00);
    dw1000_subwrite_u40(RX_TIME, 0x00, 0x00);

    while (1)
    {
        LOG_INF("\n\n");

        if (receive(&buffer, &T2) == SUCCESS)
        {
            LOG_INF("RX Success!");

            if (buffer == POLL_MSG)
            {
                LOG_INF("Poll message received! Sending timestamp 2.");
                if (transmit(T2, 5, &T3) == SUCCESS)
                {
                    LOG_INF("Response transmitted successfully! Sending T3...");
                    if (transmit(T3, 5, &aux) == SUCCESS)
                    {
                        LOG_INF("All messages were transmitted.");
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        //  k_msleep(RX_SLEEP_TIME_MS);
    }

    return 0;
}