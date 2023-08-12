#ifndef SEARCH_H_INCLUDE
#define SEARCH_H_INCLUDE

#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>

#include "game_state.h"
#include "node.h"
#include "time_manager.h"

class Search {
public:
    Search(GameState &state);
    ~Search();

    int think();
    void time_setting(int main_time);

private:
    struct Monitor {
        std::mutex mutex;
        std::condition_variable cv;

        void wait() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock);
        }
        void notify(bool all) {
            if (all) {
                cv.notify_all();
            } else {
                cv.notify_one();
            }
        }
    };

    void prepare_root_node();
    void release_node(Node *n);
    void release_tree();

    bool advance_to_new_rootstate();
    void init_pool();
    void do_one_playout();
    bool playout_recursive(GameState &curr_state, Node *node, int &eval);

    void dump_analysis();

    GameState &m_root_state;
    GameState m_last_state;
    std::unique_ptr<Node> m_root_node{nullptr};

    std::atomic<int> m_playouts;

    std::mutex m_queue_mutex;
    std::queue<Node *> m_garbage_nodes;
    Monitor m_gc_monitor;

    std::atomic<bool> m_search_running;
    std::atomic<int> m_running_threads;
    Monitor m_search_monitor;

    std::atomic<bool> m_pool_running;
    std::vector<std::thread> m_pool;

    TimeManager m_time_manager;
};

#endif
