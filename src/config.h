#ifndef CONFIG_H_INCLUDE
#define CONFIG_H_INCLUDE

#include <vector>
#include <array>

extern int cfg_node_expanding_thres;
extern int cfg_playouts;
extern int cfg_search_threads;
extern int cfg_fpu_value;
extern int cfg_c_uct;
extern bool cfg_dump_analysis;
extern int cfg_lag_buffer;
extern int cfg_main_time;
extern FILE *cfg_search_file;
extern std::vector<std::array<int, 2>> cfg_hollow_pos;

#endif
