//*********************************************/
// TX
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

int dw1000_spi_read(const struct device *spi_dev, uint8_t reg, uint8_t *data, size_t len)
{
    uint8_t tx_buf[2];
    tx_buf[0] = reg & 0x3F; // Read operation: MSB=0
    tx_buf[1] = 0;

    struct spi_buf tx_bufs[] = {
        {.buf = &tx_buf, .len = 1}, // Send register address
    };
    struct spi_buf rx_bufs[] = {
        {.buf = data, .len = len}, // Receive data
    };

    struct spi_buf_set tx = {.buffers = tx_bufs, .count = 1};
    struct spi_buf_set rx = {.buffers = rx_bufs, .count = 1};

    gpio_pin_set_dt(&cs_gpio, 0); // Assert CS
    int ret = spi_transceive_dt(&spispec, &tx, &rx);
    gpio_pin_set_dt(&cs_gpio, 1); // Deassert CS

    if (ret)
    {
        LOG_ERR("SPI read failed: %d", ret);
        return ret;
    }

    // Log the received data
    LOG_INF("SPI read successful. Data from register 0x%X: ", reg);
    for (size_t i = 1; i < len; i++)
    {
        LOG_INF("Byte %zu: 0x%02X", i - 1, data[i]);
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
        LOG_INF("\n[TX]");

        // Read and verify Device ID
        uint8_t dev_id[4] = {0};
        dw1000_spi_read(spi_dev, 0x00, dev_id, sizeof(dev_id));

        // Transmit Data
        uint8_t tx_data[4] = {0xDE, 0xDE, 0xDE, 0xDE};             // Example payload
        dw1000_spi_write(spi_dev, 0x09, tx_data, sizeof(tx_data)); // Write to TX_BUFFER (0x09)

        uint8_t chan_ctrl[4] = {0x25, 0x00, 0x00, 0x00}; // Channel 5, Preamble Code 3
        dw1000_spi_write(spi_dev, 0x1F, chan_ctrl, sizeof(chan_ctrl));

        // Configure transmission settings (TRANSMIT FRAME CONTROL = 0x08)
        uint8_t tx_fctrl[4] = {0x06, 0x00, 0x42, 0x00}; // Correct frame length
        dw1000_spi_write(spi_dev, 0x08, tx_fctrl, sizeof(tx_fctrl));

        // Start transmission (SYSTEM CONTROL = 0x0D)
        uint8_t sys_ctrl[1] = {0x02}; // Set TXSTRT bit
        dw1000_spi_write(spi_dev, 0x0D, sys_ctrl, sizeof(sys_ctrl));

        // Wait for transmission completion (SYSTEM EVENT = 0x0F)
        uint8_t sys_status[4] = {0};
        do
        {
            dw1000_spi_read(spi_dev, 0x0F, sys_status, sizeof(sys_status));
        } while (!(sys_status[0] & 0x80)); // Check TXFRS bit

        LOG_INF("Transmission complete!");

        // Clear TXFRS flag in SYSTEM EVENT (0x0F)
        uint8_t clear_status[4] = {0x80, 0x00, 0x00, 0x00}; // Clear only TXFRS bit
        dw1000_spi_write(spi_dev, 0x0F, clear_status, sizeof(clear_status));

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
