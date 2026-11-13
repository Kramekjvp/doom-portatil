# Firmware do console (ESP32-S3 N16R8): launcher + Tetris + Doom.
# ESTA pasta e o projeto ESP-IDF: main/ e components/ sao achados automaticamente.
#
#   idf.py set-target esp32s3
#   idf.py build
#
# Para validar tela, botoes e Tetris sem compilar o Doom (primeira compilacao na placa):
#   idf.py -DPORTATIL_SEM_DOOM=1 build        (para voltar: -DPORTATIL_SEM_DOOM=0)
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(portatil)
