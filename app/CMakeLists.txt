cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(GameOver)

# J_GL and J_ASSETS are compiled automatically by Zephyr's module system
# via modules/j_gl/zephyr/module.yml — no manual add_subdirectory needed.

zephyr_include_directories(src)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/j_controls.c)
target_sources(app PRIVATE src/j_assets.c)

add_subdirectory(games)
