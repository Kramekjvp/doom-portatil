# Teste da tela por GPIO simples (sem LCD_CAM, sem esp_lcd). Projeto ESP-IDF separado do firmware.
#   idf.py set-target esp32s3
#   idf.py build flash monitor
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lcd_teste_gpio)