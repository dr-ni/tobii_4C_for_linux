/*
gcc eyecalib.c -o eyecalib \
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
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#define POINTS 9
#define SAMPLE_COUNT 250

#define TARGET_OUTER 22
#define TARGET_MIDDLE 20
#define TARGET_INNER 6
#define TARGET_CROSS 36

#define MAX_RETRIES 3
#define MIN_VALID_SAMPLES 80

typedef struct
{
    float raw_x;
    float raw_y;

    float target_x;
    float target_y;

} point_t;

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

point_t p[3][3];

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

void url_cb(
    const char* url,
    void* user_data)
{
<<<<<<< HEAD
    snprintf(
        (char*)user_data,
        256,
        "%s",
        url);
=======
    const char* db =
        "/var/lib/tobii/da-setups.db";

    FILE* f = fopen(db,"r");

    if(!f)
        return 0;
    fprintf(stderr,"import_tobii_db\n");
    char line[2048];

    while(fgets(line,sizeof(line),f))
    {
        if(strstr(line,"DisplayArea"))
        {
            cfg.sensor_offset_x = 0.002f;
            cfg.sensor_offset_y = -0.003f;
        }
    }

    fclose(f);

    return 1;
}

void save_config(const char* path)
{
    FILE* f = fopen(path,"w");

    if(!f)
        return;

    fprintf(f,"version %d\n",cfg.version);

    fprintf(f,
        "sensor_offset_x %.6f\n",
        cfg.sensor_offset_x);

    fprintf(f,
        "sensor_offset_y %.6f\n",
        cfg.sensor_offset_y);

    fprintf(f,
        "screen_distance %.3f\n",
        cfg.screen_distance);

    fprintf(f,
        "screen_tilt %.3f\n",
        cfg.screen_tilt);

    fprintf(f,
        "gaze_smooth %.3f\n",
        cfg.gaze_smooth);

    fprintf(f,
        "cursor_smooth %.3f\n",
        cfg.cursor_smooth);

    fprintf(f,
        "edge_zone %.3f\n",
        cfg.edge_zone);

    fprintf(f,
        "edge_power %.3f\n",
        cfg.edge_power);

    fprintf(f,
        "# calibration\n");

    for(int y=0;y<3;y++)
    {
        for(int x=0;x<3;x++)
        {
            fprintf(
                f,
                "%.6f %.6f %.6f %.6f\n",
                p[y][x].raw_x,
                p[y][x].raw_y,
                p[y][x].target_x,
                p[y][x].target_y);
        }
    }

    fclose(f);
}

void url_cb(const char* url,void* data)
{
    snprintf((char*)data,256,"%s",url);
>>>>>>> 56a1078 (Revert to stable version)
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

    valid = 1;
}

int import_tobii_db()
{
    char db[PATH_MAX];

    snprintf(
        db,
        sizeof(db),
        "%s/.config/TobiiProEyeTrackerManager/db/da-setups.db",
        getenv("HOME"));

    printf(
        "Opening Tobii DB: %s\n",
        db);

    FILE* f = fopen(db,"r");

    if(!f)
    {
        printf(
            "No Tobii DB bootstrap found\n");

        return 0;
    }

    char line[8192];

    int found=0;

    while(fgets(line,sizeof(line),f))
    {
        if(!(strstr(line,"\"name\":\"NucBox\"") ||
             strstr(line,"\"name\":\"uwe1\"")))
        {
            continue;
        }

        printf(
            "\n[DB ACTIVE PROFILE]\n%s\n",
            line);

        found=1;

        if(strstr(line,"\"trackerPlacement\":\"under\""))
        {
            printf(
                "[DB] tracker placement: UNDER\n");

            cfg.sensor_offset_y =
                -0.004f;
        }

        char* a =
            strstr(
                line,
                "\"tracker\":{\"angle\":");

        if(a)
        {
            float angle=0.0f;

            sscanf(
                a,
                "\"tracker\":{\"angle\":%f",
                &angle);

            printf(
                "[DB] tracker angle: %.2f deg\n",
                angle);

            cfg.screen_tilt =
                angle;
        }

        char* w =
            strstr(line,"\"width\":");

        if(w)
        {
            float width_mm=0.0f;

            sscanf(
                w,
                "\"width\":%f",
                &width_mm);

            printf(
                "[DB] display width: %.2f mm\n",
                width_mm);
        }

        char* h =
            strstr(line,"\"height\":");

        if(h)
        {
            float height_mm=0.0f;

            sscanf(
                h,
                "\"height\":%f",
                &height_mm);

            printf(
                "[DB] display height: %.2f mm\n",
                height_mm);
        }

        float blx,bly,blz;
        float tlx,tly,tlz;
        float trx,try_,trz;

        int matched =
            sscanf(
                line,
                "%*[^[][%f,%f,%f],\"topLeft\":[%f,%f,%f],\"topRight\":[%f,%f,%f]",
                &blx,&bly,&blz,
                &tlx,&tly,&tlz,
                &trx,&try_,&trz);

        if(matched == 9)
        {
            printf(
                "[DB] physical geometry parsed\n");

            printf(
                "     bottomLeft : %.2f %.2f %.2f\n",
                blx,bly,blz);

            printf(
                "     topLeft    : %.2f %.2f %.2f\n",
                tlx,tly,tlz);

            printf(
                "     topRight   : %.2f %.2f %.2f\n",
                trx,try_,trz);

            float screen_skew =
                (trz - blz);

            printf(
                "[DB] screen skew z: %.2f\n",
                screen_skew);

            cfg.sensor_offset_y =
                -(screen_skew / 5000.0f);

            printf(
                "[DB] derived sensor_offset_y = %.5f\n",
                cfg.sensor_offset_y);
        }
    }

    fclose(f);

    if(!found)
    {
        printf(
            "[DB] no matching local profile found\n");

        return 0;
    }

    printf(
        "\n[DB] bootstrap import complete\n");

    printf(
        "[DB] runtime config:\n");

    printf(
        "     sensor_offset_x = %.5f\n",
        cfg.sensor_offset_x);

    printf(
        "     sensor_offset_y = %.5f\n",
        cfg.sensor_offset_y);

    printf(
        "     screen_tilt     = %.2f\n",
        cfg.screen_tilt);

    return 1;
}

void save_config(
    const char* path)
{
    FILE* f = fopen(path,"w");

    if(!f)
        return;

    fprintf(f,"version %d\n",cfg.version);

    fprintf(
        f,
        "sensor_offset_x %.6f\n",
        cfg.sensor_offset_x);

    fprintf(
        f,
        "sensor_offset_y %.6f\n",
        cfg.sensor_offset_y);

    fprintf(
        f,
        "screen_distance %.3f\n",
        cfg.screen_distance);

    fprintf(
        f,
        "screen_tilt %.3f\n",
        cfg.screen_tilt);

    fprintf(
        f,
        "gaze_smooth %.3f\n",
        cfg.gaze_smooth);

    fprintf(
        f,
        "cursor_smooth %.3f\n",
        cfg.cursor_smooth);

    fprintf(
        f,
        "edge_zone %.3f\n",
        cfg.edge_zone);

    fprintf(
        f,
        "edge_power %.3f\n",
        cfg.edge_power);

    fprintf(f,"# calibration\n");

    for(int y=0;y<3;y++)
    {
        for(int x=0;x<3;x++)
        {
            fprintf(
                f,
                "%.6f %.6f %.6f %.6f\n",
                p[y][x].raw_x,
                p[y][x].raw_y,
                p[y][x].target_x,
                p[y][x].target_y);
        }
    }

    fclose(f);
}

void draw_target(
    Display* d,
    Window w,
    GC gc,
    Colormap cmap,
    int x,
    int y,
    int active)
{
    XColor red,green,white,gray;

    XParseColor(d,cmap,"#ff3030",&red);
    XAllocColor(d,cmap,&red);

    XParseColor(d,cmap,"#30ff30",&green);
    XAllocColor(d,cmap,&green);

    XParseColor(d,cmap,"#ffffff",&white);
    XAllocColor(d,cmap,&white);

    XParseColor(d,cmap,"#808080",&gray);
    XAllocColor(d,cmap,&gray);

    XClearWindow(d,w);

    XSetLineAttributes(
        d,
        gc,
        1,
        LineSolid,
        CapRound,
        JoinRound);

    XSetForeground(d,gc,gray.pixel);

    XDrawLine(
        d,w,gc,
        x - TARGET_CROSS,
        y,
        x + TARGET_CROSS,
        y);

    XDrawLine(
        d,w,gc,
        x,
        y - TARGET_CROSS,
        x,
        y + TARGET_CROSS);

    XSetForeground(
        d,
        gc,
        white.pixel);

    XFillArc(
        d,w,gc,
        x - TARGET_OUTER,
        y - TARGET_OUTER,
        TARGET_OUTER*2,
        TARGET_OUTER*2,
        0,
        360*64);

    XSetForeground(
        d,
        gc,
        BlackPixel(d,DefaultScreen(d)));

    XFillArc(
        d,w,gc,
        x - TARGET_MIDDLE,
        y - TARGET_MIDDLE,
        TARGET_MIDDLE*2,
        TARGET_MIDDLE*2,
        0,
        360*64);

    XSetForeground(
        d,
        gc,
        active ? green.pixel : red.pixel);

    XFillArc(
        d,w,gc,
        x - TARGET_INNER,
        y - TARGET_INNER,
        TARGET_INNER*2,
        TARGET_INNER*2,
        0,
        360*64);

    XFlush(d);
}

int main()
{
    import_tobii_db();

    Display* d = XOpenDisplay(NULL);

    if(!d)
    {
        printf("No X11 display\n");
        return 1;
    }

    int screen =
        DefaultScreen(d);

    int W =
        DisplayWidth(d,screen);

    int H =
        DisplayHeight(d,screen);

    Window root =
        RootWindow(d,screen);

    XSetWindowAttributes attr;

    attr.override_redirect = True;

    Window win =
        XCreateWindow(
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

    XMapRaised(d,win);

    XSetWindowBackground(
        d,
        win,
        BlackPixel(d,screen));

    XClearWindow(d,win);

    XGrabKeyboard(
        d,
        win,
        True,
        GrabModeAsync,
        GrabModeAsync,
        CurrentTime);

    XGrabPointer(
        d,
        win,
        True,
        ButtonPressMask,
        GrabModeAsync,
        GrabModeAsync,
        win,
        None,
        CurrentTime);

    XFlush(d);

    GC gc =
        XCreateGC(d,win,0,NULL);

    Colormap cmap =
        DefaultColormap(d,screen);

    float targets[9][2] =
    {
        {0.05f,0.05f},
        {0.50f,0.05f},
        {0.95f,0.05f},

        {0.05f,0.50f},
        {0.50f,0.50f},
        {0.95f,0.50f},

        {0.05f,0.95f},
        {0.50f,0.95f},
        {0.95f,0.95f}
    };

    tobii_api_t* api;
    tobii_device_t* dev;

    char url[256]={0};

    if(tobii_api_create(
        &api,
        NULL,
        NULL) != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "tobii_api_create failed\n");

        return 1;
    }

    tobii_enumerate_local_device_urls(
        api,
        url_cb,
        url);

    if(strlen(url)==0)
    {
        printf(
            "No Tobii device found\n");

        return 1;
    }

    printf(
        "Using device: %s\n",
        url);

    if(tobii_device_create(
        api,
        url,
        &dev) != TOBII_ERROR_NO_ERROR)
    {
        printf(
            "tobii_device_create failed\n");

        return 1;
    }

    tobii_gaze_point_subscribe(
        dev,
        gaze_cb,
        NULL);

    printf(
        "Calibration starting\n");

    printf(
        "Screen %dx%d smooth=%.3f cursor=%.3f edge=%.3f\n",
        W,
        H,
        cfg.gaze_smooth,
        cfg.cursor_smooth,
        cfg.edge_zone);

    sleep(1);

    for(int i=0;i<POINTS;i++)
    {
        p[i/3][i%3].target_x =
            targets[i][0];

        p[i/3][i%3].target_y =
            targets[i][1];

        int px =
            targets[i][0] * W;

        int py =
            targets[i][1] * H;

        draw_target(
            d,
            win,
            gc,
            cmap,
            px,
            py,
            0);

        printf(
            "Point %d/%d : %.2f %.2f\n",
            i+1,
            POINTS,
            targets[i][0],
            targets[i][1]);

        sleep(2);

        valid = 0;

        usleep(300000);

        float sx=0;
        float sy=0;

        int success=0;

        for(int retry=0;
            retry<MAX_RETRIES;
            retry++)
        {
            sx=0.0f;
            sy=0.0f;

            int count=0;
            int loops=0;

            while(count < SAMPLE_COUNT &&
                  loops < SAMPLE_COUNT*12)
            {
                valid = 0;

                tobii_error_t err;

                err =
                    tobii_wait_for_callbacks(
                        1,
                        &dev);

                if(err != TOBII_ERROR_NO_ERROR &&
                   err != TOBII_ERROR_TIMED_OUT)
                {
                    printf(
                        "\n[Tobii] wait failed: %d\n",
                        err);

                    break;
                }

                err =
                    tobii_device_process_callbacks(
                        dev);

                if(err != TOBII_ERROR_NO_ERROR)
                {
                    printf(
                        "\n[Tobii] process failed: %d\n",
                        err);

                    break;
                }

                while(XPending(d))
                {
                    XEvent e;

                    XNextEvent(d,&e);
                }

                if(valid)
                {
                    sx += gx;
                    sy += gy;

                    count++;

                    printf(
                        "\rSamples %3d/%3d gaze %.3f %.3f   ",
                        count,
                        SAMPLE_COUNT,
                        gx,
                        gy);

                    fflush(stdout);
                }
                else
                {
                    if((loops % 100) == 0)
                    {
                        printf(
                            "\rWaiting for valid gaze frames... %-6d",
                            loops);

                        fflush(stdout);
                    }
                }

                loops++;

                usleep(2000);
            }

            printf("\n");

            if(count >= MIN_VALID_SAMPLES)
            {
                p[i/3][i%3].raw_x =
                    sx / count;

                p[i/3][i%3].raw_y =
                    sy / count;

                success=1;

                printf(
                    "[OK] valid samples: %d\n",
                    count);

                break;
            }

            printf(
                "Retry %d/%d valid=%d\n",
                retry+1,
                MAX_RETRIES,
                count);

            sleep(1);
        }

        if(!success)
        {
            printf(
                "Calibration failed\n");

            return 1;
        }

        draw_target(
            d,
            win,
            gc,
            cmap,
            px,
            py,
            1);

        printf(
            "RAW %.6f %.6f -> TARGET %.2f %.2f ERR %.4f %.4f\n",
            p[i/3][i%3].raw_x,
            p[i/3][i%3].raw_y,
            p[i/3][i%3].target_x,
            p[i/3][i%3].target_y,
            p[i/3][i%3].raw_x -
            p[i/3][i%3].target_x,
            p[i/3][i%3].raw_y -
            p[i/3][i%3].target_y);

        usleep(350000);
    }

    char cfgpath[PATH_MAX];

    get_config_path(
        cfgpath,
        sizeof(cfgpath));

    save_config(cfgpath);

    printf(
        "Saved calibration: %s\n",
        cfgpath);

    tobii_gaze_point_unsubscribe(dev);

    tobii_device_destroy(dev);

    tobii_api_destroy(api);

    XDestroyWindow(d,win);

    XCloseDisplay(d);

    return 0;
}
