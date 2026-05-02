/* eye_mouse_x11.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

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
    float top_gain;
    float bottom_gain;
    float left_gain;
    float right_gain;

    float prev_raw_x;
    float prev_raw_y;

    int prev_cursor_x;
    int prev_cursor_y;

    float smooth_x;
    float smooth_y;

} adaptive_t;

point_t p[3][3];

adaptive_t adapt =
{
    1.0f,
    1.0f,
    1.0f,
    1.0f,

    0.5f,
    0.5f,

    0,
    0,

    0.5f,
    0.5f
};

float gx=0.5f;
float gy=0.5f;

float sx=0.5f;
float sy=0.5f;

float cx=0;
float cy=0;

int screen_w=1920;
int screen_h=1080;

void get_config_path(char* out,size_t size)
{
    const char* home = getenv("HOME");

    snprintf(
        out,
        size,
        "%s/.local/tobii_4c/calx11.conf",
        home ? home : ".");
}

float clampf(float v,float a,float b)
{
    if(v<a) return a;
    if(v>b) return b;

    return v;
}

void warp(
    float raw_x,
    float raw_y,
    float* ox,
    float* oy)
{
    float min_x=p[0][0].raw_x;
    float max_x=p[0][2].raw_x;

    float min_y=p[0][0].raw_y;
    float max_y=p[2][0].raw_y;

    *ox=(raw_x-min_x)/(max_x-min_x);
    *oy=(raw_y-min_y)/(max_y-min_y);

    *ox=clampf(*ox,0,1);
    *oy=clampf(*oy,0,1);

    *ox=*ox*1.08f-0.04f;
    *oy=*oy*1.12f-0.06f;

    *ox=clampf(*ox,0,1);
    *oy=clampf(*oy,0,1);
}

void adaptive_warp(
    float* x,
    float* y,
    int cursor_x,
    int cursor_y,
    int screen_w,
    int screen_h)
{
    const int EDGE = 24;

    float raw_x = *x;
    float raw_y = *y;

    if(raw_y < adapt.prev_raw_y &&
       cursor_y < EDGE)
    {
        adapt.top_gain += 0.0015f;
    }

    if(raw_y > adapt.prev_raw_y &&
       cursor_y > screen_h - EDGE)
    {
        adapt.bottom_gain += 0.0015f;
    }

    if(raw_x < adapt.prev_raw_x &&
       cursor_x < EDGE)
    {
        adapt.left_gain += 0.0015f;
    }

    if(raw_x > adapt.prev_raw_x &&
       cursor_x > screen_w - EDGE)
    {
        adapt.right_gain += 0.0015f;
    }

    float top_w =
        clampf((0.25f - raw_y)/0.25f,0.0f,1.0f);

    float bottom_w =
        clampf((raw_y - 0.75f)/0.25f,0.0f,1.0f);

    float left_w =
        clampf((0.25f - raw_x)/0.25f,0.0f,1.0f);

    float right_w =
        clampf((raw_x - 0.75f)/0.25f,0.0f,1.0f);

    raw_y -= top_w * 0.08f * adapt.top_gain;
    raw_y += bottom_w * 0.08f * adapt.bottom_gain;

    raw_x -= left_w * 0.08f * adapt.left_gain;
    raw_x += right_w * 0.08f * adapt.right_gain;

    *x = clampf(raw_x,0.0f,1.0f);
    *y = clampf(raw_y,0.0f,1.0f);

    adapt.prev_raw_x = *x;
    adapt.prev_raw_y = *y;
}

void url_cb(const char* url,void* data)
{
    snprintf((char*)data,256,"%s",url);
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

    adaptive_warp(
        &gx,
        &gy,
        adapt.prev_cursor_x,
        adapt.prev_cursor_y,
        screen_w,
        screen_h);
}

int main()
{
    char cfg[PATH_MAX];

    get_config_path(cfg,sizeof(cfg));

    printf("Loading calibration: %s\n",cfg);

    FILE* f = fopen(cfg,"r");

    if(!f)
    {
        printf("Missing calibration file\n");
        return 1;
    }

    for(int y=0;y<3;y++)
    {
        for(int x=0;x<3;x++)
        {
            if(fscanf(
                f,
                "%f %f %f %f",
                &p[y][x].raw_x,
                &p[y][x].raw_y,
                &p[y][x].target_x,
                &p[y][x].target_y) != 4)
            {
                printf("Invalid calibration file\n");

                fclose(f);

                return 1;
            }
        }
    }

    fclose(f);

    Display* d = XOpenDisplay(NULL);

    if(!d)
    {
        printf("No X11 display\n");
        return 1;
    }

    int screen = DefaultScreen(d);

    screen_w = DisplayWidth(d, screen);
    screen_h = DisplayHeight(d, screen);

    printf("Screen: %dx%d\n",screen_w,screen_h);

    Window root = RootWindow(d,screen);

    cx = screen_w/2;
    cy = screen_h/2;

    tobii_api_t* api;
    tobii_device_t* dev;

    char url[256]={0};

    tobii_api_create(&api,NULL,NULL);

    tobii_enumerate_local_device_urls(
        api,
        url_cb,
        url);

    if(url[0]==0)
    {
        printf("No Tobii device found\n");
        return 1;
    }

    printf("Using device: %s\n",url);

    tobii_device_create(api,url,&dev);

    tobii_gaze_point_subscribe(
        dev,
        gaze_cb,
        NULL);

    int lx=screen_w/2;
    int ly=screen_h/2;

    while(1)
    {
        tobii_device_process_callbacks(dev);

        adapt.smooth_x =
            adapt.smooth_x*0.84f + gx*0.16f;

        adapt.smooth_y =
            adapt.smooth_y*0.84f + gy*0.16f;

        sx = adapt.smooth_x;
        sy = adapt.smooth_y;

        float tx = sx * screen_w;
        float ty = sy * screen_h;

        cx = cx*0.72f + tx*0.28f;
        cy = cy*0.72f + ty*0.28f;

        int x = clampf(cx,0,screen_w-1);
        int y = clampf(cy,0,screen_h-1);

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

        adapt.prev_cursor_x = x;
        adapt.prev_cursor_y = y;

        printf(
            "\rMouse %4d %4d",
            x,
            y);

        fflush(stdout);

        usleep(4000);
    }

    return 0;
}
