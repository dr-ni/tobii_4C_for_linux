#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cstring>
#include <ctime>
#include <X11/Xlib.h>
//#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

Display *dpy = NULL;
int *xcal;
int *ycal;
int width=1280;
int height=720;

void gaze_point_callback(tobii_gaze_point_t const *gaze_point, void *user_data) {
    int x,y;
    if (gaze_point->validity == TOBII_VALIDITY_VALID)
        printf("Gaze point: %f, %f\n", gaze_point->position_xy[0], gaze_point->position_xy[1]);
        x=int((width * gaze_point->position_xy[0]));
        if (x<0){
            if (x<*xcal){
                *xcal==x;
                printf(" cal: %d", *xcal);
            }
        }
        if (y<0 && y<*ycal) *ycal==y;
        y=int((height * gaze_point->position_xy[1]));
        printf("Point: %d, %d : %d, %d\n", x-*xcal, y-*ycal, *xcal, *ycal);
        //XTestFakeMotionEvent (dpy, 0, int(gaze_point->position_xy[0]*1366), int(gaze_point->position_xy[1]*768), CurrentTime);
}

static void url_receiver(char const *url, void *user_data) {
    char *buffer = (char *) user_data;
    if (*buffer != '\0') return; // only keep first value

    if (strlen(url) < 256)
        strcpy(buffer, url);
}

int main() {
    *xcal=0;
    *ycal=0;
    tobii_api_t *api;
    tobii_error_t error = tobii_api_create(&api, NULL, NULL);
    assert(error == TOBII_ERROR_NO_ERROR);
    XEvent event;
    setenv("DISPLAY",":0",1);
  dpy = XOpenDisplay (NULL);
  /* Get the current pointer position */
  XQueryPointer (dpy, RootWindow (dpy, 0), &event.xbutton.root,
   &event.xbutton.window, &event.xbutton.x_root,
   &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
   &event.xbutton.state);

    char url[256] = {0};
    error = tobii_enumerate_local_device_urls(api, url_receiver, url);
    assert(error == TOBII_ERROR_NO_ERROR && *url != '\0');

    tobii_device_t *device;
    error = tobii_device_create(api, url, &device);
    assert(error == TOBII_ERROR_NO_ERROR);

    error = tobii_gaze_point_subscribe(device, gaze_point_callback, 0);
    assert(error == TOBII_ERROR_NO_ERROR);

    while (1 > 0) {
        error = tobii_wait_for_callbacks(1, &device);
        assert(error == TOBII_ERROR_NO_ERROR || error == TOBII_ERROR_TIMED_OUT);

        error = tobii_device_process_callbacks(device);
        assert(error == TOBII_ERROR_NO_ERROR);
    }

    error = tobii_gaze_point_unsubscribe(device);
    assert(error == TOBII_ERROR_NO_ERROR);

    error = tobii_device_destroy(device);
    assert(error == TOBII_ERROR_NO_ERROR);

    error = tobii_api_destroy(api);
    assert(error == TOBII_ERROR_NO_ERROR);
    return 0;
}
