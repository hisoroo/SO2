#include "Client.h"
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <ncurses.h>
#include <queue>
#include <string>
#include <thread>

#define NUM_FIELDS 3

using namespace std;

queue<string> messageQueue;
mutex queueMutex;
condition_variable queueCond;

void messageReceiver(Client *client) {
    client->receiveMessageLoop(messageQueue, queueMutex, queueCond);
}

void messagePrinter(Client *client) {
    while (true) {
        unique_lock<mutex> lock(queueMutex);
        if (queueCond.wait_for(lock, chrono::seconds(1),
                               []() { return !messageQueue.empty(); })) {
            string msg = messageQueue.front();
            messageQueue.pop();
            lock.unlock();

            cout << msg << endl;
        }
    }
}

struct ConnectionDetails {
    string username;
    string ip;
    int port;
};

WINDOW *create_box(int height, int width) {
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    WINDOW *win = newwin(height, width, starty, startx);
    box(win, 0, 0);
    wrefresh(win);
    return win;
}

void show_help_dialog(WINDOW *parent) {
    int width = 52;
    int height = 9;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    WINDOW *popup = newwin(height, width, starty, startx);

    mvwaddch(popup, 0, 0, ACS_ULCORNER);
    mvwaddch(popup, 0, width - 1, ACS_URCORNER);
    mvwaddch(popup, height - 1, 0, ACS_LLCORNER);
    mvwaddch(popup, height - 1, width - 1, ACS_LRCORNER);
    mvwhline(popup, 0, 1, ACS_HLINE, width - 2);
    mvwhline(popup, height - 1, 1, ACS_HLINE, width - 2);
    mvwvline(popup, 1, 0, ACS_VLINE, height - 2);
    mvwvline(popup, 1, width - 1, ACS_VLINE, height - 2);

    wattron(popup, A_BOLD);
    char title[] = " Help ";
    int title_len = strlen(title);
    int title_x = 1 + ((width - 2 - title_len) / 2);
    mvwprintw(popup, 0, title_x, "%s", title);
    wattroff(popup, A_BOLD);

    const char *help_lines[] = {
        "- Use TAB to switch inputs",
        "- Press ENTER to connect",
        "- Type !exit! to leave the chat"
    };
    int num_lines = sizeof(help_lines) / sizeof(help_lines[0]);

    int start_line = 1 + ((height - 2 - num_lines) / 2);

    for (int i = 0; i < num_lines; i++) {
        int len = strlen(help_lines[i]);
        int x = 1 + ((width - 2 - len) / 2); 
        mvwprintw(popup, start_line + i, x, "%s", help_lines[i]);
    }

    wgetch(popup);

    werase(popup);
    wrefresh(popup);
    delwin(popup);
    touchwin(parent);
    wrefresh(parent);
}

void handle_keyboard(WINDOW *win, char *inputs[NUM_FIELDS], int positions[NUM_FIELDS][2], int max_lengths[NUM_FIELDS]) {
    int current = 0;
    int ch;
    int cursors[NUM_FIELDS] = {0};

    keypad(win, TRUE); 

    while (true) {
        int y = positions[current][0];
        int x = positions[current][1];
        wmove(win, y, x + cursors[current]);
        wrefresh(win);

        ch = wgetch(win);

        if (ch == '\t') {
            current = (current + 1) % NUM_FIELDS;
        } else if (ch == '\n') {
            break;
        } else if (ch == '?'){
            show_help_dialog(win);
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cursors[current] > 0) {
                cursors[current]--;
                inputs[current][cursors[current]] = '\0';
                mvwaddch(win, y, x + cursors[current], ' ');
                wmove(win, y, x + cursors[current]);
            }
        } else if (cursors[current] < max_lengths[current] && ch >= 32 && ch <= 126) {
            inputs[current][cursors[current]] = ch;
            mvwaddch(win, y, x + cursors[current], ch);
            cursors[current]++;
        }
    }
}

void draw_input_box(WINDOW *win, int y, int x, int width) {
    mvwaddch(win, y, x, ACS_ULCORNER);
    mvwhline(win, y, x + 1, ACS_HLINE, width - 2);
    mvwaddch(win, y, x + width - 1, ACS_URCORNER);

    mvwaddch(win, y + 1, x, ACS_VLINE);
    mvwprintw(win, y + 1, x + 1, "%*s", width - 2, ""); 
    mvwaddch(win, y + 1, x + width - 1, ACS_VLINE);

    mvwaddch(win, y + 2, x, ACS_LLCORNER);
    mvwhline(win, y + 2, x + 1, ACS_HLINE, width - 2);
    mvwaddch(win, y + 2, x + width - 1, ACS_LRCORNER);
}


void init_inputs(WINDOW *win, char *input1, char *input2, char *input3) {
    int box_width = getmaxx(win);
    int box_height = getmaxy(win);

    int username_label_len = 9; 
    int username_box_w = 28;
    int username_total_w = username_label_len + 1 + username_box_w;

    int ip_label_len = 3;  
    int ip_box_w = 18;

    int port_label_len = 5; 
    int port_box_w = 8;

    int ip_port_total_w = ip_label_len + 1 + ip_box_w + 2 + port_label_len + 1 + port_box_w;

    int form_width = std::max(username_total_w, ip_port_total_w);
    int form_x = (box_width - form_width) / 2;

    int form_height = 6;
    int form_y = (box_height - form_height) / 2;

    int username_y = form_y;
    int ip_y = form_y + 3;

    mvwprintw(win, username_y + 1, form_x, "Username:");
    draw_input_box(win, username_y, form_x + username_label_len + 1, username_box_w);

    mvwprintw(win, ip_y + 1, form_x, "IP:");
    draw_input_box(win, ip_y, form_x + ip_label_len + 1, ip_box_w);

    mvwprintw(win, ip_y + 1, form_x + ip_label_len + 1 + ip_box_w + 2, "Port:");
    draw_input_box(win, ip_y, form_x + ip_label_len + 1 + ip_box_w + 2 + port_label_len + 1, port_box_w);
    

    char *inputs[NUM_FIELDS] = {input1, input2, input3};
    int max_lengths[NUM_FIELDS] = {username_box_w - 2, ip_box_w - 2, port_box_w - 2};
    int positions[NUM_FIELDS][2] = {
        {username_y + 1, form_x + username_label_len + 2},
        {ip_y + 1, form_x + ip_label_len + 2},
        {ip_y + 1, form_x + ip_label_len + 1 + ip_box_w + 2 + port_label_len + 2}
    };

    handle_keyboard(win, inputs, positions, max_lengths);
}

int main() {
    ConnectionDetails conn;

    initscr();
    noecho();
    cbreak();

    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);
    const char *global_help_text = "type ? for help";
    int text_length = strlen(global_help_text);
    mvprintw(screen_height - 1, screen_width - text_length - 1, "%s", global_help_text);
    refresh();

    int box_height = 10;
    int box_width = 45;
    WINDOW *win = create_box(box_height, box_width);

    char username[27] = {0}; 
    char ip[17] = {0};      
    char port_str[6] = {0};

    init_inputs(win, username, ip, port_str);

    endwin();

    conn.username = string(username);
    conn.ip = string(ip);
    conn.port = atoi(port_str);

    try {
        Client *client = new Client(conn.ip.c_str(), conn.port, conn.username);

        thread receiver(messageReceiver, client);
        thread printer(messagePrinter, client);

        string message;
        cout << "Type your messages (type '!exit!' to quit):\n";
        while (getline(cin, message)) {
            if (message == "!exit!") {
                break;
            }
            message = "[" + client->getUsername() + "] " + message;
            client->sendMessage(message);
        }

        receiver.detach();
        printer.detach();
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << endl;
    }

    return 0;
}

