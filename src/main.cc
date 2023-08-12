#include <string>
#include <iostream>

#include "gtp.h"
#include "config.h"

void parse_args(int argc, char ** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string val(argv[i]);

        if (val == "-h" || val == "--help") {
            printf("-h, --help: Show the arguments.\n");
            printf("-t, --threads: Number of search threads.\n");
            printf("-p, --playouts: Number of playouts per move.\n");
            printf("--main-time: The thinking time of a game.\n");
            exit(0);
        }

        if (val == "-t" || val == "--threads") {
            cfg_search_threads = std::stoi(argv[++i]);
        } else if (val == "-p" || val == "--playouts") {
            cfg_playouts = std::stoi(argv[++i]);
        } else if (val == "--main-time") {
            cfg_main_time = std::stoi(argv[++i]);
        } else if (val == "--analysis") {
            cfg_dump_analysis = true;
        }
    }
}

int main(int argc, char ** argv) {

    parse_args(argc, argv);
    gtp_loop();

    return 0;
}
