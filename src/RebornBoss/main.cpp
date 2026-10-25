#include "App.h"
#include <cstdlib>

int main()
{
    App app;
    if (app.Init() < 0)
        return EXIT_FAILURE;
    return app.Run() < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
