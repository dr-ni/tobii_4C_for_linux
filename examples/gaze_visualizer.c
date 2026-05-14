//gcc gaze_visualizer.c -o gaze_visualizer -I/usr/include -L/usr/lib/tobii -Wl,-rpath,/usr/lib/tobii -ltobii_stream_engine -lX11
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#define W 1280
#define H 720

float gx = 0.5f;
float gy = 0.5f;

float smooth_x = 0.5f;
float smooth_y = 0.5f;

/* ---------- correction ---------- */

float correct_x( float x )
{
    float c =
        ( x + 0.05f ) /
        ( 0.83f + 0.05f );

    if( c < 0 ) c = 0;
    if( c > 1 ) c = 1;

    return c;
}

float correct_y( float y )
{
    float c =
        ( y - 0.03f ) /
        ( 1.10f - 0.03f );

    if( c < 0 ) c = 0;
    if( c > 1 ) c = 1;

    return c;
}

/* ---------- tobii ---------- */

void url_cb(
    const char* url,
    void* data )
{
    snprintf(
        (char*)data,
        256,
        "%s",
        url );
}

void gaze_cb(
    const tobii_gaze_point_t* g,
    void* ud )
{
    (void) ud;

    if( g->validity != TOBII_VALIDITY_VALID )
        return;

    gx =
        correct_x(
            g->position_xy[0] );

    gy =
        correct_y(
            g->position_xy[1] );

    printf(
        "\rRAW %.3f %.3f   ",
        g->position_xy[0],
        g->position_xy[1] );

    fflush(stdout);
}

/* ---------- main ---------- */

int main()
{
    /* ---------- X11 ---------- */

    Display* d =
        XOpenDisplay(NULL);

    if(!d)
    {
        printf("No X11 display\n");
        return 1;
    }

    int s =
        DefaultScreen(d);

    Window w =
        XCreateSimpleWindow(
            d,
            RootWindow(d,s),
            0,
            0,
            W,
            H,
            0,
            0,
            0x000000 );

    XStoreName(
        d,
        w,
        "Tobii Gaze Visualizer" );

    XMapWindow(
        d,
        w );

    GC gc =
        DefaultGC(d,s);

    /*
        white drawing color
    */

    XSetForeground(
        d,
        gc,
        0xffffff );

    XFlush(d);

    sleep(1);

    /* ---------- Tobii ---------- */

    tobii_api_t* api;
    tobii_device_t* dev;

    char url[256]={0};

    tobii_api_create(
        &api,
        NULL,
        NULL );

    tobii_enumerate_local_device_urls(
        api,
        url_cb,
        url );

    printf(
        "\nDevice: %s\n",
        url );

    tobii_device_create(
        api,
        url,
        &dev );

    tobii_gaze_point_subscribe(
        dev,
        gaze_cb,
        NULL );

    printf(
        "Live gaze visualizer running...\n" );

    /* ---------- main loop ---------- */

    while(1)
    {
        tobii_wait_for_callbacks(
            1,
            &dev );

        tobii_device_process_callbacks(
            dev );

        /*
            smoothing
        */

        smooth_x =
            smooth_x * 0.85f +
            gx * 0.15f;

        smooth_y =
            smooth_y * 0.85f +
            gy * 0.15f;

        int x =
            (int)( smooth_x * W );

        int y =
            (int)( smooth_y * H );

        printf(
            "\rDRAW %d %d     ",
            x,
            y );

        fflush(stdout);

        XClearWindow(
            d,
            w );

        /*
            draw gaze point
        */

        XFillArc(
            d,
            w,
            gc,
            x - 15,
            y - 15,
            30,
            30,
            0,
            360 * 64 );

        XFlush(d);

        usleep(10000);
    }

    return 0;
}
