#pragma once

#include <list.hpp>


class ListPLS : public List {
public:
    ListError try_consume_format(char* line) override;
};

extern ListPLS listpls;