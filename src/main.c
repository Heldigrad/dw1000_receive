//**/
// RX
// https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
// https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
// https://github.com/foldedtoad/dwm1001/tree/master
//**/

// #include "C:\Users\agape\Documents\LICENTA\functions\devices.h"
// #include "C:\Users\agape\Documents\LICENTA\functions\dw1000_ranging_functions.h"

#include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\devices.h"
#include "C:\Users\agape\Documents\LICENTA\dw1000_app\functions\dw1000_ranging_functions.h"

uint8_t Dev_id = 0x04;

int main(void)
{
    if (check_devices_ready())
    {
        LOG_ERR("Devices not ready!");
        return 1;
    }
    gpio_pin_configure_dt(&reset_gpio, GPIO_OPEN_DRAIN | GPIO_OUTPUT);
    reset_devices();

    LOG_INF("RESPONDER %0d", Dev_id);

    bip_init();
    bip_config();

    set_antenna_delay(Dev_id);

    double distance;
    uint64_t T1, T2, T3, T4;
    uint8_t msg_id = 0;

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
            get_msg_from_init(Dev_id, &T1, &T2, &T3, &T4, &msg_id);
        } while (T1 == 0 || T2 == 0 || T3 == 0 || T4 == 0);

        if (INFO_LOGS_EN)
        {
            LOG_INF("T1 = %0llX, T2 = %0llX, T3 = %0llX, T4 = %0llX", T1, T2, T3, T4);
        }

        distance = compute_distance_meters(T1, T2, T3, T4);

        // if (distance < 100 && distance > 0.2)
        {
            k_msleep(10);

            if (INFO_LOGS_EN)
            {
                LOG_INF("Distance nr. %0d = %0.2fm", msg_id, distance);
            }

            uint32_t distance_mm = (uint32_t)(distance * 1000.0);

            send_distance(Dev_id, distance_mm);
        }
    }
}