#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <zephyr/drivers/i2c.h>
#include <inttypes.h>
#include <stdbool.h>


#include <BTN.h>
#include <LED.h>

#include <j_assets.h>
#include <tots.h>
#include <j_controls.h>

#include <math.h>
#include <J_GL.h>
#include <J_ASSETS.h>

// LOW_HEIGHT - MAX_HEIGHT - LOW_LENGTH - MAX_LENGTH
uint16_t bounds[4] = {0x0000,LCD_MAX_HEIGHT,0x0000,LCD_MAX_LENGTH};

const struct gpio_dt_spec dcx_gpio = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE,dcx_gpios);
const struct spi_cs_control cs_ctrl = (struct spi_cs_control) {
  .gpio = GPIO_DT_SPEC_GET(ARDUINO_SPI_NODE,cs_gpios),
  .delay = 0u,
};

const struct device * dev = DEVICE_DT_GET(ARDUINO_SPI_NODE);
#define ARDUINO_I2C_NODE DT_NODELABEL(arduino_i2c)
const struct device * dev_i2c = DEVICE_DT_GET(ARDUINO_I2C_NODE);
const struct spi_config spi_cfg = {
  .frequency = 15000000,
  .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
  .slave = 0,
  .cs = cs_ctrl
};

int main(void)
{
    if(0 > BTN_init())
      return 0;
    if(0 > LED_init())
      return 0;
    J_init(dev,dev_i2c,&spi_cfg,&dcx_gpio,bounds);
    J_LCD_init();



    uint16_t x,y;
    x = y = 0;

    j_color BG_FILL = BLACK;

    j_component* BLACK_FILL = create_component("black_bg_fill",J_FILL,0,0,(void*)&BG_FILL,NULL);
    draw_component(BLACK_FILL);

    tots_init();
    draw_borders(true);
    
    while(1) {
        swipe_dir SWIPE = get_swipe_touch(100);
        switch(SWIPE) {
          case J_SWIPE_DOWN: 
            printk("Swiped down!\n");
            break;
          case J_SWIPE_UP:
            printk("Swiped up!\n");
            break;
          case J_SWIPE_LEFT:
            printk("Swiped left!\n");
            break;
          case J_SWIPE_RIGHT: 
            printk("Swiped right!\n");
            break;
          default:
            break;
        }
    }
    return 0;
}
