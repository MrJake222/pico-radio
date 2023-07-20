#pragma once

#include <list.hpp>


class ListM3U : public List {
public:
    ListError consume_format(char* line) override;

    ListError produce_preamble(DataInterface *di, char *scratch) override;
    ListError produce_format(DataInterface *di, char* scratch) override;
};

extern ListM3U listm3u;