//gcc research_version_test.c -o research_version_test -ldl
#include <stdio.h>
#include <dlfcn.h>

typedef char* (*get_version_fn)();

int main()
{
    void* lib = dlopen(
        "/opt/TobiiProEyeTrackerManager/resources/stage/sdk/libraries/lib/libtobii_research.so",
        RTLD_LAZY );

    if( !lib )
    {
        printf( "dlopen failed\n" );
        return 1;
    }

    get_version_fn get_version =
        (get_version_fn)dlsym(
            lib,
            "tobii_research_get_sdk_version" );

    if( !get_version )
    {
        printf( "symbol missing\n" );
        return 1;
    }

    char* version = get_version();

    if( version )
        printf( "SDK Version: %s\n", version );
    else
        printf( "NULL version\n" );

    return 0;
}
