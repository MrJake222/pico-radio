#include "decodestream.hpp"

void DecodeStream::begin() {
    DecodeBase::begin();

    client.get(path);
}

void DecodeStream::end() {
    DecodeBase::end();

    client.close();
}
