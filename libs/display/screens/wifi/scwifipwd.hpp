#pragma once

#include <screenkb.hpp>
#include <listentry.hpp>

class ScWifiPwd : public ScreenKb {

    const char* get_title() override { return "Wprowadź hasło"; }
    Screen* sc_back() override;
    Screen* sc_forward(const char* text) override;

    ListEntry* net;

public:
    using ScreenKb::ScreenKb;

    void begin(ListEntry* net_);
};
