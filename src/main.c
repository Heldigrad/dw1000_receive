//*********************************************/
// RX
//https://github.com/RT-LOC/zephyr-dwm1001/blob/master/examples/ex_02a_simple_rx/ex_02a_main.c
//https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/ieee802154/ieee802154_dw1000.c
//*********************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dw1000_spi, LOG_LEVEL_DBG);

#define SLEEP_TIME_MS 1000

// SPI Configuration
#define DW1000_SPI_FREQUENCY 2000000                    // 2 MHz
#define DW1000_SPI_MODE (SPI_MODE_CPOL | SPI_MODE_CPHA) // SPI Mode 0 (CPOL = 0, CPHA = 0)

// SPI device and configuration
const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
const struct spi_config spi_cfg = {
    .frequency = DW1000_SPI_FREQUENCY,
    .operation = SPI_WORD_SET(8) | DW1000_SPI_MODE,
    .slave = 0,
    .cs = NULL, // CS handled via GPIO
};

#define SPIOP SPI_WORD_SET(8) | SPI_TRANSFER_MSB
struct spi_dt_spec spispec = SPI_DT_SPEC_GET(DT_NODELABEL(ieee802154), SPIOP, 0);

// GPIO for Chip Select (CS) and reset
const struct gpio_dt_spec cs_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(spi2), cs_gpios);
const struct gpio_dt_spec reset_gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(ieee802154), reset_gpios);

// SPI write
int dw1000_spi_write(const struct device *spi_dev, uint8_t reg, uint8_t *data, size_t len)
{
    uint8_t tx_buf[1 + len];       // Register header + data
    tx_buf[0] = 0x80 | reg;        // Op. bit + address
    memcpy(&tx_buf[1], data, len); // Data to be written

    struct spi_buf tx_bufs[] = {
        {.buf = tx_buf, .len = sizeof(tx_buf)},
    };

    struct spi_buf_set tx = {.buffers = tx_bufs, .count = 1};

    gpio_pin_set_dt(&cs_gpio, 0); // Assert CS
    int ret = spi_write_dt(&spispec, &tx);
    gpio_pin_set_dt(&cs_gpio, 1); // Deassert CS

    if (ret)
    {
        LOG_ERR("SPI write failed: %d", ret);
    }
    return ret;
}

int dw1000_spi_subwrite(const struct device *spi_dev, uint8_t reg, uint8_t subreg, uint8_t *data, size_t len)
{
    uint8_t tx_buf[2 + len];        // (Header + address) + sub-address + data

    tx_buf[0] = 0xC0 | reg;         // Op + address
    tx_buf[1] = subreg;             // Sub-address
    memcpy(&tx_buf[2], data, len);  // Data

    struct spi_buf tx_bufs[] = {
        {.buf = tx_buf, .len = sizeof(tx_buf)},
    };

    struct spi_buf_set tx = {.buffers = tx_bufs, .count = 1};

    gpio_pin_set_dt(&cs_gpio, 0); // Assert CS
    int ret = spi_write_dt(&spispec, &tx);
    gpio_pin_set_dt(&cs_gpio, 1); // Deassert CS

    if (ret) {
        LOG_ERR("SPI sub-register write failed: %d", ret);
    }
    return ret;
}

int dw1000_spi_read(const struct device *spi_dev, uint8_t reg, uint8_t *data, size_t len)
{
    uint8_t tx_buf[1];
    tx_buf[0] = reg & 0x3F; // Read operation: MSB=0
    // tx_buf[1] = 0;

    struct spi_buf tx_bufs[] = {
        {.buf = &tx_buf, .len = 1}, // Send register address
    };
    struct spi_buf rx_bufs[] = {
        {.buf = NULL, .len = 1},
        {.buf = data, .len = len}, // Receive data
    };

    struct spi_buf_set tx = {.buffers = tx_bufs, .count = 1};
    struct spi_buf_set rx = {.buffers = rx_bufs, .count = 2};

    gpio_pin_set_dt(&cs_gpio, 0); // Assert CS
    k_msleep(1);
    int ret = spi_transceive_dt(&spispec, &tx, &rx);
    k_msleep(1);
    gpio_pin_set_dt(&cs_gpio, 1); // Deassert CS

    if (ret)
    {
        LOG_ERR("SPI read failed: %d", ret);
        return ret;
    }

    // Log the received data
    LOG_INF("SPI read successful. Data from register 0x%X: ", reg);
    for (size_t i = 0; i < len; i++)
    {
        LOG_INF("Byte %zu: 0x%02X", i, data[i]);
    }

    return 0;
}

int main(void)
{
    int ret;

    // Check if SPI device is ready
    if (!device_is_ready(spi_dev))
    {
        LOG_ERR("SPI device not ready");
        return 1;
    }

    // Configure CS GPIO
    if (!device_is_ready(cs_gpio.port))
    {
        LOG_ERR("CS GPIO device not ready");
        return 1;
    }
    ret = gpio_pin_configure_dt(&cs_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret)
    {
        LOG_ERR("Failed to configure CS GPIO: %d", ret);
        return 1;
    }
    LOG_INF("CS GPIO configured...");

    gpio_pin_set_dt(&reset_gpio, 0);
    k_msleep(2);
    gpio_pin_set_dt(&reset_gpio, 1);
    k_msleep(5);

    while (1)
    {
        LOG_INF("\n[RX]");

        // Device ID
        uint8_t dev_id[4] = {0};
        dw1000_spi_read(spi_dev, 0x00, dev_id, sizeof(dev_id));

        // CHAN_CTRL = 0x1F
        uint8_t chan_ctrl[4] = {0x11, 0x00, 0x44, 0x08}; // Channel 1, preamble code 1
        dw1000_spi_write(spi_dev, 0x1F, chan_ctrl, sizeof(chan_ctrl));

        // DRX_TUNE0b = 0x27 : 02
        uint8_t drx_tune0b[2] = {0x02, 0x00};
        dw1000_spi_subwrite(spi_dev, 0x27, 0x02, drx_tune0b, sizeof(drx_tune0b));

        // DRX_TUNE1a = 0x27 : 04
        uint8_t drx_tune1a[2] = {0x87, 0x00};
        dw1000_spi_subwrite(spi_dev, 0x27, 0x04, drx_tune1a, sizeof(drx_tune1a));

        // DRX_TUNE1b = 0x27 : 06
        uint8_t drx_tune1b[2] = {0x10, 0x00};
        dw1000_spi_subwrite(spi_dev, 0x27, 0x06, drx_tune1b, sizeof(drx_tune1b));

        // DRX_TUNE2 = 0x27 : 08
        uint8_t drx_tune2[4] = {0x2D, 0x00, 0x1A, 0x31}; // PAC size = 8 
        dw1000_spi_subwrite(spi_dev, 0x27, 0x08, drx_tune2, sizeof(drx_tune2));

        // SYS_CTRL = 0x0D
        uint8_t sys_ctrl[1] = {0x10}; 
        dw1000_spi_write(spi_dev, 0x0D, sys_ctrl, sizeof(sys_ctrl));

        while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)))
        { };


        // Wait for a valid frame (RXFCG bit in SYS_STATUS)
        uint8_t sys_status[4] = {0};
        do
        {
            dw1000_spi_read(spi_dev, 0x0F, sys_status, sizeof(sys_status));
        } while (!(sys_status[0] & (0x4000 | 0x42))); // Check RXFCG bit

        // Read received data
        uint8_t rx_data[4]; 
        dw1000_spi_read(spi_dev, 0x11, rx_data, sizeof(rx_data));

        LOG_INF("Received Data:");
        for (size_t i = 0; i < sizeof(rx_data); i++)
        {
            LOG_INF("Byte %zu: 0x%02X", i, rx_data[i]);
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}