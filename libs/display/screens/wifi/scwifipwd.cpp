#include "scwifipwd.hpp"

#include <screenmng.hpp>

Screen* ScWifiPwd::sc_back() {
    return &sc_wifi_scan;
}

Screen* ScWifiPwd::sc_forward(const char* text) {
    net->set_url(text);
    sc_wifi_conn.begin(this, net, false);
    return &sc_wifi_conn;
}

void ScWifiPwd::begin(ListEntry* net_) {
    ScreenKb::begin();
    net = net_;
}
