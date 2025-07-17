#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define HEIGHT 21
#define WIDTH 63
#define MAX_ZONES 10

struct DeadlyZone 
{
    int x;
    int y;
    int width;
    int height;
    bool active;
};

char static_maze[HEIGHT][WIDTH];
char render_buffer[HEIGHT][WIDTH];
struct Player 
{
    int x;
    int y;
};
struct Player player;
struct DeadlyZone zones[MAX_ZONES];
int game_tick = 0;

void load_maze_from_file(const char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (!file) 
    {
        perror("Maze seems to be lost in a maze!!");
        exit(1);
    }

    char line_buffer[WIDTH + 2];
    for (int y = 0; y < HEIGHT; y++) 
    {
        if (fgets(line_buffer, sizeof(line_buffer), file) != NULL) 
        {
            line_buffer[strcspn(line_buffer, "\r\n")] = 0;

            for (int x = 0; x < WIDTH; x++) 
            {
                if (x < strlen(line_buffer)) 
                {
                    static_maze[y][x] = line_buffer[x];
                } else {
                    static_maze[y][x] = ' ';
                }
            }
        }
    }
    fclose(file);
}

void setup() 
{
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(0);
    srand(time(NULL)); // Random number of Deadly Zones generation
    
    //initial player position
    player.x = 1;
    player.y = 1;
    for (int i = 0; i < MAX_ZONES; i++)
    {
        zones[i].active = false;
    }
}

void process_input() 
{
    int input = getch(); // Get key-press
    int next_x = player.x;
    int next_y = player.y;

    switch (input) 
    {
        case KEY_UP: next_y--; 
            break;
        case KEY_DOWN: next_y++;
            break;
        case KEY_LEFT: next_x--;
            break;
        case KEY_RIGHT: next_x++;
            break;
    }

    if (static_maze[next_y][next_x] != '#') 
    {
        player.x = next_x;
        player.y = next_y;
    }
}

void update_game_state() 
{
    if (game_tick % 280 == 0) 
    { //
        for (int i = 0; i < MAX_ZONES; i++) 
        {
            if (!zones[i].active) 
            {
                zones[i].active = true;
                zones[i].x = WIDTH - 6;
                zones[i].y = rand() % (HEIGHT - 5);
                zones[i].width = 5;
                zones[i].height = 5;
                break;
            }
        }
    }

    if (game_tick % 10 == 0) 
    {
        for (int i = 0; i < MAX_ZONES; i++) 
        {
            if (zones[i].active) 
            {
                zones[i].x--;
                if (zones[i].x + zones[i].width < 0) 
                {
                    zones[i].active = false;
                }
            }
        }
    }
}

void draw_frame() 
{
    for (int y = 0; y < HEIGHT; y++) 
    {
        for (int x = 0; x < WIDTH; x++) 
        {
            render_buffer[y][x] = static_maze[y][x];
        }
    }

    for (int i = 0; i < MAX_ZONES; i++) 
    {
        if (zones[i].active) {
            for (int y = 0; y < zones[i].height; y++) 
            {
                for (int x = 0; x < zones[i].width; x++) 
                {
                    int draw_x = zones[i].x + x;
                    int draw_y = zones[i].y + y;
                    if (draw_x >= 0 && draw_x < WIDTH && draw_y >= 0 && draw_y < HEIGHT) 
                    {
                        render_buffer[draw_y][draw_x] = '*';
                    }
                }
            }
        }
    }

    render_buffer[player.y][player.x] = 'o';
    render_buffer[19][62] = 'E';
    clear();
    for (int y = 0; y < HEIGHT; y++) 
    {
        for (int x = 0; x < WIDTH; x++) {
            mvaddch(y, x, render_buffer[y][x]);
        }
    }
    refresh();
}

int main(void) 
{
    setup();
    load_maze_from_file("level_1.txt");

    bool game_is_running = 1;
    while (game_is_running) 
    {
        game_tick++;
        process_input();
        update_game_state();
        draw_frame();

        if (static_maze[player.y][player.x] == 'E') 
        {
            game_is_running = false;
        }
        for (int i = 0; i < MAX_ZONES; i++) 
        {
            if (zones[i].active &&
                player.x >= zones[i].x &&
                player.x < zones[i].x + zones[i].width &&
                player.y >= zones[i].y &&
                player.y < zones[i].y + zones[i].height) 
                {
                game_is_running = false;
                }
        }

        usleep(16000); 
    }

    endwin();
    printf("Whewww, tough game!\n");
    return 0;
}
