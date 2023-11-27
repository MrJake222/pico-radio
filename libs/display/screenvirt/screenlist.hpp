#pragma once

#include <screen.hpp>
#include <loader.hpp>

#define KB_BUTTONS_MAX    4

class ScreenList : public Screen {

    // method should return how many rows are above the list
    // default 0 (status icons not clickable)
    virtual int rows_above() { return 0; }
    // rows below the list
    // default 0 (action icons like back/forward)
    virtual int rows_below() { return 1; }

    // how many action icons available
    // number of last row entries
    virtual int action_icons() = 0;

    // how many list entry buttons are there
    int kb_buttons();

    int first_list_row() { return rows_above(); }
    int last_list_row()  { return last_y() - rows_below(); }

    bool scrolled_area() { return (first_list_row() <= current_y) && (current_y <= last_list_row()); }

    // this variable holds index of the first
    // currently displayed station
    int base_y;
    int max_base_y();

    void inx() override;
    void dex() override;
    void iny() override;
    void dey() override;

    void button_pre_selection_change() override;
    void draw_top_buttons();
    void draw_bottom_buttons();
    void draw_scroll_bar();

    // scrolled text id of currently selected item
    int text_idx;

    // was the loading at least called once (the interface was displayed)?
    // set to false in <begin>
    // set to true in <show>
    // remains <true> when loading a new page
    bool loaded;

    // gated count, only updated after all stations are loaded
    int station_count;

    virtual Loader& get_ll() = 0;

    // pagination support
    int page_count;
    int page;
    void print_page();

protected:
    int size_x(int y) override final { return y == last_y() ? action_icons() : 1; }
    int size_y() override final { return kb_buttons() + rows_above() + rows_below(); }

    bool is_loaded() { return loaded; }
    int get_page() { return page; }

    // draws entry buttons, pass variables from draw_button
    void draw_button_entry(int y, bool selected);

    int get_selected_station_index() { return base_y + current_y - rows_above(); }

    const int s_base_x;     // list starting x coordinate
    const int s_base_y;     // list starting y coordinate
    const int s_res_w;      // result item width
    const int s_res_h;      // result item height
    const int s_res_mar;    // result item space between items
    const int s_res_pad;    // result item text padding (left-right)
    const int s_scr_w;      // scroll bar width
    const int s_scr_pad;    // scroll bar spacing from results

public:
    ScreenList(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
               int s_base_x_, int s_base_y_,
               int s_res_w_, int s_res_h_, int s_res_mar_, int s_res_pad_,
               int s_scr_w_, int s_scr_pad_)
        : Screen(display_, mutex_ticker_)
        , s_base_x(s_base_x_), s_base_y(s_base_y_)
        , s_res_w(s_res_w_), s_res_h(s_res_h_), s_res_mar(s_res_mar_), s_res_pad(s_res_pad_)
        , s_scr_w(s_scr_w_), s_scr_pad(s_scr_pad_)
        { }

    // subclasses should call different ll.begin() methods
    // before calling to this super-method
    void begin() override;
    void show() override;

    // to be used by player add/remove from favourites feature (both functions, or <begin>)
    // on next <show>, reload the contents
    void set_fresh_load();
    // set page/base_y/current_y from absolute index
    void set_fav_pos(int fav_index);

    friend void all_loaded_cb(void* arg, int errored);
};