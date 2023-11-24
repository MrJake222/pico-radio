#pragma once

#include <screenkb.hpp>

class ScWifiPwd : public ScreenKb {

    const char * get_title() override { return "Wprowadź hasło"; }
    Screen * sc_back() override;
    Screen * sc_forward(const char *text) override;

public:
    using ScreenKb::ScreenKb;

};
