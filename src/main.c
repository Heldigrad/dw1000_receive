//*********************************************/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
//*********************************************/

#include "C:\Users\agape\Documents\LICENTA\dw1000_app\DW1000-driver\includes.h"

int main(void)
{
    if (check_devices_ready())
    {
        LOG_ERR("Devices not ready!");
        return 1;
    }
    gpio_pin_configure_dt(&reset_gpio, GPIO_OPEN_DRAIN | GPIO_OUTPUT);
    reset_devices();

    while (1)
    {
        LOG_INF("\n\n");
        LOG_INF("RX");

        // Device ID
        // uint32_t dev_id;
        // dw1000_read_u32(0x00, &dev_id);

        reset_devices();

        generic_default_configs(4);
        rx_default_configs();
        additional_default_configs();

        rx_enable();

        // Wait for a valid frame (RXFCG bit in SYS_STATUS)
        uint32_t sys_status = {0};
        do
        {
            dw1000_read_u32(SYS_STATUS, &sys_status);
            k_msleep(10);
        } while (!(sys_status & (SYS_STATUS_RX_OK | SYS_STATUS_ALL_RX_ERR))); // Check ERR bits | RXFCG bit

        if (sys_status & SYS_STATUS_RXFCG)
        {
            // Read received data
            uint64_t T2 = get_rx_timestamp();
            LOG_INF("Reception success! T2 = %X", T2);
            uint32_t rx_data;
            dw1000_read_u32(0x11, &rx_data);

            if (rx_data == POLL_MSG)
            {
                LOG_INF("Ranging initiated!");
            }

            // Clear status bit
            // LOG_INF("Clearing status bit...");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_RXFCG);
            k_msleep(1);
            // Send response with RX timestamp
            // reset_devices();

            // generic_default_configs(5);
            // tx_default_configs();
            // // additional_default_configs();

            // // Send a message with the T2 buffer
            // dw1000_write_u32(TX_BUFFER, T2);

            // tx_start();

            // // SYSTEM EVENT = 0x0F -> wait for transmission completion
            // uint32_t sys_status_1;
            // do
            // {
            //     dw1000_read_u32(SYS_STATUS, &sys_status_1);
            //     k_msleep(10);
            // } while (!(sys_status_1 & SYS_STATUS_TXFRS)); // Check TXFRS bit

            // uint64_t T3 = get_tx_timestamp();
            // LOG_INF("Transmission complete! T3 = %X", T3);

            // // SYSTEM EVENT = 0x0F -> clear TXFRS flag
            // dw1000_write_u32(SYS_STATUS, SYS_STATUS_TXFRS);
        }
        else
        {
            // Clear err bits
            LOG_INF("Reception failed. Resend message! Clearing err bits...");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR);
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
