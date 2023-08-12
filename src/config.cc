#include "config.h"

int cfg_node_expanding_thres = 8;
int cfg_playouts = 1000;
int cfg_search_threads = 1;
int cfg_fpu_value = 5;
int cfg_c_uct = 1;
bool cfg_dump_analysis = false;
int cfg_lag_buffer = 1;
int cfg_main_time = 7 * 24 * 60 * 60;
FILE *cfg_search_file = stderr;
std::vector<std::array<int, 2>> cfg_hollow_pos = {
    {1,4}, {2,4}, {6,4}, {7,4}, {4,1}, {4,2}, {4,6}, {4,7}
};
