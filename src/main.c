#include "C:\Users\agape\Documents\LICENTA\functions\devices.h"
#include "C:\Users\agape\Documents\LICENTA\functions\dw1000_ranging_functions.h"

// #include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\devices.h"
// #include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\dw1000_ranging_functions.h"

uint8_t Dev_id = 0x04;

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

    initialize();
    configure();

    set_antenna_delay(Dev_id);

    uint64_t T2, T3, T6;
    uint8_t msg_id = 0;

    while (1)
    {
        dw1000_write_u8(SYS_CTRL, SYS_CTRL_TRXOFF);
        dw1000_write_u32(SYS_STATUS, 0xFFFFFFFF);

        T2 = 0;
        T3 = 0;
        T6 = 0;

        dw1000_subwrite_u40(RX_TIME, 0x00, 0x00);
        dw1000_subwrite_u40(TX_TIME, 0x00, 0x00);

        while (1)
        {
            get_msg_from_init(Dev_id, &T2, &T3, &T6, &msg_id);

            dw1000_write_u32(SYS_STATUS, SYS_STATUS_ALL_RX_ERR | SYS_STATUS_RX_OK);
            rx_soft_reset();
        }
    }
}