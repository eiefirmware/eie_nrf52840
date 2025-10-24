#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

zephyr_include_directories(src)

<<<<<<< HEAD
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../drivers
)
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/BTN
)
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/LED
)
=======
target_sources(app PRIVATE src/main.c)
>>>>>>> 411e7ce96d2adce819f520352c905a74509f0652
