#include <epicsExport.h>
#include <epicsExit.h>
#include <iocsh.h>
#include <epicsThread.h>

int main(int argc, char *argv[])
{
    if(argc >= 2) {
        iocsh(argv[1]);
        epicsThreadSleep(.2);
    }
    iocsh(NULL);
    epicsExit(0);
    return 0;
} 