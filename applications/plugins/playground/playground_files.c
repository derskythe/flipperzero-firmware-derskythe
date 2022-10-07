#include "playground_files.h"
#include "furi.h"
#include "flipper_format_i.h"
#include "flipper_format_stream_i.h"
#include "file_stream.h"
#include "subghz/types.h"

#define MAX_LINE 500
#define RAND_MAX_VALUE 700
#define RAND_MIN_VALUE 100
#define RAW_KEY_NAME "RAW Data"

const size_t buffer_size = 32;

uint32_t rand_range(uint32_t min, uint32_t max) {
    // size of range, inclusive
    const uint32_t length_of_range = max - min + 1;

    // add n so that we don't return a number below our range
    return (uint32_t)(rand() % length_of_range + min);
}

bool load_file_playground(Storage* storage, FlipperFormat* fff_data, const char* file_path) {
    furi_assert(storage);
    furi_assert(fff_data);
    furi_assert(file_path);

    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    Stream* fff_data_stream = flipper_format_get_raw_stream(fff_data);

    FuriString* temp_str;
    temp_str = furi_string_alloc();
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

        if(((!furi_string_cmp_str(temp_str, SUBGHZ_KEY_FILE_TYPE)) ||
            (!furi_string_cmp_str(temp_str, SUBGHZ_RAW_FILE_TYPE))) &&
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

    furi_string_free(temp_str);
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
        was_write = stream_write_format(file, "%s: ", RAW_KEY_NAME);

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
    if(stream_seek(file, offset - 1, StreamOffsetFromCurrent)) {
        // TODO: Add error
        return false;
    }

    return stream_write_char(file, '\n') > 0;
}

size_t write_file_data_playground(Stream* src, Stream* file, bool is_negative_start) {
    size_t current_position = 0;
    uint8_t buffer[buffer_size];
    bool result = false;
    bool current_negative = is_negative_start;
    do {
        uint16_t bytes_were_read = stream_read(src, buffer, buffer_size);
        if(bytes_were_read == 0) {
            break;
        }

        bool error = false;
        uint16_t last_positive_position = 0;
        uint16_t last_negative_position = 0;
        for(uint16_t i = 0; i < bytes_were_read; i++) {
            if(buffer[i] == flipper_format_eoln) {
                if(!stream_seek(src, i - bytes_were_read + 1, StreamOffsetFromCurrent)) {
                    // TODO: Add error
                    error = true;
                    break;
                }
                result = true;
                break;
            } else if(buffer[i] == flipper_format_eolr) {
                // Ignore
            } else if (buffer[i] == '-') {
                current_negative = true;
                last_negative_position = i;
            } else if (buffer[i] == ' ') {
                current_negative = false;
                last_positive_position = i;
            }
        }

        if (error) {
            break;
        }

        if (result) {
            // Need to save current position for later purpose
        } else if (current_position + bytes_were_read < MAX_LINE) {
            // Simply add bytes to stream
            stream_write(file, buffer, bytes_were_read);
            current_position += bytes_were_read;
        } else {
            // We need to check line
            size_t size = 0;
            bool value_is_correct = false;
            if (is_negative_start) {
                // Last is positive number
                size = last_positive_position - 1;
                value_is_correct = last_positive_position < last_negative_position;
            } else {
                // Last is negative number
                size = last_negative_position - 1;
                value_is_correct = last_negative_position < last_positive_position;
            }

            stream_write(file, buffer, size);
            // But now we need to understand is it next number is correct
            for(uint16_t i = 0; i < bytes_were_read; i++) {

            }
        }
        for(current_position; current_position < MAX_LINE; current_position++) {

        }

        if(result || error) {
            break;
        }
    } while(true);
}

bool write_file_split_playground(
    Storage* storage,
    FlipperFormat* flipper_string,
    const char* dir_path) {
#ifdef FURI_DEBUG
    FURI_LOG_W(TAG, "Save temp file splitted: %s", dir_path);
#endif
    uint8_t buffer[buffer_size];
    Stream* src = flipper_format_get_raw_stream(flipper_string);
    stream_rewind(src);

    Stream* file;
    file = buffered_file_stream_alloc(storage);
    bool result;
    result = buffered_file_stream_open(file, dir_path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

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

        size_t offset_start;
        offset_start = stream_tell(src);

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
        for(size_t i = 0; i < was_read; i++) {
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

        // Ready to write stream to file
        size_t current_position;
        stream_rewind(src);
        current_position = stream_copy(src, file, offset_start - 1);
        if(current_position != offset_start + 1) {
            // TODO: Add error
            break;
        }

        if(!stream_seek(src, offset_start, StreamOffsetFromStart)) {
            // TODO: Add error
            break;
        }
        found = true;

        current_position = 0;
        if(!write_file_noise_playground(file, is_negative_start, current_position, false)) {
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