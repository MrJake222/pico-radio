#pragma once

#include <screen.hpp>
#include <listloader.hpp>

#define KB_BUTTONS_MAX    4

class ScreenList : public Screen {

    // method should return how many rows are above the list
    virtual int rows_above() = 0;
    // rows below the list
    virtual int rows_below() = 0;

    int first_list_row() { return rows_above(); }
    int last_list_row()  { return last_y() - rows_below(); }

    // this variable holds index of the first
    // currently displayed station
    int base_y;
    int max_base_y();

    void iny() override;
    void dey() override;

    void button_pre_selection_change() override;
    void draw_top_buttons();
    void draw_bottom_buttons();
    void draw_scroll_bar();

    // first <show> after <begin>
    bool first;

    // gated count, only updated after all stations are loaded
    int station_count;

protected:

    ListLoader& ll;

    // how many entry buttons are there
    // used by subclass to return its size_y
    int kb_buttons();

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
               int s_scr_w_, int s_scr_pad_,
                ListLoader& ll_)
        : Screen(display_, mutex_ticker_)
        , s_base_x(s_base_x_), s_base_y(s_base_y_)
        , s_res_w(s_res_w_), s_res_h(s_res_h_), s_res_mar(s_res_mar_), s_res_pad(s_res_pad_)
        , s_scr_w(s_scr_w_), s_scr_pad(s_scr_pad_)
        , ll(ll_)
        { }

    // subclasses should call different ll.begin() methods
    // before calling to this super-method
    void begin() override;
    void show() override;

    friend void all_loaded_cb(void* arg, int errored);
};