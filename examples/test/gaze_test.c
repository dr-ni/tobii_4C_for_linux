//gcc gaze_test.c -o gaze_test -I/usr/include -L/usr/lib/tobii -Wl,-rpath,/usr/lib/tobii -ltobii_stream_engine

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

static void url_receiver( char const* url, void* user_data )
{
    char* buffer = (char*) user_data;

    strncpy( buffer, url, 255 );
    buffer[255] = '\0';
}

static void gaze_point_callback(
    tobii_gaze_point_t const* gaze_point,
    void* user_data )
{
    (void) user_data;

    if( gaze_point->validity != TOBII_VALIDITY_VALID )
        return;

    printf(
        "Gaze: %.3f %.3f\n",
        gaze_point->position_xy[0],
        gaze_point->position_xy[1] );

    fflush( stdout );
}

int main()
{
    tobii_api_t* api = NULL;
    tobii_device_t* device = NULL;
    tobii_error_t error;

    char url[256] = {0};

    printf( "Creating API...\n" );

    error = tobii_api_create( &api, NULL, NULL );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "API create failed: %d\n", error );
        return 1;
    }

    printf( "Enumerating devices...\n" );

    error = tobii_enumerate_local_device_urls(
        api,
        url_receiver,
        url );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "Enumeration failed: %d\n", error );
        tobii_api_destroy( api );
        return 1;
    }

    if( strlen( url ) == 0 )
    {
        printf( "No device found\n" );
        tobii_api_destroy( api );
        return 1;
    }

    printf( "Device URL: %s\n", url );

    printf( "Creating device...\n" );

    error = tobii_device_create(
        api,
        url,
        &device );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "Device create failed: %d\n", error );
        tobii_api_destroy( api );
        return 1;
    }

    printf( "Subscribing to gaze point stream...\n" );

    error = tobii_gaze_point_subscribe(
        device,
        gaze_point_callback,
        NULL );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "Subscribe failed: %d\n", error );

        tobii_device_destroy( device );
        tobii_api_destroy( api );

        return 1;
    }

    printf( "Reading gaze data...\n" );
    printf( "Move your eyes across the screen.\n" );
    printf( "Press Ctrl+C to stop.\n\n" );

    while( 1 )
    {
        error = tobii_wait_for_callbacks(
            1,
            &device );

        if( error != TOBII_ERROR_NO_ERROR &&
            error != TOBII_ERROR_TIMED_OUT )
        {
            printf(
                "Wait failed: %d\n",
                error );

            break;
        }

        error = tobii_device_process_callbacks(
            device );

        if( error != TOBII_ERROR_NO_ERROR &&
            error != TOBII_ERROR_TIMED_OUT )
        {
            printf(
                "Process callbacks failed: %d\n",
                error );

            break;
        }
    }

    printf( "\nCleaning up...\n" );

    tobii_gaze_point_unsubscribe( device );
    tobii_device_destroy( device );
    tobii_api_destroy( api );

    return 0;
}
