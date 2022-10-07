#pragma once
#include "furi.h"
#include "flipper_format_i.h"
#include "flipper_format_stream_i.h"
#include "stream_i.h"
#include "subghz/types.h"

uint32_t rand_range(uint32_t min, uint32_t max);
bool load_file_playground(Storage* storage, FlipperFormat* fff_data, const char* file_path);
bool write_file_noise_playground(
    Stream* file,
    bool is_negative_start,
    size_t current_position,
    bool empty_line);
bool write_file_split_playground(
    Storage* storage,
    FlipperFormat* flipper_string,
    const char* dir_path);