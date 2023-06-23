#pragma once

enum class FileType {
    WAV,
    MP3,
    RADIO,
    UNSUPPORTED
};

const char* filetype_to_string(FileType filetype);

FileType filetype_from_name(const char* name);
const char* filetype_from_name_string(const char* name);