//*********************************************/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
//*********************************************/

#include "C:\Users\agape\Documents\LICENTA\functions\includes.h"

int main(void)
{
    if(check_devices_ready()){
        LOG_ERR("Devices not ready!");
        return 1;
    }

    reset_devices();

    initialize();
    // dw1000_default_config();

    //configure();

    while (1)
    {
        LOG_INF("SIMPLE_RX");

        // Device ID
        uint32_t dev_id;
        dw1000_read_u32(0x00, &dev_id);

        rx_enable();

        // Wait for a valid frame (RXFCG bit in SYS_STATUS)
        uint32_t sys_status = {0};
        do
        {
            dw1000_read_u32(SYS_STATUS, &sys_status);
            k_msleep(100);
        } while (!(sys_status & (SYS_STATUS_RX_OK | SYS_STATUS_ALL_RX_ERR))); // Check ERR bits | RXFCG bit

        if (sys_status & SYS_STATUS_RX_OK)
        {
            // Read received data
            LOG_INF("Received Data:");
            uint32_t rx_data;
            dw1000_read_u32(0x11, &rx_data);

            // Clear status bit
            LOG_INF("Clearing status bit");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_RXFCG);
        }
        else
        {
            // Clear err bits
            LOG_INF("Clearing err bits");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR);
        }

        k_msleep(SLEEP_TIME_MS);

    }
    
    return 0;
}
