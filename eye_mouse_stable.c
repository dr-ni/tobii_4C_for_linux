
//gcc eye_mouse_stable.c -o eye_mouse_stable -I/usr/include -L/usr/lib/tobii -Wl,-rpath,/usr/lib/tobii -ltobii_stream_engine -lX11 -lm
/* eye_mouse_x11.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

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

float cx=0;
float cy=0;

int screen_w=0;
int screen_h=0;

float clampf(float v,float a,float b)
{
    if(v<a) return a;
    if(v>b) return b;

    return v;
}

int bary(
    float px,float py,
    float x0,float y0,
    float x1,float y1,
    float x2,float y2,
    float* u,float* v,float* w)
{
    float den =
        (y1-y2)*(x0-x2)+
        (x2-x1)*(y0-y2);

    if(fabsf(den)<1e-6f)
        return 0;

    *u=((y1-y2)*(px-x2)+(x2-x1)*(py-y2))/den;

    *v=((y2-y0)*(px-x2)+(x0-x2)*(py-y2))/den;

    *w=1.0f-*u-*v;

    return 1;
}

int inside(float u,float v,float w)
{
    return
        u>=-0.2f &&
        v>=-0.2f &&
        w>=-0.2f;
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

        if(inside(u,v,w))
        {
            *ox =
                u*t[i].tx[0] +
                v*t[i].tx[1] +
                w*t[i].tx[2];

            *oy =
                u*t[i].ty[0] +
                v*t[i].ty[1] +
                w*t[i].ty[2];

            /*
                no expansion anymore
            */

            *ox = clampf(*ox,0.0f,1.0f);
            *oy = clampf(*oy,0.0f,1.0f);

            return;
        }
    }

    *ox = clampf(raw_x,0.0f,1.0f);
    *oy = clampf(raw_y,0.0f,1.0f);
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

    /*
        only mesh warp
    */

    warp(
        g->position_xy[0],
        g->position_xy[1],
        &gx,
        &gy);
}

int main()
{
    FILE* f = fopen("calx11.conf","r");

    if(!f)
    {
        printf("Missing calx11.conf\n");
        return 1;
    }

    for(int y=0;y<3;y++)
    {
        for(int x=0;x<3;x++)
        {
            fscanf(
                f,
                "%f %f %f %f",
                &p[y][x].raw_x,
                &p[y][x].raw_y,
                &p[y][x].target_x,
                &p[y][x].target_y);
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

    printf("Device: %s\n",url);

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

        /*
            light smoothing only
        */

        sx =
            sx*0.88f + gx*0.12f;

        sy =
            sy*0.88f + gy*0.12f;

        /*
            edge damping
        */

        float ex = sx;
        float ey = sy;

        if(ex < 0.08f)
        {
            ex =
                0.08f *
                powf(ex / 0.08f,1.35f);
        }

        if(ex > 0.92f)
        {
            float t =
                (1.0f - ex) / 0.08f;

            ex =
                1.0f -
                0.08f *
                powf(t,1.35f);
        }

        if(ey < 0.08f)
        {
            ey =
                0.08f *
                powf(ey / 0.08f,1.35f);
        }

        if(ey > 0.92f)
        {
            float t =
                (1.0f - ey) / 0.08f;

            ey =
                1.0f -
                0.08f *
                powf(t,1.35f);
        }

        float tx = ex * screen_w;
        float ty = ey * screen_h;

        /*
            cursor smoothing
        */

        cx = cx*0.78f + tx*0.22f;
        cy = cy*0.78f + ty*0.22f;

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

        printf(
            "\rMouse %4d %4d",
            x,
            y);

        fflush(stdout);

        usleep(4000);
    }

    return 0;
}
