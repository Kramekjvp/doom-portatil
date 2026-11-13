# Doom (doomgeneric) como jogo do launcher. Compila o motor + a camada portatil
# (dg_portatil.c, doom_game.c) + o que e especifico da placa (tempo, WAD na flash).
#
# O doomgeneric vem do submodulo third_party/doomgeneric, na raiz do repositorio
# (ozkl/doomgeneric, GPL-2).
# A lista de fontes e a MESMA do simulador (doomgeneric_sources.txt).
set(DG_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../third_party/doomgeneric")   # raiz do repositorio
set(DG_DIR  "${DG_ROOT}/doomgeneric")

if(NOT EXISTS "${DG_DIR}/doomgeneric.h")
    message(FATAL_ERROR "doomgeneric ausente em ${DG_ROOT}. Rode: git submodule update --init --recursive")
endif()

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/doomgeneric_sources.txt" DG_SRC_NAMES REGEX "^[^#]")
set(DG_SRCS "")
foreach(f ${DG_SRC_NAMES})
    list(APPEND DG_SRCS "${DG_DIR}/${f}")
endforeach()

idf_component_register(
    SRCS ${DG_SRCS} "dg_portatil.c" "doom_game.c" "doom_port_esp.c" "w_file_esp.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "${DG_DIR}"
    REQUIRES game_api
    PRIV_REQUIRES esp_timer esp_partition log
)

# Codigo de terceiros (GPL, escrito para PCs de 1993): sem -Werror por causa dele.
set_source_files_properties(${DG_SRCS} PROPERTIES COMPILE_OPTIONS "-w")

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    CMAP256                       # framebuffer de 8 bits (indice de paleta): 64 KB em vez de 256 KB
    DOOMGENERIC_RESX=320
    DOOMGENERIC_RESY=200
    NORMALUNIX
)

# Desvios sem editar o doomgeneric: "Quit Game" -> sair para o launcher; WAD virtual "existe".
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=I_Quit"
    "-Wl,--wrap=M_FileExists"
)
