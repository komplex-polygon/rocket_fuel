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
#include <limits.h>
#include <dirent.h>

#define MAX_DEVICES 32
#define KEY_STATE_SIZE 256

// gcc -O2 -lm -o out rock.c &&sudo ./out

static int kb_fd = -1;
static char key_states[KEY_STATE_SIZE] = {0};

int keyb[32] = {0};
int keyint = 0;

void list_usb_input_devices()
{
    DIR *dir = opendir("/sys/class/input");
    if (!dir)
        return;

    struct dirent *entry;
    char path[PATH_MAX], name[256], resolved[PATH_MAX];

    while ((entry = readdir(dir)))
    {
        if (strncmp(entry->d_name, "event", 5))
            continue;

        snprintf(path, sizeof(path), "/sys/class/input/%s/device/name", entry->d_name);
        FILE *f = fopen(path, "r");
        if (!f)
            continue;

        fgets(name, sizeof(name), f);
        fclose(f);
        name[strcspn(name, "\n")] = 0;

        snprintf(path, sizeof(path), "/sys/class/input/%s/device", entry->d_name);
        if (realpath(path, resolved) && strstr(resolved, "/usb"))
        {
            printf("%-3d %s\n", keyint, name);
            keyb[keyint] = (int)atoi(entry->d_name + 5);
            keyint++;
        }
    }
    closedir(dir);
}

static int find_keyboard_device(int idll2)
{
    char path[256];
    unsigned long ev_bits = 0;

    for (int i = 0; i < 1; i++)
    {
        snprintf(path, sizeof(path), "/dev/input/event%d", idll2);
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

int initialize(int idll)
{
    if (kb_fd != -1)
        return 1;

    kb_fd = find_keyboard_device(idll);
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

float sphere_volume_cm3(float radius_cm)
{
    return (4.0 / 3.0) * 3.141592 * radius_cm * radius_cm * radius_cm;
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
    float r;
};

struct plant
{
    float x;
    float xa;
    float y;
    float ya;
    float size;
    float mass;
    int is_act;
    int color;
};

float noise2d(int x, int y)
{

    uint32_t hash = (x * 0x1e35a7bd) ^ (y * 0x9e3779b9);

    hash = (hash << 13) ^ hash;
    hash = hash * (hash * hash * 15731 + 789221) + 1376312589;

    return (float)(hash & 0x7FFFFFFF) / 0x7FFFFFFF;
}

int cameraIndex = 0;

struct cam_s cam_ss[15] = {0};
struct plant planets[15] = {0};

void add_cam(float xc, float yc, float rc)
{
    cam_ss[cameraIndex].x = xc;
    cam_ss[cameraIndex].y = yc;
    cam_ss[cameraIndex].r = rc;

    cameraIndex = (cameraIndex + 1) % 15;
}

float invq(float dist)
{
    return 1.0f / ((float)dist * (float)dist);
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
}

int smoke[] = {220, 214, 208, 202, 160, 88, 52, 234};

float cam_o_x = 0;
float cam_o_y = 0;
float cam_o_r = 0;

int is_act = 0;

int screen_x = 0;
int screen_y = 0;

int main()
{
    system("clear");
    printf("chuse your keybord. :3\n");
    list_usb_input_devices();
    int keee;

    printf("enter keybord number :3 :");
    fflush(stdout);

    scanf("%d", &keee);

    system("clear");

    if (keee < 0 || keee >= keyint)
    {
        return 0;
    }

    printf("Don't press any keys!!!\n");

    sleep(2);

    int *screen = NULL;
    srand(time(NULL));
    if (initialize(keyb[keee]) == 0)
    {
        fprintf(stderr, "Run as root (sudo) :3\n");
        return 1;
    }

    int old_row = 0;
    int old_col = 0;

    float xp = 0;
    float yp = 100;
    float xpa = 0;
    float ypa = 2;
    float rp = 0;
    float rpa = 0;

    srand(time(NULL));
    struct winsize ws;

    printf("\e[?25l");

    planets[0].is_act = 1;
    planets[0].size = 20;
    // planets[0].x = 200;
    planets[0].mass = 200;
    planets[0].color = 230;

    planets[1].is_act = 1;
    planets[1].size = 5;
    planets[1].x = 150;
    // planets[1].y = 200;
    planets[1].ya = 1;
    planets[1].mass = 50;
    planets[1].color = 35;

    while (!is_key_pressed(KEY_ESC))
    {

        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

        if (old_row != ws.ws_row || old_col != ws.ws_col)
        {
            system("clear");
            screen_x = ws.ws_col;
            screen_y = (ws.ws_row - 3) * 2;

            screen = realloc(screen, (screen_y * screen_x) * sizeof(int));
            memset(screen, 0, (screen_y * screen_x) * sizeof(int));
        }

        old_row = ws.ws_row;
        old_col = ws.ws_col;

        memset(screen, 0, (screen_y * screen_x) * sizeof(int));

        refresh_keys();

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

        if (is_key_pressed(KEY_SPACE))
        {

            xpa -= sin(rp) * 0.1;
            ypa -= cos(rp) * 0.1;

            for (int fuck = 0; fuck < 50; fuck++)
            {

                float angel = (2.0f * (rand() / (float)RAND_MAX) - 1.0f);
                float speed = (rand() / (float)RAND_MAX) * 8.0f + 2.0f;
                float lifes = (rand() / (float)RAND_MAX) * 4.0f + 1.0f;

                float xpaa = sin(rp + angel * 0.2 + angss) * speed;
                float ypaa = cos(rp + angel * 0.2 + angss) * speed;

                add_par(xp + sin(rp) * 4, yp + cos(rp) * 4, xpaa + xpa, ypaa + ypa, lifes);
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

        for (int pl = 0; pl < 15; pl++)
        {
            if (planets[pl].is_act == 1)
            {
                float dol = dis(planets[pl].x, planets[pl].y, xp, yp);
                float grav = invq(dol) * planets[pl].mass;
                float fx = (planets[pl].x - xp) / dol * grav;
                float fy = (planets[pl].y - yp) / dol * grav;
                xpa += fx;
                ypa += fy;
            }
        }

        for (int pl2 = 0; pl2 < 15; pl2++)
        {
            if (planets[pl2].is_act == 1)
            {
                for (int pl = 0; pl < 15; pl++)
                {
                    if (planets[pl].is_act == 1 && pl != pl2)
                    {
                        float dol = dis(planets[pl].x, planets[pl].y, planets[pl2].x, planets[pl2].y);
                        // dol = isnan(dol) ? 0.0f : dol;
                        float grav = invq(dol) * planets[pl].mass;
                        float fx = (planets[pl].x - planets[pl2].x) / dol * grav;
                        float fy = (planets[pl].y - planets[pl2].y) / dol * grav;
                        planets[pl2].xa += fx;
                        planets[pl2].ya += fy;
                        // printf("%f   %f    %f   %f       ",planets[pl].x,planets[pl].y,planets[pl2].x,planets[pl2].y);
                        // return 0;
                    }
                }
            }
        }

        for (int pl2 = 0; pl2 < 15; pl2++)
        {
            if (planets[pl2].is_act == 1)
            {
                planets[pl2].x += planets[pl2].xa;
                planets[pl2].y += planets[pl2].ya;
            }
        }

        xp += xpa;
        yp += ypa;
        rp += rpa;

        add_cam(xp, yp, rp);

        float cam_x = 0;
        float cam_y = 0;
        float cam_r = 0;

        for (int camera_soml2 = 0; camera_soml2 < 15; camera_soml2++)
        {
            cam_x += cam_ss[camera_soml2].x;
            cam_y += cam_ss[camera_soml2].y;
            cam_r += cam_ss[camera_soml2].r;
        }

        cam_x /= 15;
        cam_y /= 15;
        cam_r /= 15;

        cam_o_x = (float)cam_x; // - screen_x / 2;
        cam_o_y = (float)cam_y; // - screen_y / 2;
        cam_o_r = (float)cam_r;

        printf("\033[H");

        for (int paro = 0; paro < 200; paro++)
        {
            if (parts[paro].act == 1)
            {
                float prot = atan2(parts[paro].y - cam_o_y, parts[paro].x - cam_o_x);
                float pdis = dis(parts[paro].x, parts[paro].y, cam_o_x, cam_o_y);
                prot += cam_o_r;
                float prx = (float)cos(prot) * pdis + screen_x / 2;
                float pry = (float)sin(prot) * pdis + screen_y / 2;
                int id = (int)floor(prx) + ((int)floor(pry) * screen_x);
                if (id >= 0 && id < screen_x * screen_y && floor(prx) < screen_x && floor(prx) >= 0 && floor(pry) < screen_y && floor(pry) >= 0)
                {
                    screen[id] = smoke[(int)floor((float)((float)1 - parts[paro].life / parts[paro].full) * 8)];
                }
            }
        }

        float span = atan2(yp - cam_o_y, xp - cam_o_x);
        float spdi = dis(xp, yp, cam_o_x, cam_o_y);
        span += cam_o_r;
        float srx = cos(span) * spdi + screen_x / 2;
        float sry = sin(span) * spdi + screen_y / 2;

        int jjx = (int)floor(srx) - 16;
        int jjy = (int)floor(sry) - 16;

        for (int bby = 0; bby < 32; bby++)
        {
            for (int bbx = 0; bbx < 32; bbx++)
            {
                // int id1 = (bbx + jjx) + ((bby + jjy) * screen_x);
                // screen[id1] = 255;

                float ang = atan2(bby - 16, bbx - 16);
                float dio = dis(bbx, bby, 16, 16) * 2;

                ang += rp - cam_o_r;

                int tex_x = floor((float)cos(ang) * dio + 0.5) + 16;
                int tex_y = floor((float)sin(ang) * dio + 0.5) + 16;

                if (tex_x > 0 && tex_x < 32 && tex_y > 0 && tex_y < 32)
                {
                    // int id2 = (bbx + jjx) + ((bby + jjy) * screen_x);
                    // screen[id2] = 250;
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

                int color = screen[col + ((row * 2) * screen_x)]; // col/ws.ws_col;
                if (color == 0)
                {
                    float scrot = atan2((float)(row * 2) - screen_y / 2, (float)col - screen_x / 2);
                    float sgdis = dis((float)col, (row * 2), (float)screen_x / 2, (float)screen_y / 2);
                    scrot -= cam_o_r;
                    float prx = (float)cos(scrot) * sgdis + cam_o_x;
                    float pry = (float)sin(scrot) * sgdis + cam_o_y;
                    if ((float)noise2d((int)floor(prx), (int)floor(pry)) > 0.99)
                    {
                        color = 255;
                    }
                    for (int plas = 0; plas < 15; plas++)
                    {
                        if (dis(prx, pry, planets[plas].x, planets[plas].y) < planets[plas].size)
                        {
                            color = planets[plas].color;
                        }
                    }
                }

                int colorb = screen[col + ((row * 2 + 1) * screen_x)]; // col/ws.ws_col;
                if (colorb == 0)
                {
                    float scrot = atan2((float)(row * 2 + 1) - screen_y / 2, (float)col - screen_x / 2);
                    float sgdis = dis((float)col, (row * 2 + 1), (float)screen_x / 2, (float)screen_y / 2);
                    scrot -= cam_o_r;
                    float prx = (float)cos(scrot) * sgdis + cam_o_x;
                    float pry = (float)sin(scrot) * sgdis + cam_o_y;
                    if ((float)noise2d((int)floor(prx), (int)floor(pry)) > 0.99)
                    {
                        colorb = 255;
                    }
                    for (int plas = 0; plas < 15; plas++)
                    {
                        if (dis(prx, pry, planets[plas].x, planets[plas].y) < planets[plas].size && planets[plas].is_act == 1)
                        {
                            colorb = planets[plas].color;
                        }
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

    disconnect();
    system("clear");
    printf("\e[?25h");
    printf("Have a good day.\n");
    sleep(1);
    printf("I realy mean that. ;3\n");
    sleep(2);
    system("clear");

    return 0;
}
