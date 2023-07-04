#pragma once

#include <list.hpp>


class ListPLS : public List {
public:
    ListError consume_format(char* line) override;
};

extern ListPLS listpls;