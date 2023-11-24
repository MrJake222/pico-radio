#include "scsearch.hpp"

#include <screenmng.hpp>

Screen* ScSearch::sc_back() {
    return &sc_fav;
}

Screen* ScSearch::sc_forward(const char* text) {
    sc_search_res.begin(text);
    return &sc_search_res;
}
