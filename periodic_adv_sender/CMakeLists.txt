cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(periodic_adv_sender)

target_sources(app PRIVATE src/main.c src/saadc_helpers.c src/timer_helpers.c src/bt_helpers.c)
