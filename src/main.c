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

    bip_init();
    bip_config();

    set_rx_antenna_delay(RX_ANT_DLY);
    set_tx_antenna_delay(TX_ANT_DLY);

    double distance;
    uint64_t buffer, T1, T2, T3, T4, aux;
    dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

    while (1)
    {
        dw1000_subwrite_u40(TX_TIME, 0x00, 0x00);
        dw1000_subwrite_u40(RX_TIME, 0x00, 0x00);

        if (receive(&buffer, &T2) == SUCCESS)
        {

            if (buffer == POLL_MSG)
            {
                transmit(0x1020304050, 5, &T3);

                uint32_t status_reg;
                dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

                new_rx_enable(0);

                do
                {
                    dw1000_read_u32(SYS_STATUS, &status_reg);
                } while (!(status_reg & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

                if (!(status_reg & SYS_STATUS_ALL_RX_ERR) && (status_reg & SYS_STATUS_RXFCG))
                {
                    dw1000_subread_u40(RX_BUFFER, 0x00, &T1);
                    dw1000_subread_u40(RX_BUFFER, 0x05, &T4);
                    distance = compute_distance(T1, T2, T3, T4);
                    LOG_INF("T1 = %llX, T2 = %llX, T3 = %llX, T4 = %llX, Distance = %f m", T1, T2, T3, T4, distance);
                    /* Clear good RX frame event in the DW1000 status register. */
                    dw1000_write_u32(SYS_STATUS, SYS_STATUS_RXFCG);
                }
                else
                {
                    /* Clear RX error events in the DW1000 status register. */
                    dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR);
                    rx_soft_reset();
                }
            }
        }
    }
}