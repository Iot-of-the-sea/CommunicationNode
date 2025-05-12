#include "main.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        cerr << "invalid input. " << argc << " arguments provided. needs 1 or 2." << endl;
        exit(0);
    }

    char *mode = argv[1];

    const char *file = (argc == 3) ? argv[2] : "";

    if (strcmp(mode, "rov") == 0)
        runFSM(true, file);
    else if (strcmp(mode, "sensor") == 0)
        runFSM(false, file);
    else
    {
        cerr << "invalid input. " << "mode must be \"rov\" or \"sensor\". given "
             << mode << "." << endl;
        exit(0);
    }
    return 0;
}