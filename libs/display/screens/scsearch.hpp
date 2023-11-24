#pragma once

#include <config.hpp>
#include <screenkb.hpp>
#include <cstring>

class ScSearch : public ScreenKb {

    const char* get_title() override { return "Wyszukaj stację"; }

public:
    using ScreenKb::ScreenKb;
};
