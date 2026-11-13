/*
 * Abertura do WAD na placa: em vez de um arquivo, o WAD fica numa particao de
 * flash ("wad", tipo data/0x40) mapeada na memoria (esp_partition_mmap). O Doom
 * ja sabe usar WAD mapeado (wad_file_t.mapped): W_CacheLumpNum devolve ponteiros
 * direto da flash, sem copiar para a RAM. Substitui w_file.c e w_file_stdc.c do
 * doomgeneric neste alvo (por isso eles nao estao em doomgeneric_sources.txt).
 */
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "w_file.h"

static const char *TAG = "w_file_esp";

typedef struct {
    wad_file_t wad;                           /* tem de ser o primeiro membro */
    esp_partition_mmap_handle_t handle;
} esp_wad_t;

static wad_file_class_t esp_wad_class;

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static wad_file_t *esp_wad_open(char *path)
{
    const esp_partition_t *part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, (esp_partition_subtype_t)0x40, CONFIG_DOOM_WAD_PARTITION);
    if (!part) {
        ESP_LOGE(TAG, "particao \"%s\" (data/0x40) nao existe: confira partitions.csv", CONFIG_DOOM_WAD_PARTITION);
        return NULL;
    }

    const void *ptr = NULL;
    esp_partition_mmap_handle_t handle;
    esp_err_t err = esp_partition_mmap(part, 0, part->size, ESP_PARTITION_MMAP_DATA, &ptr, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_partition_mmap falhou: %s", esp_err_to_name(err));
        return NULL;
    }

    /* A particao e maior que o WAD: o tamanho real vem do cabecalho
     * ("IWAD"/"PWAD", n lumps, offset do diretorio; o diretorio fica no fim). */
    const uint8_t *b = (const uint8_t *)ptr;
    if (memcmp(b, "IWAD", 4) != 0 && memcmp(b, "PWAD", 4) != 0) {
        ESP_LOGE(TAG, "a particao \"%s\" nao contem um WAD (cabecalho 0x%02X%02X%02X%02X). "
                      "Grave o doom1.wad: veja firmware/README.md",
                 CONFIG_DOOM_WAD_PARTITION, b[0], b[1], b[2], b[3]);
        esp_partition_munmap(handle);
        return NULL;
    }
    const uint32_t numlumps = le32(b + 4), dir = le32(b + 8);
    const uint64_t len = (uint64_t)dir + (uint64_t)numlumps * 16u;
    if (len > part->size) {
        ESP_LOGE(TAG, "WAD maior que a particao (%llu > %u bytes)", (unsigned long long)len, (unsigned)part->size);
        esp_partition_munmap(handle);
        return NULL;
    }

    esp_wad_t *w = calloc(1, sizeof *w);
    if (!w) { esp_partition_munmap(handle); return NULL; }
    w->wad.file_class = &esp_wad_class;
    w->wad.mapped = (byte *)ptr;
    w->wad.length = (unsigned int)len;
    w->handle = handle;
    ESP_LOGI(TAG, "WAD mapeado da flash: %u lumps, %u bytes (pedido: %s)", (unsigned)numlumps, (unsigned)len, path);
    return &w->wad;
}

static void esp_wad_close(wad_file_t *file)
{
    esp_wad_t *w = (esp_wad_t *)file;
    esp_partition_munmap(w->handle);
    free(w);
}

static size_t esp_wad_read(wad_file_t *file, unsigned int offset, void *buffer, size_t buffer_len)
{
    if (offset >= file->length) return 0;
    if (buffer_len > file->length - offset) buffer_len = file->length - offset;
    memcpy(buffer, file->mapped + offset, buffer_len);
    return buffer_len;
}

static wad_file_class_t esp_wad_class = {
    .OpenFile = esp_wad_open,
    .CloseFile = esp_wad_close,
    .Read = esp_wad_read,
};

/* ---- API de w_file.h (o que w_file.c fazia no doomgeneric) ---- */

wad_file_t *W_OpenFile(char *path)         { return esp_wad_class.OpenFile(path); }
void        W_CloseFile(wad_file_t *wad)   { wad->file_class->CloseFile(wad); }
size_t      W_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t len)
{
    return wad->file_class->Read(wad, offset, buffer, len);
}
