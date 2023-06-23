#include <filetype.hpp>
#include <cstring>

const char* filetype_to_string(FileType filetype) {
    switch (filetype) {
        case FileType::WAV:         return "WAV";
        case FileType::MP3:         return "MP3";
        case FileType::RADIO:       return "RADIO";

        case FileType::UNSUPPORTED:
        default:
            return "UNSUPPORTED";
    }
}

FileType filetype_from_name(const char* name) {
    if (strncmp(name, "http", 4) == 0) {
        return FileType::RADIO;
    }

    const char *extension = name + strlen(name) - 4;

    if (strcmp(extension, ".mp3") == 0)
        return FileType::MP3;

    else if ((strcmp(extension, ".wav") == 0) || (strcmp(extension, "wave") == 0))
        return FileType::WAV;

    else
        return FileType::UNSUPPORTED;
}

const char* filetype_from_name_string(const char* name) {
    return filetype_to_string(filetype_from_name(name));
}
