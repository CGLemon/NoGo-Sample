#include <random>
#include <algorithm>

#include "game_state.h"
#include "random.h"

void GameState::clear_board(int board_size, float komi) {
    board.reset_board(board_size);

    m_game_history.clear();
    m_game_history.emplace_back(std::make_shared<Board>(board));

    m_komi = komi;
    m_movenum = 0;
}

std::vector<int> GameState::get_legal_moves(int color) const {
    std::vector<int> legal_moves;
    int board_size = board.get_board_size();

    for (int y = 0; y < board_size; ++y) {
        for (int x = 0; x < board_size; ++x) {
            int vtx = board.get_vertex(x,y);
            if (board.legal_move(vtx, color)) {
                legal_moves.emplace_back(vtx);
            }
        }
    }
    return legal_moves;
}

bool GameState::is_gameover(int color) const {
    auto legal_moves = get_legal_moves(color);
    return legal_moves.empty();
}

bool GameState::play_move(int vtx, int color) {
    if (!board.legal_move(vtx, color)) {
        return false;
    }
    if (vtx != Board::RESIGN) {
        board.play_move_assume_legal(vtx, color);
        m_game_history.resize(++m_movenum);
        m_game_history.emplace_back(std::make_shared<Board>(board));
    }
    return true;
}

bool GameState::play_move_fast(int vtx, int color) {
    if (!board.legal_move(vtx, color)) {
        return false;
    }
    if (vtx != Board::RESIGN) {
        board.play_move_assume_legal(vtx, color);
    }
    return true;
}

void GameState::undo_move() {
    if (m_movenum <= 0) return;

    m_game_history.resize(m_movenum--);
    board = *m_game_history[m_movenum];
}

int GameState::play_random_move(int color, bool use_fast) {
    auto legal_moves = get_legal_moves(color);  

    if (legal_moves.empty()) {
        return Board::RESIGN;
    }

    std::shuffle(std::begin(legal_moves),
        std::end(legal_moves), PRNG::get());

    int move;
    int size = legal_moves.size();
    for (int i = 0; i < size; ++i) {
        move = legal_moves[i];
        if (!board.is_eyeshape(move, color)) {
            break;
        }
    }

    if (use_fast) {
        play_move_fast(move, color);
    } else {
        play_move(move, color);
    }
    return move;
}

int GameState::rollouts() {
    auto fork_state = *this;
    int color, move;
    while (true) {
        color = fork_state.get_tomove();
        move = fork_state.play_random_move(color, true);

        if (move == Board::RESIGN) {
            break;
        }
    }
    int black_win = (color == Board::WHITE);
    return black_win;
}

bool GameState::legal_move(int vtx, int color) {
    return board.legal_move(vtx, color);
}

bool GameState::superko() {
    std::uint64_t hash = board.compute_hash();
    for (int i = 0; i < m_movenum-1; ++i) {
        if (hash == m_game_history[i]->compute_hash()) {
            return true;
        }
    }
    return false;
}

float GameState::final_score() {
    return board.compute_reach_color(Board::BLACK) -
               board.compute_reach_color(Board::WHITE) - m_komi;
}

int GameState::get_vertex(int x, int y) const {
    return board.get_vertex(x,y);
}

int GameState::get_index(int x, int y) const {
    return board.get_index(x,y);
}


void GameState::showboard() {
    std::string color_map[4] = {"black", "white", "empty", "invalid"};

    std::cerr << board.to_string();
    std::cerr << "{"
                  "Next: " << color_map[get_tomove()] << ", "
                  << "Moves: " << m_movenum 
                  << "}"
                  << std::endl;
    std::cerr << "Hash: " << std::hex << board.compute_hash() << std::endl;
}

int GameState::get_state(int vtx) const {
    return board.get_state(vtx);
};

int GameState::get_x(int vtx) const {
    return board.get_x(vtx);
}

int GameState::get_y(int vtx) const {
    return board.get_y(vtx);
}

int GameState::get_tomove() const {
    return board.get_tomove();
}

int GameState::get_last_move() const {
    return board.get_last_move();
}

int GameState::get_komove() const {
    return board.get_komove();
}

int GameState::get_board_size() const {
    return board.get_board_size();
}

int GameState::get_passes() const {
    return board.get_passes();
}

int GameState::get_movenum() const {
    return m_movenum;
}

float GameState::get_komi() const {
    return m_komi;
}

void GameState::set_komi(float komi) {
    m_komi = komi;
}

void GameState::set_to_move(int color) {
    board.set_to_move(color);
}
