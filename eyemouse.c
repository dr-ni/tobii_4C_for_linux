/*
gcc eyemouse.c -o eyemouse \
-O2 -Wall -Wextra \
-I/usr/include \
-L/usr/lib/tobii \
-Wl,-rpath,/usr/lib/tobii \
-ltobii_stream_engine \
-lX11 -lm
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

#include <X11/Xlib.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#define DEADZONE 3

#define ENABLE_SMOOTHING     1
#define ENABLE_EDGE_DAMPING  1

typedef struct
{
    float raw_x;
    float raw_y;

    float target_x;
    float target_y;

} calib_point_t;

typedef struct
{
    int version;

    float sensor_offset_x;
    float sensor_offset_y;

    float screen_distance;
    float screen_tilt;

    float gaze_smooth;
    float cursor_smooth;

    float edge_zone;
    float edge_power;

} runtime_config_t;

runtime_config_t cfg =
{
    .version = 2,

    .sensor_offset_x = 0.0f,
    .sensor_offset_y = 0.0f,

    .screen_distance = 63.0f,
    .screen_tilt = 0.0f,

    .gaze_smooth = 0.12f,
    .cursor_smooth = 0.22f,

    .edge_zone = 0.08f,
    .edge_power = 1.35f
};

calib_point_t calib[9];

float gx=0.5f;
float gy=0.5f;

float sx=0.5f;
float sy=0.5f;

float cx=0.0f;
float cy=0.0f;

int valid=0;

float clampf2(float v,float a,float b)
{
    if(v < a) return a;
    if(v > b) return b;

    return v;
}

void get_config_path(char* out,size_t size)
{
    const char* home = getenv("HOME");

    snprintf(
        out,
        size,
        "%s/.local/tobii_4c/calx11.conf",
        home ? home : ".");
}

int load_config(const char* path)
{
    FILE* f = fopen(path,"r");

    if(!f)
        return 0;

    char line[512];

    int idx=0;

    while(fgets(line,sizeof(line),f))
    {
        if(line[0]=='#' ||
           line[0]=='\n')
            continue;

        sscanf(
            line,
            "sensor_offset_x %f",
            &cfg.sensor_offset_x);

        sscanf(
            line,
            "sensor_offset_y %f",
            &cfg.sensor_offset_y);

        sscanf(
            line,
            "screen_distance %f",
            &cfg.screen_distance);

        sscanf(
            line,
            "screen_tilt %f",
            &cfg.screen_tilt);

        sscanf(
            line,
            "gaze_smooth %f",
            &cfg.gaze_smooth);

        sscanf(
            line,
            "cursor_smooth %f",
            &cfg.cursor_smooth);

        sscanf(
            line,
            "edge_zone %f",
            &cfg.edge_zone);

        sscanf(
            line,
            "edge_power %f",
            &cfg.edge_power);

        float rx,ry,tx,ty;

        if(sscanf(
            line,
            "%f %f %f %f",
            &rx,
            &ry,
            &tx,
            &ty) == 4)
        {
            if(idx < 9)
            {
                calib[idx].raw_x = rx;
                calib[idx].raw_y = ry;

                calib[idx].target_x = tx;
                calib[idx].target_y = ty;
            }

            idx++;
        }
    }

    fclose(f);

    return 1;
}

void url_cb(
    const char* url,
    void* data)
{
    snprintf(
        (char*)data,
        256,
        "%s",
        url);
}

void gaze_cb(
    const tobii_gaze_point_t* g,
    void* user_data)
{
    (void)user_data;

    valid = 0;

    if(g->validity != TOBII_VALIDITY_VALID)
        return;

    if(g->position_xy[0] < 0.0f ||
       g->position_xy[0] > 1.0f ||
       g->position_xy[1] < 0.0f ||
       g->position_xy[1] > 1.0f)
    {
        return;
    }

    gx =
        g->position_xy[0] +
        cfg.sensor_offset_x;

    gy =
        g->position_xy[1] +
        cfg.sensor_offset_y;

    gx = clampf2(gx,0.0f,1.0f);
    gy = clampf2(gy,0.0f,1.0f);

    valid = 1;
}

int main()
{
    char cfgpath[PATH_MAX];

    get_config_path(
        cfgpath,
        sizeof(cfgpath));

    if(!load_config(cfgpath))
    {
        printf(
            "No calibration config found:\n%s\n",
            cfgpath);

        return 1;
    }

    printf(
        "Loaded calibration: %s\n",
        cfgpath);

    Display* d = XOpenDisplay(NULL);

    if(!d)
    {
        printf("No X11 display\n");
        return 1;
    }

    int screen =
        DefaultScreen(d);

    int screen_w =
        DisplayWidth(d,screen);

    int screen_h =
        DisplayHeight(d,screen);

    printf(
        "Screen: %dx%d\n",
        screen_w,
        screen_h);

    Window root =
        RootWindow(d,screen);

    tobii_api_t* api;

    tobii_error_t err =
        tobii_api_create(
            &api,
            NULL,
            NULL);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "tobii_api_create failed\n");

        return 1;
    }

    char url[256]={0};

    tobii_enumerate_local_device_urls(
        api,
        url_cb,
        url);

    if(url[0] == 0)
    {
        printf("No Tobii device found\n");

        tobii_api_destroy(api);

        return 1;
    }

    printf("Using device: %s\n",url);

    tobii_device_t* dev;

    err =
        tobii_device_create(
            api,
            url,
            &dev);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "tobii_device_create failed\n");

        tobii_api_destroy(api);

        return 1;
    }

    err =
        tobii_gaze_point_subscribe(
            dev,
            gaze_cb,
            NULL);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "gaze subscribe failed\n");

        tobii_device_destroy(dev);

        tobii_api_destroy(api);

        return 1;
    }

    int lx=screen_w/2;
    int ly=screen_h/2;

    cx=lx;
    cy=ly;

    printf("Eye mouse running...\n");

    while(1)
    {
        valid=0;

        tobii_device_process_callbacks(dev);

        if(!valid)
        {
            usleep(4000);
            continue;
        }

#if ENABLE_SMOOTHING

        sx =
            sx*(1.0f-cfg.gaze_smooth) +
            gx*cfg.gaze_smooth;

        sy =
            sy*(1.0f-cfg.gaze_smooth) +
            gy*cfg.gaze_smooth;

#else

        sx=gx;
        sy=gy;

#endif

        float ex=sx;
        float ey=sy;

#if ENABLE_EDGE_DAMPING

        if(ex < cfg.edge_zone)
        {
            ex =
                cfg.edge_zone *
                powf(
                    ex/cfg.edge_zone,
                    cfg.edge_power);
        }

        if(ex > 1.0f-cfg.edge_zone)
        {
            float t =
                (1.0f-ex) /
                cfg.edge_zone;

            ex =
                1.0f -
                cfg.edge_zone *
                powf(
                    t,
                    cfg.edge_power);
        }

        if(ey < cfg.edge_zone)
        {
            ey =
                cfg.edge_zone *
                powf(
                    ey/cfg.edge_zone,
                    cfg.edge_power);
        }

        if(ey > 1.0f-cfg.edge_zone)
        {
            float t =
                (1.0f-ey) /
                cfg.edge_zone;

            ey =
                1.0f -
                cfg.edge_zone *
                powf(
                    t,
                    cfg.edge_power);
        }

#endif

        float tx =
            ex * screen_w;

        float ty =
            ey * screen_h;

        cx =
            cx*(1.0f-cfg.cursor_smooth) +
            tx*cfg.cursor_smooth;

        cy =
            cy*(1.0f-cfg.cursor_smooth) +
            ty*cfg.cursor_smooth;

        int mx =
            clampf2(
                cx,
                0,
                screen_w-1);

        int my =
            clampf2(
                cy,
                0,
                screen_h-1);

        if(abs(mx-lx) < DEADZONE &&
           abs(my-ly) < DEADZONE)
        {
            usleep(4000);
            continue;
        }

        XWarpPointer(
            d,
            None,
            root,
            0,
            0,
            0,
            0,
            mx,
            my);

        XFlush(d);

        lx=mx;
        ly=my;

        printf(
            "\rMouse %4d %4d  gaze %.3f %.3f   ",
            mx,
            my,
            gx,
            gy);

        fflush(stdout);

        usleep(4000);
    }

    tobii_gaze_point_unsubscribe(dev);

    tobii_device_destroy(dev);

    tobii_api_destroy(api);

    XCloseDisplay(d);

    return 0;
}
