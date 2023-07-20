#pragma once

#include <list.hpp>
#include <cstdio>
#include <cassert>

class ListPLS : public List {
public:
    ListError consume_format(char* line) override;
    ListError produce_format(DataInterface *di, char *scratch) override {
        puts("unsupported");
        assert(false);
        return ListError::ERROR;
    }
};

extern ListPLS listpls;