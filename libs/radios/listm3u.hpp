#pragma once

#include <list.hpp>


class ListM3U : public List {
public:
    ListError try_consume_format(char* line) override;
};

extern ListM3U listm3u;