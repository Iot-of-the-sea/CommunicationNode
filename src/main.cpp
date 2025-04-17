#include "main.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "invalid input. " << argc << " arguments provided. needs 1." << endl;
        exit(0);
    }

    char *mode = argv[1];
    if (strcmp(mode, "rov") == 0)
        runFSM(true);
    else if (strcmp(mode, "sensor") == 0)
        runFSM(false);
    else
    {
        cerr << "invalid input. " << "mode must be \"rov\" or \"sensor\". given "
             << mode << "." << endl;
        exit(0);
    }
    return 0;
}