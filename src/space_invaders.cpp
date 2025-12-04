// Space Invaders - ASCII clone for AWK interpreter easter egg
// Classic arcade gameplay with terminal graphics

#include "space_invaders.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#endif

// Game constants
constexpr int SCREEN_WIDTH = 60;
constexpr int SCREEN_HEIGHT = 22;
constexpr int ALIEN_ROWS = 4;
constexpr int ALIEN_COLS = 8;
constexpr int ALIEN_SPACING_X = 6;
constexpr int ALIEN_SPACING_Y = 2;
constexpr int PLAYER_Y = SCREEN_HEIGHT - 2;
constexpr int INITIAL_LIVES = 3;
constexpr int ALIEN_SHOOT_CHANCE = 50;  // 1 in N chance per frame

// Platform-specific terminal functions
#ifdef _WIN32

void clear_screen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void hide_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void show_cursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

bool key_pressed() {
    return _kbhit() != 0;
}

int get_key() {
    int ch = _getch();
    // Handle arrow keys (they send 0 or 224 followed by another code)
    if (ch == 0 || ch == 224) {
        ch = _getch();
        switch (ch) {
            case 75: return 'a';  // Left arrow
            case 77: return 'd';  // Right arrow
            case 72: return 'w';  // Up arrow (fire)
        }
    }
    return ch;
}

#else  // Unix/Linux/macOS

static struct termios old_termios;
static bool termios_saved = false;

void set_raw_mode() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    termios_saved = true;
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void restore_mode() {
    if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    }
}

void clear_screen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

void hide_cursor() {
    std::cout << "\033[?25l" << std::flush;
}

void show_cursor() {
    std::cout << "\033[?25h" << std::flush;
}

bool key_pressed() {
    fd_set fds;
    struct timeval tv = {0, 0};
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

int get_key() {
    int ch = getchar();
    // Handle escape sequences for arrow keys
    if (ch == 27) {  // ESC
        if (key_pressed()) {
            int ch2 = getchar();
            if (ch2 == '[' && key_pressed()) {
                int ch3 = getchar();
                switch (ch3) {
                    case 'A': return 'w';  // Up
                    case 'B': return 's';  // Down
                    case 'C': return 'd';  // Right
                    case 'D': return 'a';  // Left
                }
            }
        }
        return 'q';  // ESC alone = quit
    }
    return ch;
}

#endif

// Game structures
struct Alien {
    int x, y;
    bool alive;
    int type;  // 0, 1, 2 for different alien types
};

struct Bullet {
    int x, y;
    bool active;
    bool player_owned;
};

struct Barrier {
    int x, y;
    int health;  // 0-4, degrades when hit
};

struct Player {
    int x;
    int lives;
    int score;
};

class SpaceInvaders {
private:
    Player player;
    std::vector<Alien> aliens;
    std::vector<Bullet> bullets;
    std::vector<Barrier> barriers;
    int alien_direction;  // 1 = right, -1 = left
    bool game_over;
    bool victory;
    int frame_count;
    int alien_move_delay;
    std::vector<std::string> screen;

    // Alien sprites (different types)
    const char* alien_sprites[3] = {
        "/@@\\",  // Type 0 - top row
        "<##>",  // Type 1 - middle rows
        "{^^}"   // Type 2 - bottom row
    };

    const char* player_sprite = "/_A_\\";
    const int player_width = 5;
    const int alien_width = 4;

public:
    void init() {
        player.x = SCREEN_WIDTH / 2 - player_width / 2;
        player.lives = INITIAL_LIVES;
        player.score = 0;

        alien_direction = 1;
        game_over = false;
        victory = false;
        frame_count = 0;
        alien_move_delay = 8;

        // Initialize aliens
        aliens.clear();
        for (int row = 0; row < ALIEN_ROWS; ++row) {
            for (int col = 0; col < ALIEN_COLS; ++col) {
                Alien a;
                a.x = 4 + col * ALIEN_SPACING_X;
                a.y = 2 + row * ALIEN_SPACING_Y;
                a.alive = true;
                a.type = row == 0 ? 0 : (row < 3 ? 1 : 2);
                aliens.push_back(a);
            }
        }

        // Initialize barriers
        barriers.clear();
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                Barrier b;
                b.x = 8 + i * 14 + j;
                b.y = SCREEN_HEIGHT - 5;
                b.health = 4;
                barriers.push_back(b);
            }
        }

        bullets.clear();

        // Initialize screen buffer
        screen.resize(SCREEN_HEIGHT);
        for (auto& line : screen) {
            line.resize(SCREEN_WIDTH, ' ');
        }

        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    void handle_input() {
        while (key_pressed()) {
            int ch = get_key();
            switch (ch) {
                case 'a':
                case 'A':
                    if (player.x > 0) player.x--;
                    break;
                case 'd':
                case 'D':
                    if (player.x < SCREEN_WIDTH - player_width) player.x++;
                    break;
                case 'w':
                case 'W':
                case ' ':
                    // Fire bullet (limit active player bullets)
                    {
                        int player_bullets = 0;
                        for (const auto& b : bullets) {
                            if (b.active && b.player_owned) player_bullets++;
                        }
                        if (player_bullets < 3) {
                            Bullet b;
                            b.x = player.x + player_width / 2;
                            b.y = PLAYER_Y - 1;
                            b.active = true;
                            b.player_owned = true;
                            bullets.push_back(b);
                        }
                    }
                    break;
                case 'q':
                case 'Q':
                    game_over = true;
                    break;
            }
        }
    }

    void update() {
        frame_count++;

        // Move aliens
        if (frame_count % alien_move_delay == 0) {
            bool should_descend = false;

            // Check if any alien hit edge
            for (const auto& a : aliens) {
                if (!a.alive) continue;
                if ((alien_direction > 0 && a.x + alien_width >= SCREEN_WIDTH - 1) ||
                    (alien_direction < 0 && a.x <= 1)) {
                    should_descend = true;
                    break;
                }
            }

            if (should_descend) {
                alien_direction = -alien_direction;
                for (auto& a : aliens) {
                    a.y++;
                    // Check if aliens reached player level
                    if (a.alive && a.y >= PLAYER_Y - 1) {
                        game_over = true;
                    }
                }
                // Speed up as aliens descend
                if (alien_move_delay > 2) alien_move_delay--;
            } else {
                for (auto& a : aliens) {
                    a.x += alien_direction;
                }
            }
        }

        // Alien shooting
        if (frame_count % 10 == 0) {
            std::vector<Alien*> alive_aliens;
            for (auto& a : aliens) {
                if (a.alive) alive_aliens.push_back(&a);
            }
            if (!alive_aliens.empty() && std::rand() % ALIEN_SHOOT_CHANCE == 0) {
                Alien* shooter = alive_aliens[std::rand() % alive_aliens.size()];
                Bullet b;
                b.x = shooter->x + alien_width / 2;
                b.y = shooter->y + 1;
                b.active = true;
                b.player_owned = false;
                bullets.push_back(b);
            }
        }

        // Move bullets
        for (auto& b : bullets) {
            if (!b.active) continue;
            if (b.player_owned) {
                b.y--;
                if (b.y < 0) b.active = false;
            } else {
                b.y++;
                if (b.y >= SCREEN_HEIGHT) b.active = false;
            }
        }

        // Check collisions - player bullets vs aliens
        for (auto& b : bullets) {
            if (!b.active || !b.player_owned) continue;
            for (auto& a : aliens) {
                if (!a.alive) continue;
                if (b.x >= a.x && b.x < a.x + alien_width &&
                    b.y >= a.y && b.y <= a.y + 1) {
                    a.alive = false;
                    b.active = false;
                    // Score based on alien type
                    player.score += (3 - a.type) * 10 + 10;
                    break;
                }
            }
        }

        // Check collisions - alien bullets vs player
        for (auto& b : bullets) {
            if (!b.active || b.player_owned) continue;
            if (b.y == PLAYER_Y && b.x >= player.x && b.x < player.x + player_width) {
                b.active = false;
                player.lives--;
                if (player.lives <= 0) {
                    game_over = true;
                }
            }
        }

        // Check collisions - bullets vs barriers
        for (auto& b : bullets) {
            if (!b.active) continue;
            for (auto& bar : barriers) {
                if (bar.health <= 0) continue;
                if (b.x == bar.x && b.y == bar.y) {
                    b.active = false;
                    bar.health--;
                    break;
                }
            }
        }

        // Remove inactive bullets
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.active; }),
            bullets.end()
        );

        // Check victory
        bool any_alive = false;
        for (const auto& a : aliens) {
            if (a.alive) {
                any_alive = true;
                break;
            }
        }
        if (!any_alive) {
            victory = true;
            game_over = true;
        }
    }

    void render() {
        // Clear screen buffer
        for (auto& line : screen) {
            std::fill(line.begin(), line.end(), ' ');
        }

        // Draw borders
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            screen[y][0] = '|';
            screen[y][SCREEN_WIDTH - 1] = '|';
        }
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            screen[0][x] = '-';
            screen[SCREEN_HEIGHT - 1][x] = '-';
        }
        screen[0][0] = '+';
        screen[0][SCREEN_WIDTH - 1] = '+';
        screen[SCREEN_HEIGHT - 1][0] = '+';
        screen[SCREEN_HEIGHT - 1][SCREEN_WIDTH - 1] = '+';

        // Draw aliens
        for (const auto& a : aliens) {
            if (!a.alive) continue;
            const char* sprite = alien_sprites[a.type];
            for (int i = 0; i < alien_width && a.x + i < SCREEN_WIDTH - 1; ++i) {
                if (a.x + i > 0 && a.y > 0 && a.y < SCREEN_HEIGHT - 1) {
                    screen[a.y][a.x + i] = sprite[i];
                }
            }
        }

        // Draw barriers
        for (const auto& b : barriers) {
            if (b.health <= 0) continue;
            if (b.x > 0 && b.x < SCREEN_WIDTH - 1 && b.y > 0 && b.y < SCREEN_HEIGHT - 1) {
                char c = '#';
                if (b.health == 3) c = 'X';
                else if (b.health == 2) c = 'x';
                else if (b.health == 1) c = '.';
                screen[b.y][b.x] = c;
            }
        }

        // Draw bullets
        for (const auto& b : bullets) {
            if (!b.active) continue;
            if (b.x > 0 && b.x < SCREEN_WIDTH - 1 && b.y > 0 && b.y < SCREEN_HEIGHT - 1) {
                screen[b.y][b.x] = b.player_owned ? '|' : '*';
            }
        }

        // Draw player
        for (int i = 0; i < player_width; ++i) {
            if (player.x + i > 0 && player.x + i < SCREEN_WIDTH - 1) {
                screen[PLAYER_Y][player.x + i] = player_sprite[i];
            }
        }

        // Output to console
        clear_screen();

        std::cout << "  SPACE INVADERS - AWK Edition\n";
        std::cout << "  Score: " << player.score << "  Lives: ";
        for (int i = 0; i < player.lives; ++i) std::cout << "<3 ";
        std::cout << "\n";

        for (const auto& line : screen) {
            std::cout << line << "\n";
        }

        std::cout << "  [A/D or Arrows] Move  [Space/W] Fire  [Q] Quit\n";
        std::cout << std::flush;
    }

    int run() {
        init();

        hide_cursor();

#ifndef _WIN32
        set_raw_mode();
#endif

        // Show title screen
        clear_screen();
        std::cout << R"(

     #####  #     # #    #
    #     # #  #  # #   #
    #     # #  #  # #  #
    #######  # # #  ###
    #     #  # # #  #  #
    #     #  # # #  #   #
    #     #   # #   #    #

    ### #   # #   #  ###  ####  ##### ####   ####  ###
     #  ##  # #   # #   # #   # #     #   # #     ###
     #  # # # #   # ##### #   # ####  ####   ###   #
     #  #  ##  # #  #   # #   # #     #   #     # ###
    ### #   #   #   #   # ####  ##### #   # ####   #

                    *** AWK EDITION ***

               Press any key to start...
                      [Q] to quit

)" << std::flush;

        // Wait for key
        while (!key_pressed()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        int ch = get_key();
        if (ch == 'q' || ch == 'Q') {
            show_cursor();
#ifndef _WIN32
            restore_mode();
#endif
            return 0;
        }

        // Main game loop
        while (!game_over) {
            handle_input();
            update();
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Game over screen
        clear_screen();
        std::cout << "\n\n";
        if (victory) {
            std::cout << R"(
    #     # ##### #   #   #     # ##### #   #   #
    #   #   #   # #   #   #     #   #   ##  #   #
     # #    #   # #   #   #  #  #   #   # # #   #
      #     #   # #   #   # # # #   #   #  ##
      #     ##### #####    #   #  ##### #   #   #

         *** CONGRATULATIONS! YOU WIN! ***

)" << std::flush;
        } else {
            std::cout << R"(
      ####   ###  #   # #####   ###  #   # ##### ####
     #      #   # ## ## #      #   # #   # #     #   #
     # ###  ##### # # # ####   #   # #   # ####  ####
     #   #  #   # #   # #      #   #  # #  #     #   #
      ####  #   # #   # #####   ###    #   ##### #   #

              *** BETTER LUCK NEXT TIME! ***

)" << std::flush;
        }
        std::cout << "\n";
        std::cout << "                 Final Score: " << player.score << "\n\n";
        std::cout << "              Press any key to exit...\n" << std::flush;

        // Wait for key
        while (!key_pressed()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        get_key();

        show_cursor();
#ifndef _WIN32
        restore_mode();
#endif
        clear_screen();

        return 0;
    }
};

int run_space_invaders() {
    SpaceInvaders game;
    return game.run();
}
