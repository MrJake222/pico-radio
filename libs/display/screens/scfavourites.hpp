#pragma once

#include <screen.hpp>

class ScFavourites : public Screen {

    const char* get_title() override { return "Ulubione stacje"; }

public:
    using Screen::Screen;
    void show() override;
};
