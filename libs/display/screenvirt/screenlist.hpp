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

    bool inx() override;
    bool dex() override;
    bool iny() override;
    bool dey() override;

    // clear button + scroll bar area
    void clear_subarea();

    void draw_list_buttons(int from, int to);
    inline void draw_list_all_buttons()    { draw_list_buttons(0, kb_buttons());   }
    inline void draw_list_top_buttons()    { draw_list_buttons(0, kb_buttons()-1); }
    inline void draw_list_bottom_buttons() { draw_list_buttons(1, kb_buttons());   }
    void draw_scroll_bar();

    // scrolled text id of currently selected item
    int text_idx;

    // gated count, only updated after all stations are loaded
    int station_count;

    virtual Loader& get_ll() = 0;

    // pagination support
    int page_count;
    int page;
    void print_page();

    // see below at <lp_src>
    // defined as int, because no enum forward declaration
    int src;

    // default behaviour is to load data each time using a Loader
    // <run_action> of subclasses can override this by setting this to true
    // it is reset to false in <show>
    bool preserve;

protected:
    int size_x(int y) override final { return y == last_y() ? action_icons() : 1; }
    int size_y() override final { return kb_buttons() + rows_above() + rows_below(); }

    // load page source
    // from where the new page was requested
    enum lp_src {
        lp_src_show,        // <show> function, first load
        lp_src_new_page,    // new page, use cache
        lp_src_dir,         // directory descent
    };

    void load_page(lp_src src_);
    int get_page() { return page; }
    void set_preserve() { preserve = true; }

    // draws entry buttons, pass variables from draw_button
    void draw_button_entry(int y, bool selected, bool was_selected);

    int get_selected_station_index() { return base_y + current_y - rows_above(); }

    const int s_base_x;     // list starting x coordinate
    const int s_base_y;     // list starting y coordinate
    const int s_res_w;      // result item width
    const int s_res_h;      // result item height
    const int s_res_mar;    // result item space between items
    const int s_res_pad;    // result item text padding (left-right)
    const int s_scr_w;      // scroll bar width
    const int s_scr_pad;    // scroll bar spacing from results
    const bool info_load_show;   // show loading text on first load
    const bool info_load; // show loading text on reloads

public:
    ScreenList(ST7735S& display_, SemaphoreHandle_t& mutex_ticker_,
               int s_base_x_, int s_base_y_,
               int s_res_w_, int s_res_h_, int s_res_mar_, int s_res_pad_,
               int s_scr_w_, int s_scr_pad_,
               bool info_load_, bool info_reload_)
        : Screen(display_, mutex_ticker_)
        , s_base_x(s_base_x_), s_base_y(s_base_y_)
        , s_res_w(s_res_w_), s_res_h(s_res_h_), s_res_mar(s_res_mar_), s_res_pad(s_res_pad_)
        , s_scr_w(s_scr_w_), s_scr_pad(s_scr_pad_)
        , info_load_show(info_load_), info_load(info_reload_)
        { }

    // called on forward-entry only
    void begin() override;

    // loading setup should happen here (this is called from screen manager)
    // on every call (sub-screens can change loader settings this, and won't call begin() by design)
    // subclasses should call different ll.begin() methods before calling to this super-method)
    void show() override;

    // this is called after <show> initialized loading and it was finished
    void show_loaded();

    void show_overlay(int bg, const char *msg) override;

    // set page/base_y/current_y from absolute index into the list
    void set_abs_pos(int abs_index);

    friend void all_loaded_cb(void* arg, int errored);
};
