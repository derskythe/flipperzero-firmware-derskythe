#include "playground_files.h"
#include "furi.h"
#include "flipper_format_i.h"
#include "flipper_format_stream_i.h"
#include "file_stream.h"
#include "subghz/types.h"

#define MAX_LINE 500
#define RAND_MAX_VALUE = 700
#define RAND_MIN_VALUE = 100

uint32_t rand_range(uint32_t n, uint32_t m) {
    // size of range, inclusive
    const uint32_t length_of_range = m - n + 1;

    // add n so that we don't return a number below our range
    return (uint32_t)(rand() % length_of_range + n);
}

bool load_file_playground(Storage* storage, FlipperFormat* fff_data, const char* file_path) {
    furi_assert(storage);
    furi_assert(fff_data);
    furi_assert(file_path);

    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    Stream* fff_data_stream = flipper_format_get_raw_stream(fff_data);

    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;
    bool result = false;

    do {
        stream_clean(fff_data_stream);
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            break;
        }

        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }

        if(((!strcmp(string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing Frequency");
            break;
        }

        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            break;
        }

        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        stream_copy_full(
            flipper_format_get_raw_stream(fff_data_file), flipper_format_get_raw_stream(fff_data));

        result = true;
    } while(0);

    string_clear(temp_str);
    flipper_format_free(fff_data_file);
    //furi_record_close(RECORD_STORAGE);

    return result;
}

bool write_file_noise_playground(
    Stream* file,
    bool is_negative_start,
    size_t current_position,
    bool empty_line) {
    size_t was_write = 0;
    if(empty_line) {
        was_write = stream_write_cstring(file, "RAW Data: ");

        if(was_write <= 0) {
            // TODO: Add error
            return false;
        }
    }

    int8_t first;
    int8_t second;
    if(is_negative_start) {
        first = -1;
        second = 1;
    } else {
        first = 1;
        second = -1;
    }
    while(current_position < MAX_LINE) {
        was_write = stream_write_format(
            file,
            "%d %d ",
            rand_range(RAND_MIN_VALUE, RAND_MAX_VALUE) * first,
            rand_range(RAND_MIN_VALUE, RAND_MAX_VALUE) * second);

        if(was_write <= 0) {
            // TODO: Add error
            return false;
        }

        current_position += was_write;
    }

    // Step back to write \n instead of space
    size_t offset = stream_tell(file);
    if(stream_seek(file, offset - 1, StreamOffsetFromStart)) {
        // TODO: Add error
        return false;
    }

    return stream_write_char(file, '\n') > 0;
}

bool write_file_split_playground(
    Storage* storage,
    FlipperFormat* flipper_string,
    void* current_item,
    const char* dir_path) {
#ifdef FURI_DEBUG
    FURI_LOG_W(TAG, "Save temp file splitted: %s", dir_path);
#endif
    const size_t buffer_size = 32;
    uint8_t buffer[buffer_size];
    Stream* src = flipper_format_get_raw_stream(flipper_string);
    stream_rewind(src);

    Stream* file = buffered_file_stream_alloc(storage);
    bool result = buffered_file_stream_open(file, dir_path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    do {
        if(!result) {
            // TODO: Add error
            break;
        }
        if(!flipper_format_stream_seek_to_key(src, "RAW Data", true)) {
            // TODO: Add error
            break;
        }
        bool is_negative_start = false;
        bool is_negative_end = false;
        bool found = false;

        size_t offset_start = stream_tell(src);

        // Check for negative value at the start and end to align file by correct values
        size_t was_read = stream_read(src, buffer, 1);
        if(was_read <= 0) {
            // TODO: Add error
            break;
        }

        is_negative_start = buffer[0] == '-';
        // Goto end
        stream_seek(src, 32, StreamOffsetFromEnd);
        was_read = stream_read(src, buffer, buffer[buffer_size]);

        if(was_read <= 0) {
            // TODO: Add error
            break;
        }
        // Seek for last value
        for(size_t i = 0; i < buffer_size; i++) {
            if(buffer[i] == flipper_format_eoln || buffer[i] == flipper_format_eolr) {
                // End of line
                break;
            }
            if(buffer[i] == ' ') {
                is_negative_end = false;
            } else if(buffer[i] == '-') {
                is_negative_end = true
            } else {
                // Other values is digits
            }
        }

        // Ready to
        if(!stream_seek(src, offset_start, StreamOffsetFromStart)) {
            // TODO: Add error
            break;
        }
        found = true;
        size_t line_len = 0;

        if(!write_file_noise_playground(src, is_negative_start, is_negative_end, line_len, false)) {
            // TODO: Add error
            break;
        }

        do {
            uint16_t bytes_were_read = stream_read(src, buffer, buffer_size);
            if(bytes_were_read == 0) break;

            bool error = false;
            for(uint16_t i = 0; i < bytes_were_read; i++) {
                if(buffer[i] == flipper_format_eoln) {
                    if(!stream_seek(src, i - bytes_were_read + 1, StreamOffsetFromCurrent)) {
                        error = true;
                        break;
                    }
                    string_push_back(str_result, buffer[i]);
                    result = true;
                    break;
                } else if(buffer[i] == flipper_format_eolr) {
                    // Ignore
                } else {
                    string_push_back(str_result, buffer[i]);
                }
            }

            if(result || error) {
                break;
            }
        } while(true);

        result = found;
    } while(false);

    stream_free(file);
    //    if(stream_save_to_file(dst, storage, dir_path, FSOM_CREATE_ALWAYS) >
    //       0) {
    //        flipper_format_free(item->flipper_string);
    //        item->flipper_string = NULL;
    //#ifdef FURI_DEBUG
    //        FURI_LOG_I(TAG, "Save done!");
    //#endif
    //        // This item contains fake data to load from SD
    //        item->is_file = true;
    //    } else {
    //        FURI_LOG_E(TAG, "Stream copy failed!");
    //    }

    return result;
}