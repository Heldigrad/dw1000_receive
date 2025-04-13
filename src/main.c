//*********************************************/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
//*********************************************/

#include "C:\Users\agape\Documents\LICENTA\new_1\functions\spi_operations.h"
#include "C:\Users\agape\Documents\LICENTA\new_1\functions\defines.h"
#include "C:\Users\agape\Documents\LICENTA\new_1\functions\dw_functions.h"
#include "C:\Users\agape\Documents\LICENTA\new_1\functions\includes.h"

int main(void)
{
    check_dev_status();
    reset_device();

    // initialize();
    // configure();
    dw1000_restore_default();
    while (1)
    {
        LOG_INF("SIMPLE_RX");

        // Device ID
        // uint8_t dev_id[4] = {0};
        // dw1000_spi_read(0x00, dev_id, sizeof(dev_id));

        rx_enable();

        // Wait for a valid frame (RXFCG bit in SYS_STATUS)
        uint32_t sys_status = {0};
        do
        {
            dw1000_spi_read(SYS_STATUS, sys_status);
            k_msleep(100);
        } while (!(sys_status & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))); // Check ERR bits | RXFCG bit

        if (sys_status & SYS_STATUS_RXFCG)
        {
            // Read received data
            LOG_INF("Received Data:");
            uint8_t rx_data[4];
            dw1000_spi_read(0x11, rx_data);

            // Clear status bit
            LOG_INF("Clearing status bit");
            dw1000_spi_write(SYS_STATUS, SYS_STATUS_RXFCG);
        }
        else
        {
            // Clear err bits
            LOG_INF("Clearing err bits");
            dw1000_spi_write(SYS_STATUS, SYS_STATUS_ALL_RX_ERR);
        }

        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}