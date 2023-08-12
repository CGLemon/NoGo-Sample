#ifndef NODE_H_INCLUDE
#define NODE_H_INCLUDE

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>

#include "game_state.h"

class Node {
public:
    explicit Node(int vertex);
    explicit Node(Node &&n);
    ~Node();

    bool expand_children(GameState &state, int &eval);

    Node *uct_select_child(int color);
    int get_vertex() const;
    int get_visits() const;
    double get_eval(int color, bool use_virtual_loss=false) const;

    void update(int val);
    bool is_expanded() const;
    void sort_children();

    std::vector<Node*> &get_children();
    int get_children_size() const;

    int count_nodes() const;

    Node *get_child(int vtx);
    Node *pop_child(int vtx);
    Node *get_best_child();

    void increment_virtual_loss();
    void decrement_virtual_loss();
    int get_virtual_loss() const;

private:
    void wait_expanded();

    std::vector<Node*> m_children;

    std::atomic<int> m_black_wins{0};
    std::atomic<int> m_visits{0};
    std::atomic<int> m_virtual_loss{0};
    std::atomic<bool> m_expanded{false};
    std::mutex m_mtx;

    int m_vertex;
};

#endif
