#pragma once

#include <list.hpp>


class ListPLS : public List {
public:
    ListError try_consume() override;
};

extern ListPLS listpls;