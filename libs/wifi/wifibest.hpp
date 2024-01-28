#pragma once

namespace wifi {

typedef void(*conn_cb)();

// tries to connect to highest-quality saved network
void connect_best_saved(conn_cb cb);

} // namespace