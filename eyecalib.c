/* calibration_x11.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#define POINTS 9
#define SAMPLE_COUNT 250

#define MAX_RETRIES 3
#define MIN_VALID_SAMPLES 80

typedef struct
{
    float raw_x;
    float raw_y;

    float target_x;
    float target_y;

} calib_point_t;

calib_point_t points[POINTS] =
{
    {0,0,0.1f,0.1f},
    {0,0,0.5f,0.1f},
    {0,0,0.9f,0.1f},

    {0,0,0.1f,0.5f},
    {0,0,0.5f,0.5f},
    {0,0,0.9f,0.5f},

    {0,0,0.1f,0.9f},
    {0,0,0.5f,0.9f},
    {0,0,0.9f,0.9f}
};

float gx=0.5f;
float gy=0.5f;

int valid=0;

void get_config_path(char* out,size_t size)
{
    const char* home = getenv("HOME");

    snprintf(
        out,
        size,
        "%s/.local/tobii_4c/calx11.conf",
        home ? home : ".");
}

void url_cb(const char* url, void* data)
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

    gx = g->position_xy[0];
    gy = g->position_xy[1];

    valid = 1;
}

void draw_target(
    Display* d,
    Window w,
    GC gc,
    int x,
    int y)
{
    XClearWindow(d,w);

    XFillArc(
        d,
        w,
        gc,
        x-20,
        y-20,
        40,
        40,
        0,
        360*64);

    XFlush(d);
}

void cleanup(
    Display* d,
    Window w)
{
    XUngrabPointer(d,CurrentTime);
    XUngrabKeyboard(d,CurrentTime);

    XDestroyWindow(d,w);
}

int main()
{
    Display* d = XOpenDisplay(NULL);

    if(!d)
    {
        printf("No X11 display\n");
        return 1;
    }

    int screen = DefaultScreen(d);

    int W = DisplayWidth(d, screen);
    int H = DisplayHeight(d, screen);

    printf("Screen: %dx%d\n",W,H);

    Window root = RootWindow(d,screen);

    XSetWindowAttributes attr;

    attr.override_redirect = True;

    Window w = XCreateWindow(
        d,
        root,
        0,
        0,
        W,
        H,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWOverrideRedirect,
        &attr);

    XMapRaised(d,w);

    XSetWindowBackground(
        d,
        w,
        BlackPixel(d,screen));

    XClearWindow(d,w);

    XGrabKeyboard(
        d,
        w,
        True,
        GrabModeAsync,
        GrabModeAsync,
        CurrentTime);

    XGrabPointer(
        d,
        w,
        True,
        ButtonPressMask,
        GrabModeAsync,
        GrabModeAsync,
        w,
        None,
        CurrentTime);

    XFlush(d);

    GC gc = XCreateGC(d,w,0,NULL);

    XSetForeground(
        d,
        gc,
        WhitePixel(d,screen));

    tobii_api_t* api;

    tobii_error_t err =
        tobii_api_create(&api,NULL,NULL);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf("tobii_api_create failed\n");

        cleanup(d,w);

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

        cleanup(d,w);

        tobii_api_destroy(api);

        return 1;
    }

    printf("Using device: %s\n",url);

    tobii_device_t* dev;

    err = tobii_device_create(api,url,&dev);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "tobii_device_create failed: %d\n",
            err);

        cleanup(d,w);

        tobii_api_destroy(api);

        return 1;
    }

    err = tobii_gaze_point_subscribe(
        dev,
        gaze_cb,
        NULL);

    if(err != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "gaze subscribe failed: %d\n",
            err);

        tobii_device_destroy(dev);

        cleanup(d,w);

        tobii_api_destroy(api);

        return 1;
    }

    if(system("mkdir -p ~/.local/tobii_4c") != 0)
    {
        printf("Failed to create config directory\n");
        tobii_gaze_point_unsubscribe(dev);
        tobii_device_destroy(dev);
        cleanup(d,w);
        tobii_api_destroy(api);
        return 1;
    }
    
    char cfg[PATH_MAX];
    get_config_path(cfg,sizeof(cfg));
    printf("Saving calibration: %s\n",cfg);
    FILE* f = fopen(cfg,"w");

    if(!f)
    {
        printf("Cannot write calibration file\n");
        tobii_gaze_point_unsubscribe(dev);
        tobii_device_destroy(dev);
        cleanup(d,w);
        tobii_api_destroy(api);
        return 1;
    }

    printf("Calibration starting...\n");

    for(int i=0;i<POINTS;i++)
    {
        int tx = points[i].target_x * W;
        int ty = points[i].target_y * H;

        draw_target(d,w,gc,tx,ty);

        sleep(2);

        float sx=0;
        float sy=0;

        int retries=0;
        int success=0;
        int final_count=0;

        while(retries < MAX_RETRIES)
        {
            sx=0;
            sy=0;

            int count=0;
            int loops=0;

            valid=0;

            while(count < SAMPLE_COUNT &&
                  loops < SAMPLE_COUNT*8)
            {
                tobii_device_process_callbacks(dev);

                if(valid)
                {
                    sx += gx;
                    sy += gy;
                    count++;
                }

                loops++;
                usleep(4000);
            }

            if(count >= MIN_VALID_SAMPLES)
            {
                success=1;
                final_count=count;
                break;
            }

            retries++;
            printf(
                "Point %d failed (%d/%d)\n",
                i+1,
                retries,
                MAX_RETRIES);
        }

        if(!success)
        {
            printf(
                "Calibration aborted after %d failed attempts\n",
                MAX_RETRIES);
            fclose(f);
            tobii_gaze_point_unsubscribe(dev);
            tobii_device_destroy(dev);
            cleanup(d,w);
            tobii_api_destroy(api);
            return 1;
        }

        points[i].raw_x = sx / final_count;
        points[i].raw_y = sy / final_count;

        fprintf(
            f,
            "%.6f %.6f %.6f %.6f\n",
            points[i].raw_x,
            points[i].raw_y,
            points[i].target_x,
            points[i].target_y);

        printf(
            "RAW %.6f %.6f -> TARGET %.1f %.1f\n",
            points[i].raw_x,
            points[i].raw_y,
            points[i].target_x,
            points[i].target_y);
    }

    fclose(f);
    tobii_gaze_point_unsubscribe(dev);
    tobii_device_destroy(dev);
    cleanup(d,w);
    tobii_api_destroy(api);
    printf("Saved calibration\n");
    return 0;
}
