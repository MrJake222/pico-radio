#pragma once

#include <config.hpp>
#include <screenkb.hpp>

class ScSearch : public ScreenKb {

    const char* get_title() override { return "Wyszukaj stację"; }

    int text_max_len() override { return MAX_PROMPT_LEN; }
    Screen * sc_back() override;
    Screen * sc_forward(const char* text) override;

public:
    using ScreenKb::ScreenKb;
};
