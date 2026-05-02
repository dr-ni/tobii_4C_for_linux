//gcc research_gaze_test.c -o research_gaze_test -ldl
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

typedef void* TobiiResearchEyeTracker;

typedef struct
{
    double gaze_point_on_display_normalized[2];
    int validity;
} TobiiResearchGazeData;

/* Function pointer types */
typedef int (*find_all_fn)(TobiiResearchEyeTracker** trackers, int* count);
typedef void (*free_trackers_fn)(TobiiResearchEyeTracker* trackers);
typedef int (*subscribe_gaze_fn)(
    TobiiResearchEyeTracker tracker,
    void (*callback)(TobiiResearchGazeData*, void*),
    void* user_data);
typedef int (*unsubscribe_gaze_fn)(TobiiResearchEyeTracker tracker);

/* Callback */
void gaze_callback(
    TobiiResearchGazeData* gaze,
    void* user_data)
{
    (void)user_data;

    if (!gaze)
        return;

    printf(
        "Gaze: %.3f %.3f\n",
        gaze->gaze_point_on_display_normalized[0],
        gaze->gaze_point_on_display_normalized[1]);
}

int main()
{
    void* lib = dlopen(
        "/opt/TobiiProEyeTrackerManager/resources/stage/sdk/libraries/lib/libtobii_research.so",
        RTLD_LAZY);

    if (!lib)
    {
        printf("dlopen failed\n");
        return 1;
    }

    find_all_fn find_all =
        (find_all_fn)dlsym(lib, "tobii_research_find_all_eyetrackers");

    free_trackers_fn free_trackers =
        (free_trackers_fn)dlsym(lib, "tobii_research_free_eyetrackers");

    subscribe_gaze_fn subscribe =
        (subscribe_gaze_fn)dlsym(lib, "tobii_research_subscribe_to_gaze_data");

    unsubscribe_gaze_fn unsubscribe =
        (unsubscribe_gaze_fn)dlsym(lib, "tobii_research_unsubscribe_from_gaze_data");

    if (!find_all || !subscribe)
    {
        printf("Missing symbols\n");
        return 1;
    }

    TobiiResearchEyeTracker* trackers = NULL;
    int count = 0;

    if (find_all(&trackers, &count) != 0 || count == 0)
    {
        printf("No trackers found\n");
        return 1;
    }

    printf("Found %d tracker(s)\n", count);

    TobiiResearchEyeTracker tracker = trackers[0];

    if (subscribe(tracker, gaze_callback, NULL) != 0)
    {
        printf("Subscribe failed\n");
        return 1;
    }

    printf("Reading gaze data...\n");

    while (1)
    {
        /* busy loop – research API pushes callbacks */
        usleep(10000);
    }

    unsubscribe(tracker);

    free_trackers(trackers);

    dlclose(lib);

    return 0;
}
