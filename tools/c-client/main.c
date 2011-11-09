#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"

int main(int argc, char *argv[])
{
    int fd;
    char buf[256];
    size_t count;

    if (argc < 2)
    {
        fprintf(stderr, "Usage %s <serial device>\n", argv[0]);
        return 1;
    }

    fd = serial_open(argv[1]);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }

    if (0 != serial_setup(fd, B115200))
    {
        fprintf(stderr, "Failed to configure %s\n", argv[1]);
        return 1;
    }

    while((count = read(0, buf, 1)) > 0)
    {
        serial_write(fd, buf, count);
#if 1
//        if (*buf == '\r' || *buf == '\n')
//            usleep(100000);
//        else
            usleep(1000);
#endif
    }

    serial_close(fd);

    return 0;
}


