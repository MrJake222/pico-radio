#include "screenlist.hpp"

#include <cstdio>

#include <ubuntu_mono.hpp>
#include <icons.hpp>
#include <screenmng.hpp>
#include <wificonnect.hpp>

// #define s_base_x      3
// #define s_base_y     31
//
// #define s_res_w     147
// #define s_res_h      20
// #define s_res_mar     1
// #define s_res_pad     2
//
// #define s_scr_w    5
// #define s_scr_pad  2

int ScreenList::max_base_y() {
    return station_count - kb_buttons();
}

int ScreenList::kb_buttons() {
    return MIN(KB_BUTTONS_MAX, station_count);
}

void ScreenList::clear_subarea() {
    display.fill_rect(s_base_x,
                      s_base_y,
                      s_res_w + s_scr_pad + s_scr_w,
                      (s_res_h + s_res_mar) * KB_BUTTONS_MAX,
                      COLOR_BG);
}

void ScreenList::draw_list_buttons(int from, int to) {
    for (int y=from; y<to; y++) {
        draw_button(0, y + rows_above(), false, false);
    }
}

void ScreenList::draw_scroll_bar() {
    if (station_count == 0)
        return;

    const int x = s_base_x + s_res_w + s_scr_pad;
    const int y = s_base_y;
    const int h = (s_res_h + s_res_mar) * 3 + s_res_h;

    display.fill_rect(x, y, s_scr_w, h, COLOR_BG_DARK);

    const float segment = h / (float)station_count;
    const float start   = y + segment * (float)base_y;         // skip base_y segments
    const float height  =     segment * (float)kb_buttons();   // display 4 segments high bar

    display.fill_rect(x, (int)start, s_scr_w, (int)height, COLOR_ACC1);
}

bool ScreenList::inx() {
    if (scrolled_area()) {
        // inside of scrolling area
        int page_orig = page;

        if (get_ll().is_in_progress())
            return false;

        // inx -> next page
        page++;
        if (page_count != -1)
            page %= page_count;

        // reload page
        if (page != page_orig) {
            load_page(lp_src_new_page);
        }

        // don't do any button redraw
        // show_loaded() will draw all necessary list buttons
        return false;
    }

    // if not scrolled area, use default impl
    return Screen::inx();
}

bool ScreenList::dex() {
    if (scrolled_area()) {
        // inside of scrolling area
        int page_orig = page;

        if (get_ll().is_in_progress())
            return false;

        // dex -> prev page
        page--;
        if (page < 0) {
            if (page_count == -1)
                // infinite, stay at 0
                page = 0;
            else
                page = page_count - 1;
        }

        // reload page
        if (page != page_orig) {
            load_page(lp_src_new_page);
        }

        // don't do any button redraw
        // show_loaded() will draw all necessary list buttons
        return false;
    }

    return Screen::dex();
}

bool ScreenList::iny() {
    if (current_y == last_list_row() && get_selected_entry_index_on_page() + 1 < station_count) {
        // last kb row
        // hidden station below exists
        base_y++;
        draw_scroll_bar();
        reset_scrolled_texts();
        draw_list_top_buttons();

        // let Screen::input() handle last button
        return true;
    }

    if ((current_y == last_y() && rows_above() == 0) || (current_y == first_list_row() - 1)) {
        // bottom icon and no icons above the list
        // the cursor will jump straight into topmost list position
        // OR
        // last top icon
        // the cursor will enter the list from above
        //
        // move to top of the list (after this the first row is selected)
        base_y = 0;
        draw_scroll_bar();
        Screen::iny();
        draw_list_bottom_buttons(); // no need to redraw topmost entry (y changed, Screen::input() will redraw it)

        // redraw top entry and deselect last icon
        return true;
    }

    return Screen::iny();
}

bool ScreenList::dey() {
    if (current_y == first_list_row() && base_y > 0) {
        // hidden station above exists
        base_y--;
        draw_scroll_bar();
        reset_scrolled_texts();
        draw_list_bottom_buttons();

        // let Screen::input() handle last button
        return true;
    }

    if (current_y == last_list_row() + 1) {
        // last bottom icon
        // move to bottom of the list
        // after this the last row is selected
        base_y = max_base_y();
        draw_scroll_bar();
        Screen::dey();
        draw_list_top_buttons(); // no need to redraw topmost entry (y changed, Screen::input() will redraw it)

        // redraw top entry and deselect last icon
        return true;
    }

    return Screen::dey();
}

void ScreenList::draw_button_entry(int y, bool selected, bool was_selected) {

    unsigned char xs;
    unsigned char ys;
    const ListEntry* ent;
    const char* name;
    int bg;

    // 0-align y coordinate
    y -= first_list_row();

    xs = s_base_x;
    ys = s_base_y + (s_res_h + s_res_mar) * y;
    ent = get_ll().get_entry(base_y + y);
    name = ent->get_name();

    bg = get_btn_bg(selected, true);
    display.fill_rect(xs, ys, s_res_w, s_res_h, bg);

    const struct icon* icons[2] = { nullptr, nullptr };

    switch (ent->type) {
        case le_type_local:
            if (ent->llocal.is_dir) {
                icons[0] = &icon_folder;
            }

            break;

        case le_type_wifi:
            icons[0] = wifi::quality_to_icon(ent->lwifi.quality);
            if (wifi::is_connected_to(name))
                icons[1] = &icon_conn;

            break;

        default:
            // no additional icons
            break;
    }

    int pad_right = 0;
    for (auto icon : icons) {
        if (icon) {
            const int pad = (s_res_h - icon->h) / 2;
            display.draw_icon(xs + s_res_pad + pad_right, ys + pad, icon, bg, COLOR_FG);

            // move text
            pad_right += icon->w + 3;
        }
    }

    if (was_selected && !selected)
        // this is a deselection call from Screen:input()
        reset_scrolled_texts();

    const struct font* font = ubuntu_font_get_size(UbuntuFontSize::FONT_16);
    const int pad = (s_res_h - font->H) / 2;
    text_idx = add_scrolled_text_or_normal(
            xs + s_res_pad + pad_right, ys + pad - 1, name, font,
            bg, COLOR_FG,
            s_res_w - s_res_pad * 2 - pad_right, selected);
}

void all_loaded_cb(void* arg, int errored);

void ScreenList::load_page(lp_src src_) {
    src = src_;

    reset_scrolled_texts(); // TODO reset only owned

    if (((src == lp_src_show) && info_load_show) || ((src != lp_src_show) && info_load)) {
        clear_subarea();

        add_normal_text(10, 40, "Ładowanie",
                        ubuntu_font_get_size(UbuntuFontSize::FONT_24),
                        COLOR_BG, COLOR_FG,
                        display.W);
    }

    // only if new page
    // else set false
    get_ll().use_cache(src == lp_src_new_page);

    get_ll().load(page);
}

void ScreenList::show() {
    // called from input

    // save <station_count>
    // (this is done so that Screen::show() won't draw any buttons, causes flickering)
    const int scnt = station_count;

    // reset loaded stations & call show
    // (show tries to draw a button list from old screen,
    //  this causes flickering)
    station_count = 0;
    Screen::show();

    // setup loading
    // <ll.begin> called by subclass before this
    get_ll().set_cb_arg(this);
    get_ll().set_all_loaded_cb(all_loaded_cb);

    // must be after loader setup
    if (preserve) {
        preserve = false;

        // restore <station_count>
        // (so show_loaded can draw buttons)
        station_count = scnt;
        src = lp_src_show;

        show_loaded();
        return;
    }

    // must be after preserve
    // (show_overlay sets preserve)
    if (is_overlay_displayed)
        return;

    load_page(lp_src_show);
}

void ScreenList::show_loaded() {
    if (src == lp_src_new_page || src == lp_src_dir) {
        // this is executed only on new page / new directory
        // reset position to the top of the page
        current_y = default_y();
        base_y = 0;
    }
    // on first entry position is reset by <begin>

    clear_subarea();
    draw_buttons();
    draw_scroll_bar();
    print_page();
}

void ScreenList::begin() {
    Screen::begin();

    // start on top of first page
    set_abs_pos(0);

    // reset preserve
    preserve = false;
}

void ScreenList::set_abs_pos(int abs_index) {
    const int old_page = page;
    page = abs_index / get_max_entries();

    if (old_page != page)
        // if some screen changes page on this one
        // for example ScPlay plays through entire album
        // reset preserve
        preserve = false;

    // set to last row of keyboard
    // it's always possible, as opposed to top row (not enough stations below)
    int offset = KB_BUTTONS_MAX - 1; // from bottom to top row
    base_y = (abs_index % get_max_entries()) - offset;
    current_y = rows_above() + offset;

    if (base_y < 0) {
        offset = -base_y;
        base_y += offset;
        current_y -= offset;
    }

    current_x = 0;
}

void ScreenList::print_page() {
    const struct font* font = ubuntu_font_get_size(UbuntuFontSize::FONT_12);

    char buf[10];
    if (page_count == -1)
        // infinite pages
        sprintf(buf, "%2d / \xE2\x88\x9E", get_page()+1); // infinity sign
    else
        sprintf(buf, "%2d / %2d", get_page()+1, page_count);

    const int width = strlen_utf8(buf) * font->W;
    const int x = (display.W - width) / 2;

    add_normal_text(x, 114, buf,
                        font,
                        COLOR_BG, COLOR_FG,
                        display.W);
}

void all_loaded_cb(void* arg, int errored) {
    // called from RadioSearch task
    auto sc = ((ScreenList*) arg);

    // set station & page count
    sc->station_count = sc->get_ll().get_entry_count();
    sc->page_count =    sc->get_ll().get_page_count();

    if (errored > 0) {
        // show error
        char c[80];
        snprintf(c, 80, "Błąd: nie udało się załadować %d dostawców stacji.", errored);
        sc->show_error(c);
        return;
    }

    // re-draw the screen
    // (if no error)
    sc->show_loaded();
}

void ScreenList::show_overlay(int bg, const char* msg) {
    set_preserve();
    Screen::show_overlay(bg, msg);
}
