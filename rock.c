#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "rock.h"
#include <string.h>
// #include <termios.h>
// #include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>

#define MAX_DEVICES 32
#define KEY_STATE_SIZE 256

// gcc -O2 -lm -o out rock.c &&sudo ./out

static int kb_fd = -1;
static char key_states[KEY_STATE_SIZE] = {0};

static int find_keyboard_device(void)
{
    char path[256];
    unsigned long ev_bits = 0;

    for (int i = 0; i < MAX_DEVICES; i++)
    {
        snprintf(path, sizeof(path), "/dev/input/event%d", i);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd == -1)
            continue;

        if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), &ev_bits) != -1)
        {
            if (ev_bits & (1 << EV_KEY))
            {
                return fd;
            }
        }
        close(fd);
    }
    return -1;
}

int initialize(void)
{
    if (kb_fd != -1)
        return 1;

    kb_fd = find_keyboard_device();
    if (kb_fd == -1)
        return 0;

    if (ioctl(kb_fd, EVIOCGRAB, 1) == -1)
    {
        close(kb_fd);
        kb_fd = -1;
        return 0;
    }

    return 1;
}

void refresh_keys(void)
{
    if (kb_fd == -1)
        return;

    struct input_event ev;
    while (read(kb_fd, &ev, sizeof(ev)) == sizeof(ev))
    {
        if (ev.type == EV_KEY && ev.code < KEY_STATE_SIZE)
        {
            key_states[ev.code] = (ev.value >= 1) ? 1 : 0;
        }
    }
}

int is_key_pressed(int key_code)
{
    if (key_code < 0 || key_code >= KEY_STATE_SIZE)
        return 0;
    return key_states[key_code];
}

void disconnect(void)
{
    if (kb_fd != -1)
    {
        ioctl(kb_fd, EVIOCGRAB, 0);
        close(kb_fd);
        kb_fd = -1;
    }
    memset(key_states, 0, sizeof(key_states));
}

float dis(int x, int y, int tx, int ty)
{
    return sqrt((x - tx) * (x - tx) + (y - ty) * (y - ty));
}

struct par
{
    float life;
    float full;
    float x;
    float y;
    float xa;
    float ya;
    int act;
};

struct cam_s
{
    float x;
    float y;
};

float noise2d(int x, int y)
{
    // Combine coordinates with prime numbers and bit operations
    uint32_t hash = (x * 0x1e35a7bd) ^ (y * 0x9e3779b9);

    // Improve hash distribution through bit manipulation
    hash = (hash << 13) ^ hash;
    hash = hash * (hash * hash * 15731 + 789221) + 1376312589;

    // Convert to float in [0,1] range
    return (float)(hash & 0x7FFFFFFF) / 0x7FFFFFFF;
}

int cameraIndex = 0;

struct cam_s cam_ss[15] = {0};

void add_cam(float xc, float yc)
{
    cam_ss[cameraIndex].x = xc;
    cam_ss[cameraIndex].y = yc;

    cameraIndex = (cameraIndex + 1) % 15;
}

struct par parts[200] = {0};

void add_par(float xp, float yp, float xv, float yv, float lifeo)
{
    int id = rand() % 200;
    parts[id].x = xp;
    parts[id].y = yp;
    parts[id].xa = xv;
    parts[id].ya = yv;
    parts[id].act = 1;
    parts[id].life = lifeo;
    parts[id].full = lifeo;
    // fotts[currentIndex].x = xg;
    // fotts[currentIndex].y = yg;
    // fotts[currentIndex].ang = angg;
    // fotts[currentIndex].type = typg;
    // fotts[currentIndex].life = 5.0f;
    // fotts[currentIndex].is_act = 1;

    // currentIndex = (currentIndex + 1) % 50;
}

int smoke[] = {220, 214, 208, 202, 160, 88, 52, 234};

int cam_o_x = 0;
int cam_o_y = 0;

int is_act = 0;

int screen_x = 0;
int screen_y = 0;

int main()
{
    printf("Don't press any keys!!!\n");

    sleep(2);

    int *screen = NULL;
    srand(time(NULL));
    if (initialize() == 0)
    {
        fprintf(stderr, "Run as root (sudo) :3\n");
        return 1;
    }

    int old_row = 0;
    int old_col = 0;

    float xp = 32;
    float yp = 32;
    float xpa = 0;
    float ypa = 0;
    float rp = 0;
    float rpa = 0;

    srand(time(NULL));
    struct winsize ws;

    // Hide cursor
    printf("\e[?25l");

    // int tangent;

    // struct timespec sleep_time = {0, 66666666}; // 66.666 ms för 15 Hz

    // int space_is_pressed = 0;

    while (!is_key_pressed(KEY_ESC))
    {

        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

        if (old_row != ws.ws_row || old_col != ws.ws_col)
        {
            system("clear");
            screen_x = ws.ws_col;
            screen_y = (ws.ws_row - 3) * 2;
            // free(screen);
            // screen = malloc((screen_y*screen_x) * sizeof(int));
            screen = realloc(screen, (screen_y * screen_x) * sizeof(int));
            memset(screen, 0, (screen_y * screen_x) * sizeof(int));
        }

        old_row = ws.ws_row;
        old_col = ws.ws_col;

        memset(screen, 0, (screen_y * screen_x) * sizeof(int));

        refresh_keys();

        // if (is_key_pressed(KEY_UP)) {
        //     yp -= 1;
        // }

        //

        if (is_key_pressed(KEY_SPACE))
        {
            float angss = 0;
            if (is_key_pressed(KEY_LEFT))
            {
                rpa += 0.005;
                angss -= 0.3f;
            }

            if (is_key_pressed(KEY_RIGHT))
            {
                rpa -= 0.005;
                angss += 0.3f;
            }
            //    yp += 1;
            xpa -= sin(rp) * 0.05;
            ypa -= cos(rp) * 0.05;

            for (int fuck = 0; fuck < 50; fuck++)
            {

                float angel = (2.0f * (rand() / (float)RAND_MAX) - 1.0f);
                float speed = (rand() / (float)RAND_MAX) * 8.0f + 2.0f;
                float lifes = (rand() / (float)RAND_MAX) * 4.0f + 1.0f;

                float xpaa = sin(rp + angel * 0.3 + angss) * speed;
                float ypaa = cos(rp + angel * 0.3 + angss) * speed;

                add_par(xp + sin(rp) * 8, yp + cos(rp) * 8, xpaa + xpa, ypaa + ypa, lifes);
            }
        }

        is_act = 0;

        for (int parb = 0; parb < 200; parb++)
        {
            if (parts[parb].act == 1)
            {
                parts[parb].x += parts[parb].xa;
                parts[parb].y += parts[parb].ya;
                parts[parb].xa *= 0.95;
                parts[parb].ya *= 0.95;
                parts[parb].life -= 0.3;
                if (parts[parb].life <= 0)
                {
                    parts[parb].act = 0;
                }
                is_act += 1;
            }
        }

        // ypa += 0.01;

        xp += xpa;
        yp += ypa;
        rp += rpa;

        add_cam(xp, yp);

        float cam_x = 0;
        float cam_y = 0;

        for (int camera_soml2 = 0; camera_soml2 < 15; camera_soml2++)
        {
            cam_x += cam_ss[camera_soml2].x;
            cam_y += cam_ss[camera_soml2].y;
            // camera_time += cam_ss[camera_soml2].time;
        }

        cam_x /= 15;
        cam_y /= 15;

        cam_o_x = (int)floor(cam_x) - screen_x / 2;
        cam_o_y = (int)floor(cam_y) - screen_y / 2;

        // rp += 0.1;
        //  Move cursor to top-left corner
        printf("\033[H");

        for (int paro = 0; paro < 200; paro++)
        {
            if (parts[paro].act == 1)
            {
                int id = (int)floor(parts[paro].x - cam_o_x) + ((int)floor(parts[paro].y - cam_o_y) * screen_x);
                if (id >= 0 && id < screen_x * screen_y && floor(parts[paro].x - cam_o_x) < screen_x && floor(parts[paro].x - cam_o_x) >= 0 && floor(parts[paro].y - cam_o_y) < screen_y && floor(parts[paro].y - cam_o_y) >= 0)
                {
                    screen[id] = smoke[(int)floor((float)((float)1 - parts[paro].life / parts[paro].full) * 8)];
                }
            }
        }

        int jjx = floor(xp) - 32 - cam_o_x;
        int jjy = floor(yp) - 32 - cam_o_y;

        for (int bby = 0; bby < 64; bby++)
        {
            for (int bbx = 0; bbx < 64; bbx++)
            {
                float ang = atan2((bby + jjy) - (yp - cam_o_y), (bbx + jjx) - (xp - cam_o_x));
                float dio = dis((bbx + jjx), (bby + jjy), (xp - cam_o_x), (yp - cam_o_y)) * 1;

                ang += rp;

                int tex_x = floor((float)cos(ang) * dio + 0.5) + 16;
                int tex_y = floor((float)sin(ang) * dio + 0.5) + 16;

                if (tex_x > 0 && tex_x < 32 && tex_y > 0 && tex_y < 32)
                {
                    int pos = tex_y * 32 + tex_x;
                    unsigned char brightness = image2_raw[pos];
                    if (floor((float)((float)brightness / 255) * 100) != 10)
                    {
                        int id = (bbx + jjx) + ((bby + jjy) * screen_x);
                        if (id >= 0 && id < screen_x * screen_y && (bbx + jjx) < screen_x && (bbx + jjx) >= 0 && (bby + jjy) < screen_y && (bby + jjy) >= 0)
                        {
                            screen[id] = floor((float)((float)brightness / 255) * 23 + 232);
                        }
                    }
                }
            }
        }

        int row;
        int col;
        for (row = 0; row < screen_y / 2; row++)
        {
            for (col = 0; col < screen_x; col++)
            {
                // float random = (float)rand() / (float)RAND_MAX;

                // int over = 0;
                // int overb = 0;
                int color = screen[col + ((row * 2) * screen_x)]; // col/ws.ws_col;
                if (color == 0)
                {
                    if ((float)noise2d(col + cam_o_x, row * 2 + cam_o_y) > 0.99)
                    {
                        color = 255;
                    }
                }

                int colorb = screen[col + ((row * 2 + 1) * screen_x)]; // col/ws.ws_col;
                if (colorb == 0)
                {
                    if ((float)noise2d(col + cam_o_x, row * 2 + 1 + cam_o_y) > 0.99)
                    {
                        colorb = 255;
                    }
                }

                printf("\x1b[38;5;%dm\x1b[48;5;%dm▄\x1b[0m", (int)colorb, (int)color);

                // float ang = atan2(row*2-(yp-cam_o_y),col-(xp-cam_o_x));
                // float dio = dis(col,row*2,(xp-cam_o_x),(yp-cam_o_y))*1;

                // ang += rp;

                // int tex_x = floor((float) cos(ang)*dio+0.5)+16;
                // int tex_y = floor((float) sin(ang)*dio+0.5)+16;

                // if(tex_x > 0 && tex_x < 32 && tex_y > 0 && tex_y < 32){
                //     int pos = tex_y * 32 + tex_x;
                //     unsigned char brightness = image2_raw[pos];
                //     if(floor((float)((float) brightness/255)*100) != 10){
                //         color = floor((float)((float) brightness/255)*23+ 232);
                //         over = 1;
                //     }
                // }

                // float ang2 = atan2((row*2+1)-(yp-cam_o_y),col-(xp-cam_o_x));
                // float dio2 = dis(col,row*2+1,(xp-cam_o_x),(yp-cam_o_y))*1;

                // ang2 += rp;

                // int tex_x2 = floor((float) cos(ang2)*dio2+0.5)+16;
                // int tex_y2 = floor((float) sin(ang2)*dio2+0.5)+16;

                // if(tex_x2 > 0 && tex_x2 < 32 && tex_y2 > 0 && tex_y2 < 32){
                //     int pos2 = tex_y2 * 32 + tex_x2;
                //     unsigned char brightness2 = image2_raw[pos2];
                //     if(floor((float)((float) brightness2/255)*100) != 10){
                //         colorb = floor((float)((float) brightness2/255)*23+ 232);
                //         overb = 1;
                //     }
                // }

                // if(over == 0){
                //     for(int paro = 0;paro < 200;paro++){
                //         if(floor(parts[paro].x-cam_o_x) == col && floor(parts[paro].y-cam_o_y) == row*2 && parts[paro].act == 1){
                //             color = smoke[(int) floor((float)((float)1-parts[paro].life/parts[paro].full)*8)];
                //
                //         }
                //     }
                // }
                // if(overb == 0){
                //     for(int paro2 = 0;paro2 < 200;paro2++){
                //         if(floor(parts[paro2].x-cam_o_x) == col && floor(parts[paro2].y-cam_o_y) == row*2+1 && parts[paro2].act == 1){
                //             colorb = smoke[(int) floor((float)((float)1-parts[paro2].life/parts[paro2].full)*8)];
                //         }
                //     }
                // }

                // float dee = dis(col,row*2,ws.ws_col/4,ws.ws_row/2);
                // float deeb = dis(col,row*2+1,ws.ws_col/4,ws.ws_row/2);

                // if(dee < ws.ws_row/3+10 && dee > ws.ws_row/3-10){
                //   color = floor((float) ( (float)((float) cos((float) ((float) ((float)dee-(ws.ws_row/3))/10)*3.141592)+1)/2)*23);
                // }
                // if(deeb < ws.ws_row/3+10 && deeb > ws.ws_row/3-10){
                //   colorb = floor((float) ( (float)((float) cos((float) ((float) ((float)deeb-(ws.ws_row/3))/10)*3.141592)+1)/2)*23);
                // }

                // printf("\033[38;2;%d;%d;%dm▄\033[0m",color,color,color );

                // printf("\x1b[38;5;%dm██\x1b[0m", (int) color + 232);
            }
            printf("\n");
        }
        usleep(1000000 / 30);
    }

    // Show cursor again (in case the program is terminated)
    disconnect();
    system("clear");
    printf("\e[?25h");
    printf("you are a femboy :3 ");
    // close(fd);
    return 0;
}
