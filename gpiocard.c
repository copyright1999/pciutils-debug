#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>>


#define IO_CONFIG 0XF8
#define IO_BYTE_1 0XFC

int main(int argc,char *argv[])
{
    int i,fd;
    uint32_t offset, config[5];
    uint8_t *bar0,value;

    /*let us open the bar's resource file */
    //这里是作者的GPIO 卡
    fd = open("/sys/bus/pci/devices/0000:04:08.0/resource0",O_RDWR|O_SYNC);

    if (fd < 0)
    {
        perror("Error openning BAR's resource file");
        return -1;
    }

    bar0 = (uint8_t *) mmap(NULL,256,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    close (fd);

    /*let us check if we have a valid pointer */
    if (bar0 == MAP_FAILED)
    {
        perror("Error mapping of  BAR failed");
        return -1;
    }

    /*let us take care of the offset */
    fd = open("/sys/bus/pci/devices/0000:04:08.0/config",O_RDONLY);
    if (fd < 0)
    {
        perror("Error openning BAR's config file");
        return -1;
    }

    i = read(fd,config,0x14);

    close(fd);

    if (i != 0x14)
    {
        perror("Error reading PCI config header");
        munmap(bar0,256);
        return -1;
    }
    
    /*calculate the offset */
    offset = (config[4] & 0xfffffff0)%4096;

    /*adjust the bar's pointer*/
    bar0 = bar0 + offset;


    /*let us access the card*/
    *(bar0 + IO_CONFIG) = 0x0;/*sets all bytes to input*/


    /*let us read GPIO Byte 1 values*/
    value = *(bar0 + IO_BYTE_1);

    for ( i = 0; i < 4; i++)
    {
        printf("ID %d : %s \n",i,((value &(1 <<i))>0)?1:0);
    }
    
    /*let us check whether we have to write the outputs*/
    if (argc > 1)
    {
        value = (uint8_t)atoi(argv[1]);
        /*set byte 0 to output*/
        *(bar0 + IO_CONFIG) = 0x1;

        /*set the leds*/
        *(bar0 + IO_BYTE_1) = (~value)<< 5;
    }

    munmap(bar0,256);
    return 0;
   
}


/**/