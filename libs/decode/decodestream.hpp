#pragma once


#include "decodebase.hpp"
#include "httpclientpico.hpp"

class DecodeStream : public DecodeBase {

    HttpClientPico client;

public:
    void begin(const char* path_, Format* format_) override;
    void end() override;

    using DecodeBase::DecodeBase;
};
