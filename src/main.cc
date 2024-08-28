#include <string>
#include <iostream>

#include "gtp.h"
#include "config.h"

void parse_args_and_loop(int argc, char ** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string val(argv[i]);

        if (val == "-h" || val == "--help") {
            std::cerr
                << "                      -h, --help: show the arguments\n"
                << "             -t, --threads <int>: number of search threads\n"
                << "            -p, --playouts <int>: number of playouts per move\n"
                << "--node-expanding-threshold <int>: expanding visits threshold\n"
                << "                --main-time<int>: the thinking time of a game\n"
                << "                      --analysis: show MCTS search status\n"
                << "                     --no-hollow: remove the hollow positions\n"
                << "                     --no-resign: disable the resign move when low win-rate\n";
            exit(0);
        }

        if (val == "-t" || val == "--threads") {
            cfg_search_threads = std::stoi(argv[++i]);
        } else if (val == "-p" || val == "--playouts") {
            cfg_playouts = std::stoi(argv[++i]);
        } else if (val == "--node-expanding-threshold") {
            cfg_node_expanding_thres = std::stoi(argv[++i]);
        } else if (val == "--main-time") {
            cfg_main_time = std::stoi(argv[++i]);
        } else if (val == "--analysis") {
            cfg_dump_analysis = true;
        } else if (val == "--no-hollow") {
            cfg_hollow_pos.clear();
        } else if (val == "--no-resign") {
            cfg_enable_resign = false;
        }
    }
    gtp_loop();
}

int main(int argc, char ** argv) {

    parse_args_and_loop(argc, argv);

    return 0;
}
