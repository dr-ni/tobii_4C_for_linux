/*
gcc -O2 -Wall -Wextra \
     -I/usr/include \
     eyemouse.c \
     -o eyemouse \
     -L/usr/lib/tobii \
     -Wl,-rpath,/usr/lib/tobii \
     -ltobii_stream_engine \
     -lX11 -lm -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>

#include <linux/input.h>

#include <X11/Xlib.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#define DEADZONE 3

typedef struct
{
    float raw_x;
    float raw_y;

    float target_x;
    float target_y;

} point_t;

typedef struct
{
    float rx[3];
    float ry[3];

    float tx[3];
    float ty[3];

} tri_t;

point_t p[3][3];

float gx=0.5f;
float gy=0.5f;

float sx=0.5f;
float sy=0.5f;

float cx=0.0f;
float cy=0.0f;

float gyro_dx=0.0f;
float gyro_dy=0.0f;

int screen_w=0;
int screen_h=0;


void print_help(const char* prog)
{
    printf(
        "Usage:\n"
        "  %s [config_file]\n"
        "\n"
        "Options:\n"
        "  -h --help     show help\n"
        "  --usegyro     use Quha gyro\n"
        "\n"
        "Examples:\n"
        "  %s\n"
        "  %s ~/.local/tobii_4c/calx11.conf\n"
        "  %s --usegyro\n",
        prog,
        prog,
        prog,
        prog);
}

int load_config(const char* path)
{
    FILE* f =
        fopen(path,"r");

    if(!f)
    {
        perror(path);
        return 0;
    }

    printf(
        "Loading calibration: %s\n",
        path);

    char line[256];

    int idx = 0;

    while(fgets(line,sizeof(line),f))
    {
        /*
            skip comments
        */

        if(line[0] == '#')
            continue;

        /*
            skip empty lines
        */

        if(line[0] == '\n')
            continue;

        float rx,ry,tx,ty;

        /*
            calibration point
        */

        if(sscanf(
            line,
            "%f %f %f %f",
            &rx,
            &ry,
            &tx,
            &ty) == 4)
        {
            int y = idx / 3;
            int x = idx % 3;

            if(idx < 9)
            {
                p[y][x].raw_x = rx;
                p[y][x].raw_y = ry;

                p[y][x].target_x = tx;
                p[y][x].target_y = ty;

                printf(
                    "CAL %d "
                    "raw=%f,%f "
                    "target=%f,%f\n",
                    idx,
                    rx,
                    ry,
                    tx,
                    ty);

                idx++;
            }
        }
        else
        {
            printf(
                "Ignoring line: %s",
                line);
        }
    }

    fclose(f);

    printf(
        "Loaded %d calibration points\n",
        idx);

    if(idx != 9)
    {
        fprintf(
            stderr,
            "Need exactly 9 calibration points\n");

        return 0;
    }

    return 1;
}


float clampf(float v,float a,float b)
{
    if(v<a) return a;
    if(v>b) return b;

    return v;
}

int bary(
    float px,float py,
    float ax,float ay,
    float bx,float by,
    float cx_,float cy_,
    float* u,
    float* v,
    float* w)
{
    float d =
        (by-cy_)*(ax-cx_) +
        (cx_-bx)*(ay-cy_);

    if(fabs(d) < 1e-6f)
        return 0;

    *u =
        ((by-cy_)*(px-cx_) +
         (cx_-bx)*(py-cy_)) / d;

    *v =
        ((cy_-ay)*(px-cx_) +
         (ax-cx_)*(py-cy_)) / d;

    *w =
        1.0f - *u - *v;

    return 1;
}

void warp(
    float raw_x,
    float raw_y,
    float* ox,
    float* oy)
{
    tri_t t[] =
    {
        {
            {p[0][0].raw_x,p[0][1].raw_x,p[1][0].raw_x},
            {p[0][0].raw_y,p[0][1].raw_y,p[1][0].raw_y},
            {p[0][0].target_x,p[0][1].target_x,p[1][0].target_x},
            {p[0][0].target_y,p[0][1].target_y,p[1][0].target_y}
        },

        {
            {p[1][0].raw_x,p[0][1].raw_x,p[1][1].raw_x},
            {p[1][0].raw_y,p[0][1].raw_y,p[1][1].raw_y},
            {p[1][0].target_x,p[0][1].target_x,p[1][1].target_x},
            {p[1][0].target_y,p[0][1].target_y,p[1][1].target_y}
        },

        {
            {p[0][1].raw_x,p[0][2].raw_x,p[1][1].raw_x},
            {p[0][1].raw_y,p[0][2].raw_y,p[1][1].raw_y},
            {p[0][1].target_x,p[0][2].target_x,p[1][1].target_x},
            {p[0][1].target_y,p[0][2].target_y,p[1][1].target_y}
        },

        {
            {p[1][1].raw_x,p[0][2].raw_x,p[1][2].raw_x},
            {p[1][1].raw_y,p[0][2].raw_y,p[1][2].raw_y},
            {p[1][1].target_x,p[0][2].target_x,p[1][2].target_x},
            {p[1][1].target_y,p[0][2].target_y,p[1][2].target_y}
        },

        {
            {p[1][0].raw_x,p[1][1].raw_x,p[2][0].raw_x},
            {p[1][0].raw_y,p[1][1].raw_y,p[2][0].raw_y},
            {p[1][0].target_x,p[1][1].target_x,p[2][0].target_x},
            {p[1][0].target_y,p[1][1].target_y,p[2][0].target_y}
        },

        {
            {p[2][0].raw_x,p[1][1].raw_x,p[2][1].raw_x},
            {p[2][0].raw_y,p[1][1].raw_y,p[2][1].raw_y},
            {p[2][0].target_x,p[1][1].target_x,p[2][1].target_x},
            {p[2][0].target_y,p[1][1].target_y,p[2][1].target_y}
        },

        {
            {p[1][1].raw_x,p[1][2].raw_x,p[2][1].raw_x},
            {p[1][1].raw_y,p[1][2].raw_y,p[2][1].raw_y},
            {p[1][1].target_x,p[1][2].target_x,p[2][1].target_x},
            {p[1][1].target_y,p[1][2].target_y,p[2][1].target_y}
        },

        {
            {p[2][1].raw_x,p[1][2].raw_x,p[2][2].raw_x},
            {p[2][1].raw_y,p[1][2].raw_y,p[2][2].raw_y},
            {p[2][1].target_x,p[1][2].target_x,p[2][2].target_x},
            {p[2][1].target_y,p[1][2].target_y,p[2][2].target_y}
        }
    };

    for(int i=0;i<8;i++)
    {
        float u,v,w;

        if(!bary(
            raw_x,raw_y,
            t[i].rx[0],t[i].ry[0],
            t[i].rx[1],t[i].ry[1],
            t[i].rx[2],t[i].ry[2],
            &u,&v,&w))
        {
            continue;
        }

        if(u >= -0.02f &&
           v >= -0.02f &&
           w >= -0.02f)
        {
            *ox =
                u*t[i].tx[0] +
                v*t[i].tx[1] +
                w*t[i].tx[2];

            *oy =
                u*t[i].ty[0] +
                v*t[i].ty[1] +
                w*t[i].ty[2];

            *ox = clampf(*ox,0.0f,1.0f);
            *oy = clampf(*oy,0.0f,1.0f);

            return;
        }
    }

    *ox = clampf(raw_x,0.0f,1.0f);
    *oy = clampf(raw_y,0.0f,1.0f);
}

int open_quha_device(void)
{
    DIR* dir =
        opendir("/dev/input/by-id");

    if(!dir)
    {
        perror("opendir");
        return -1;
    }

    struct dirent* de;

    while((de = readdir(dir)))
    {
        if(!strstr(de->d_name,"Quha"))
            continue;

        if(!strstr(de->d_name,"event"))
            continue;

        char linkpath[PATH_MAX];

        snprintf(
            linkpath,
            sizeof(linkpath),
            "/dev/input/by-id/%s",
            de->d_name);

        char target[PATH_MAX];

        ssize_t len =
            readlink(
                linkpath,
                target,
                sizeof(target)-1);

        if(len <= 0)
            continue;

        target[len] = 0;

        char final[PATH_MAX];

        snprintf(
            final,
            sizeof(final),
            "/dev/input/%s",
            strrchr(target,'/')+1);

        printf(
            "Using Quha device: %s\n",
            final);

        int fd =
            open(final,O_RDONLY);

        if(fd >= 0)
        {
            closedir(dir);
            return fd;
        }
    }

    closedir(dir);

    fprintf(
        stderr,
        "No Quha device found\n");

    return -1;
}

void* gyro_thread(void* arg)
{
    (void)arg;

    int fd =
        open_quha_device();

    if(fd < 0)
        return NULL;

    struct input_event ev;

    while(read(fd,&ev,sizeof(ev)) > 0)
    {
        if(ev.type != EV_REL)
            continue;

        if(ev.code == REL_X)
            gyro_dx += ev.value * 2.0f;

        if(ev.code == REL_Y)
            gyro_dy += ev.value * 2.0f;
    }

    close(fd);

    return NULL;
}

void url_cb(
    const char* url,
    void* user_data)
{
    snprintf(
        (char*)user_data,
        256,
        "%s",
        url);
}

void gaze_cb(
    const tobii_gaze_point_t* g,
    void* user_data)
{
    (void)user_data;

    if(g->validity != TOBII_VALIDITY_VALID)
        return;

    warp(
        g->position_xy[0],
        g->position_xy[1],
        &gx,
        &gy);
}

void get_config_path(
    char* out,
    size_t size)
{
    const char* home =
        getenv("HOME");

    snprintf(
        out,
        size,
        "%s/.local/tobii_4c/calx11.conf",
        home ? home : ".");
}

int main(int argc,char** argv)
{
    int use_gyro = 0;
    char cfg[PATH_MAX];

    get_config_path(
        cfg,
        sizeof(cfg));

    for(int i=1;i<argc;i++)
    {
        if(!strcmp(argv[i],"-h") ||
           !strcmp(argv[i],"--help"))
        {
            print_help(argv[0]);
            return 0;
        }

        if(!strcmp(argv[i],"--usegyro"))
        {
            use_gyro = 1;
            continue;
        }

        /*
            config path
        */

        snprintf(
            cfg,
            sizeof(cfg),
            "%s",
            argv[i]);
    }

    if(!load_config(cfg))
    {
        printf("Invalid calibration file\n");
        return 1;
    }

    Display* d =
        XOpenDisplay(NULL);

    if(!d)
    {
        printf(
            "No X11 display\n");

        return 1;
    }

    int screen =
        DefaultScreen(d);

    screen_w =
        DisplayWidth(d,screen);

    screen_h =
        DisplayHeight(d,screen);

    printf(
        "Screen: %dx%d\n",
        screen_w,
        screen_h);

    Window root =
        RootWindow(d,screen);

    cx = screen_w/2;
    cy = screen_h/2;

    tobii_api_t* api;
    tobii_device_t* dev;

    char url[256]={0};

    tobii_api_create(
        &api,
        NULL,
        NULL);

    tobii_enumerate_local_device_urls(
        api,
        url_cb,
        url);

    if(url[0] == 0)
    {
        printf("No Tobii device found\n");
        return 1;
    }

    printf(
        "Device: %s\n",
        url);

    tobii_device_create(
        api,
        url,
        &dev);

    tobii_gaze_point_subscribe(
        dev,
        gaze_cb,
        NULL);

    if(use_gyro)
    {
        pthread_t gt;

        pthread_create(
            &gt,
            NULL,
            gyro_thread,
            NULL);
        printf(
            "Gyro enabled\n");
    }
    else
    {
        printf(
            "Gyro disabled\n");
    }

    int lx=screen_w/2;
    int ly=screen_h/2;

    while(1)
    {
        tobii_device_process_callbacks(
            dev);

        sx =
            sx*0.88f +
            gx*0.12f;

        sy =
            sy*0.88f +
            gy*0.12f;

        float tx =
            sx * screen_w;

        float ty =
            sy * screen_h;

        cx =
            cx*0.78f +
            (tx + gyro_dx)*0.22f;

        cy =
            cy*0.78f +
            (ty + gyro_dy)*0.22f;

        gyro_dx *= 0.92f;
        gyro_dy *= 0.92f;

        int x =
            clampf(
                cx,
                0,
                screen_w-1);

        int y =
            clampf(
                cy,
                0,
                screen_h-1);

        if(abs(x-lx)<DEADZONE &&
           abs(y-ly)<DEADZONE)
        {
            usleep(4000);
            continue;
        }

        XWarpPointer(
            d,
            None,
            root,
            0,0,0,0,
            x,
            y);

        XFlush(d);

        lx=x;
        ly=y;

        printf(
            "\rMouse %4d %4d",
            x,
            y);

        fflush(stdout);

        usleep(4000);
    }

    return 0;
}
