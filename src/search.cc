#include <stack>
#include <sstream>
#include <string>
#include <iostream>

#include "search.h"
#include "board.h"
#include "config.h"

Search::Search(GameState &state) : m_root_state(state) {
    init_pool();
}

Search::~Search() {
    m_pool_running.store(false);
    m_search_monitor.notify(true);
    m_gc_monitor.notify(true);

    for (std::thread & worker: m_pool) {
        worker.join();
    }
}

void Search::init_pool() {
    auto search_worker = [this]() {
        while(m_pool_running.load(std::memory_order_relaxed)) {
            m_search_monitor.wait();
            m_running_threads.fetch_add(
                1, std::memory_order_relaxed);
            while (m_search_running.load(std::memory_order_relaxed)) {
                do_one_playout();
            }
            m_running_threads.fetch_sub(
                1, std::memory_order_relaxed);
        }
    };
    auto gc_worker = [this]() {
        while(m_pool_running.load(std::memory_order_relaxed)) {
            m_gc_monitor.wait();

            while (true) {
                Node *n = nullptr;
                {
                    std::unique_lock<std::mutex> lock(m_queue_mutex);
                    if (m_garbage_nodes.empty()) {
                        break;
                    }
                    n = m_garbage_nodes.front();
                    m_garbage_nodes.pop();
                }
                delete n;
            }
        }
    };

    m_running_threads.store(0, std::memory_order_relaxed);
    m_pool_running.store(true);

    int num_search_threads = cfg_search_threads;
    for (int i = 0; i < num_search_threads; ++i) {
        m_pool.emplace_back(search_worker);
    }
    m_pool.emplace_back(gc_worker);
    fprintf(cfg_search_file, "The search pool is ready\n");
}

void Search::time_setting(int main_time) {
    m_time_manager.time_setting(main_time, cfg_lag_buffer);
}

int Search::think() {
    prepare_root_node();

    if (m_root_node->get_children_size() == 0) {
        fprintf(cfg_search_file, "No legal move. I will resign.\n");
        return Board::RESIGN;
    }

    int max_playouts = cfg_playouts;
    int color = m_root_state.get_tomove();

    m_time_manager.clock(color, m_root_state);
    float thinking_time = m_time_manager.get_thinking_time(color);
    fprintf(cfg_search_file, "The thinking time is %.2f(sec).\n", thinking_time);

    m_playouts.store(0, std::memory_order_relaxed);
    m_search_running.store(true, std::memory_order_relaxed);
    m_search_monitor.notify(true);

    while (m_playouts.load(std::memory_order_relaxed) < max_playouts) {
        if (m_time_manager.should_stop(color)) {
            break;
        }
        std::this_thread::yield();
    }

    m_search_running.store(false, std::memory_order_relaxed);
    while (m_running_threads.load(std::memory_order_relaxed) != 0) {
        std::this_thread::yield();
    }

    m_time_manager.stop(color);

    if (cfg_dump_analysis) {
        dump_analysis();
    }
    m_last_state = m_root_state;

    fprintf(cfg_search_file,
        "Do %d playout(s). The win-rate is %.2f(%%). Time left is %.2f (sec).\n",
        m_playouts.load(std::memory_order_relaxed),
        100 * m_root_node->get_eval(color),
        m_time_manager.get_time_left(color));

    if (m_root_node->get_eval(color) < 0.2f && cfg_enable_resign) {
        fprintf(cfg_search_file, "The Win-rate looks bad. I will resign.\n");
        return Board::RESIGN;
    }
    Node *best_node = m_root_node->get_best_child();
    int best_move = best_node->get_vertex();
    return best_move;
}

void Search::do_one_playout() {
    GameState curr_state = m_root_state; // copy
    int eval;

    if (playout_recursive(curr_state, m_root_node.get(), eval)) {
        m_playouts.fetch_add(1, std::memory_order_relaxed);
    }
}

bool Search::playout_recursive(GameState &curr_state, Node *node, int &eval) {
    node->increment_virtual_loss();
    bool success = true;
    int color = curr_state.get_tomove();

    if (node->is_expanded()) {
        // not leaf
        Node *next = node->uct_select_child(color);
        curr_state.play_move(next->get_vertex(), color);
        success = playout_recursive(curr_state, next, eval);
    } else {
        // leaf 
        if (curr_state.is_gameover(color)) {
            if (color == Board::BLACK) {
                eval = 0; // white won
            } else {
                eval = 1; // black won
            }
        } else {
            if (node->get_visits() < cfg_node_expanding_thres) {
                eval = curr_state.rollouts();
            } else {
                success = node->expand_children(curr_state, eval);
            }
        }
    }

    if (success) {
        node->update(eval);
    }
    node->decrement_virtual_loss();

    return success; // true
}

void Search::prepare_root_node() {
    bool reused = advance_to_new_rootstate();

    if (!reused) {
        release_tree();
        m_root_node = std::make_unique<Node>(Board::NULL_VERTEX);

        int eval;
        m_root_node->expand_children(m_root_state, eval);
        m_root_node->update(eval);
    } else {
        fprintf(cfg_search_file, "Reused %d nodes.\n", m_root_node->count_nodes());
    }
}

void Search::release_node(Node *n) {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_garbage_nodes.emplace(n);
    }
    m_gc_monitor.notify(false);
}

void Search::release_tree() {
    if (m_root_node) {
        Node *p = m_root_node.release();
        release_node(p);
    }
}

bool Search::advance_to_new_rootstate() {
    if (!m_root_node) {
        return false;
    }

    const int depth =
        m_root_state.get_movenum() - m_last_state.get_movenum();

    if (depth < 0) {
        return false;
    }

    std::stack<int> move_list;
    GameState test = m_root_state;
    for (auto i = 0; i < depth; ++i) {
        move_list.emplace(test.get_last_move());
        test.undo_move();
    }

    if (test.board.compute_hash() !=
            m_last_state.board.compute_hash()) {
        return false;
    }

    while (!move_list.empty()) {
        int vtx = move_list.top();
        int color = m_last_state.get_tomove();

        Node *next = m_root_node->pop_child(vtx);
        Node *p = m_root_node.release();
        release_node(p);

        if (next) {
            m_root_node.reset(next);
        } else {
            return false;
        }

        m_last_state.play_move(vtx, color);
        move_list.pop();
    }

    if (m_root_state.board.compute_hash() !=
            m_last_state.board.compute_hash()) {
        return false;
    }

    if (!m_root_node->is_expanded()) {
        return false;
    }

    return true;
}

void Search::dump_analysis() {
    auto vertex_to_str = [](int vtx, GameState& state) {
        std::string out;
        if (vtx == Board::PASS) {
            out = "pass";
        } else if (vtx == Board::RESIGN) {
            out = "resign";
        } else if (vtx == Board::NULL_VERTEX) {
            out = "null";
        } else {
            const char *x_lable_map = "ABCDEFGHJKLMNOPQRST";
            int x = state.get_x(vtx);
            int y = state.get_y(vtx);
            out += x_lable_map[x];
            out += std::to_string(y+1);
        }
        return out;
    };

    std::ostringstream ss;
    m_root_node->sort_children();
    std::vector<Node*> children = m_root_node->get_children();

    int max_show_size = std::min((int)children.size(), 10);
    int color = m_root_state.get_tomove();
    for (int i = 0; i < max_show_size; ++i) {
        Node *n = children[i];
        int visits = n->get_visits();
        if (visits > 0) {
            ss << vertex_to_str(n->get_vertex(), m_root_state) << " -> "
                   << "V(" << 100 * (n->get_eval(color)) << "%), "
                   << "N(" << visits << ")" << std::endl;
        }
    }
    int remaining_size = children.size() - max_show_size;
    if (remaining_size > 0) {
        ss << "......Fold the other " << remaining_size << " node(s) status." << std::endl;
    }

    ss << "Tree has "
           << m_root_node->count_nodes()
           << " visited nodes." << std::endl;
    ss << "The root visits is "
           << m_root_node->get_visits()
           << "." << std::endl;
    fprintf(cfg_search_file, "%s", ss.str().c_str());
}
