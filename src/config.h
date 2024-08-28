#ifndef CONFIG_H_INCLUDE
#define CONFIG_H_INCLUDE

#include <vector>
#include <array>

extern int cfg_node_expanding_thres;
extern int cfg_playouts;
extern int cfg_search_threads;
extern float cfg_fpu_value;
extern float cfg_c_uct;
extern bool cfg_dump_analysis;
extern int cfg_lag_buffer;
extern int cfg_main_time;
extern bool cfg_enable_resign;
extern FILE *cfg_search_file;
extern std::vector<std::array<int, 2>> cfg_hollow_pos;

#endif
