# Game Over - nRF52840

This project runs a touchscreen game on the nRF52840 development kit using Zephyr and [J Graphics Library](https://github.com/elitetq/J-Graphics-Library), my own personal graphics library reused here. The game is called TOTS. It uses swipe controls to move through a maze, collect points and avoid enemies.

## Hardware

- nRF52840 DK with its on-board debug probe and a USB data cable.
- The 240x320 SPI display with capacitive I2C touch supported by J_GL (the library documents the Adafruit 2.8-inch display).
- Shared ground and display power connected according to the display's specifications.

The display uses `arduino_spi`, and the touch controller uses `arduino_i2c`. The [board overlay](app/boards/nrf52840dk_nrf52840.overlay) selects P1.11 for display D/C and P1.12 for chip select. Connect the remaining SPI/I2C signals using the nRF52840 DK board pin mapping; these are not reassigned by this overlay. The overlay also assigns PWM outputs to P0.13-P0.16.

## Set up the workspace

Install Git, Python with a virtual environment, west, CMake, Ninja, and a Zephyr-compatible Arm SDK. Use the [Zephyr 4.2 setup guide](https://docs.zephyrproject.org/4.2.0/develop/getting_started/index.html) for host dependencies and the [nRF52840 DK guide](https://docs.zephyrproject.org/4.2.0/boards/nordic/nrf52840dk/doc/index.html) for flashing tools.

First, activate your Python environment and make sure west is installed. Then run these commands:

```sh
git clone --branch release https://github.com/elitetq/GameOver_NRF.git
cd GameOver_NRF
west update
west zephyr-export
python -m pip install -r zephyr/scripts/requirements.txt
```

This repository already includes `.west/config`, pointing to [app/west.yml](app/west.yml), so do not run `west init` inside it. The manifest downloads Zephyr **v4.2.0** and its dependencies, along with J-Graphics-Library in `modules/j_gl`. You need to run `west update` because cloning this repo alone does not download the graphics library. The graphics dependency follows `main`, so future `west update` runs can change it.

## Build and flash

From the repository root, with the SDK and flashing tools configured:

```sh
west build -p always -b nrf52840dk/nrf52840 app -d build -- -DDTC_OVERLAY_FILE=boards/nrf52840dk_nrf52840.overlay
west flash -d build
```

The overlay argument selects the display pin configuration. These commands create the build in `build/`. Do not use `app/build.bak/` to build the project, as it contains files from an older build.

After flashing, the display should draw the game. Swipe up, down, left or right to change direction. Confirm that the player responds and the point display updates. If the screen is blank, check power, common ground, SPI wiring, D/C and chip select. If drawing works but swipes do not, check the I2C touch wiring. Swipe thresholds and timing are in [j_controls.h](app/src/j_controls.h).

## Project layout

| Path | Purpose |
| --- | --- |
| `app/src/main.c` | Display setup and game entry point |
| `app/src/j_controls.c` | Touch/swipe input |
| `app/games/TOTS/` | Game logic, levels, drawing and resources |
| `app/prj.conf` | Zephyr configuration |
| `app/west.yml` | Dependency manifest |
| `modules/j_gl/` | Graphics module fetched by west |
| `tots_images/` | Source images and converted assets |
| `cnv_to_bit.py`, `cnv_to_bitmap.py` | Asset conversion scripts |

## Notes

To use another board or display, you will need to change the overlay and graphics initialization. There are no automated hardware tests, so check drawing and touch input after flashing.
