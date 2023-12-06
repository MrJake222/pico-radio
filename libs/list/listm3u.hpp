#pragma once

#include <list.hpp>


class ListM3U : public List {
public:
    ListError consume_line_format(char* line) override;
};