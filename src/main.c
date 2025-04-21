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

    // soft reset
    // uint8_t softreset = 0x00;
    // dw1000_subwrite_u8(PMSC, 0x04, softreset);

    // initialize();
    dw1000_default_config();

    // configure();

    while (1)
    {
        LOG_INF("SIMPLE_RX");

        // Device ID
        uint32_t dev_id;
        dw1000_read_u32(0x00, &dev_id);

        uint32_t tx_fctrl = 0;
        tx_fctrl |= (6 & 0x7F); // TFLEN
        tx_fctrl |= (0 << 7);   // TFLE = 0
        tx_fctrl |= (0 << 10);  // Reserved = 0
        tx_fctrl |= (2 << 13);  // TXBR = 10 (6.8 Mbps)
        tx_fctrl |= (1 << 16);  // TXPRF = 01 (16 MHz)
        tx_fctrl |= (1 << 18);  // TXPSR = 01
        tx_fctrl |= (1 << 20);  // PE = 01
        tx_fctrl |= (0 << 22);  // TXBOFFS = 0x000

        dw1000_write_u32(TX_FCTRL, tx_fctrl);

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
            LOG_INF("Reception success!\nReceived Data:");
            uint32_t rx_data;
            dw1000_read_u32(0x11, &rx_data);

            // Clear status bit
            LOG_INF("Clearing status bit...");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_RXFCG);
        }
        else
        {
            // Clear err bits
            LOG_INF("Clearing err bits...");
            dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR);
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
