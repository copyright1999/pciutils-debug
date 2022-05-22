#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/pci.h"

/** struct to represent a bitfield in the PCI configuration space**/
struct config_space_bitfield
{
    char name[64];
    unsigned int offset;
    unsigned int size;
};

/** Prototypes**/
void print_pci_header(struct pci_dev *pdev);
struct pci_dev * search_device(struct pci_access *pacc,u8 bus,u8 slot,u8 func);
void int_2_hexstr(u32 value,unsigned int size,char * destination);
int convert_hexstring(char *hexstring);

/** PCI Type 0 Header for Endpoints**/
struct config_space_bitfield type_0_header[] = {
    {"Vendor ID",                       0X0,       2},
    {"Device ID",                       0X2,       2},
    {"Command",                         0X4,       2},
    {"Staus",                           0X6,       2},
    {"Revision ID",                     0X8,       1},
    {"Class Code",                      0X9,       3},
    {"Cache Line S.",                   0XC,       1},
    {"Lat. Timer",                      0XD,       1},
    {"Header Type",                     0XE,       1},
    {"BIST",                            0XF,       1},
    {"BAR 0",                           0X10,      4},
    {"BAR 1",                           0X14,      4},
    {"BAR 2",                           0X18,      4},
    {"BAR 3",                           0X1C,      4},
    {"BAR 4",                           0X20,      4},
    {"BAR 5",                           0X24,      4},
    {"Cardbus CIS Pointer",             0X28,      4},
    {"Subsystem Vendor ID",             0X2C,      2},
    {"Subsystem ID",                    0X2E,      2},
    {"Expansion ROM Base Address",      0X30,      4},
    {"Cap.Pointer",                     0X34,      1},
    {"Reserved",                        0X35,      3},
    {"Reserved",                        0X38,      4},
    {"Interrupt Line",                  0X3C,      1},
    {"Interrupt Pin",                   0X3D,      1},
    {"Min Gnt.",                        0X3E,      1},
    {"Max Lat.",                        0X3F,      1},
    {"End     ",                        0X40,      5},
};

/** PCI Type 0 Header for Endpoints**/
struct config_space_bitfield type_1_header[] = {
    {"Vendor ID",                           0X0,       2},
    {"Device ID",                           0X2,       2},
    {"Command",                             0X4,       2},
    {"Staus",                               0X6,       2},
    {"Revision ID",                         0X8,       1},
    {"Class Code",                          0X9,       3},
    {"Cache Line S.",                       0XC,       1},
    {"Lat. Timer",                          0XD,       1},
    {"Header Type",                         0XE,       1},
    {"BIST",                                0XF,       1},
    {"BAR 0",                               0X10,      4},
    {"BAR 1",                               0X14,      4},
    {"Pirmary Bus Number",                  0X18,      1},
    {"Secondary Bus Number",                0X19,      1},
    {"Subordinate Bus Number",              0X1A,      1},
    {"Secondary Latency Timer",             0X1B,      1},
    {"I/O Base",                            0X1C,      1},
    {"I/O Limit",                           0X1D,      1},
    {"Secondary Status",                    0X1E,      2},
    {"Memory Base",                         0X20,      2},
    {"Memory Limit",                        0X22,      2},
    {"Prefetchable Memory Base",            0X24,      2},
    {"Prefetchable Memory Limit",           0X26,      2},
    {"Prefetchable Base Upper 32 Bits",     0X28,      4},
    {"Prefetchable Limit Upper 32 Bits",    0X2C,      4},
    {"I/O Base Upper 16 Bits",              0X30,      2},
    {"I/O Limit Upper 16 Bits",             0X32,      2},
    {"Cap.Pointer",                         0X34,      1},
    {"Reserved",                            0X35,      3},
    {"Expansion ROM Base Address",          0X38,      4},
    {"Interrupt Line",                      0X3C,      1},
    {"Interrupt Pin",                       0X3D,      1},
    {"Bridge Control",                      0X3E,      2},
    {"End     ",                            0X40,      5},
};



/** Variable contain two possible header types**/
struct config_space_bitfield *types[2] = {
    &type_0_header[0],
    &type_1_header[0]
};



/** 
 * @brief prints the PCI config header of a given device
 * 
 * @param device to print its header
 * 
 */
void print_pci_header(struct pci_dev *pdev)
{
    u8 header_type = 0;
    u32 value,bf_value;
    u64 mask;
    unsigned int i, space_available, padding ,bitfield =0 ,bf2;
    int j;
    struct config_space_bitfield *ptr;
    char str_value[16];
    const char *ctypes[] = {"n Endpoint" ,"Bridge"};

    /** Check if device is valid**/
    if (pdev == NULL)
    {
        return;
    }
    

    /** Check if device is bridge or EP**/
    header_type = pci_read_byte(pdev,PCI_HEADER_TYPE) & 0X1;//不懂这里是干啥
    ptr = types[header_type];
    printf("Selected device %x:%x:%x is a%s\n",pdev->bus,pdev->dev,pdev->func,ctypes[header_type]);

    /** Read config space and dump it to console**/
    printf("|  Byte 0       |  Byte 1    |  Byte 2   |  Byte 3  |\t\t|  Byte 0       |  Byte 1    |  Byte 2   |  Byte 3  |\n");
    printf("| ------------------------------------------------- |\t\t| ------------------------------------------------- |\t Address\n");

    for ( i = 0; i < 0x40; i +=4)
    {
        bf2 = bitfield;
        /** Print defintion of PCI header line**/
        putchar('|');

        while (ptr[bitfield].offset < i+4)
        {
            space_available = 14 * ptr[bitfield].size + (ptr[bitfield].size -1);
            padding = (space_available - strlen(ptr[bitfield].name))/2;

            for ( j = 0; j < (int)padding; j++)
            {
                putchar(' ');
            }
            printf("%s",ptr[bitfield].name);
            
            for ( j = (int)padding + strlen(ptr[bitfield].name); j < (int)space_available; j++)
            {
                putchar(' ');
            }
            putchar('|');
            bitfield ++ ;
        }



        /** Read Value**/
        value = pci_read_long(pdev,i);

        /** Print Vaules of PCI header line**/
        bitfield = bf2;
        printf("\t\t|");

        while (ptr[bitfield].offset < i+4)
        {
            if (ptr[bitfield].size == 5)
            {
                break;
            }
            
            /** Extracting Bitfield of interest**/
            mask = ( (1L << (ptr[bitfield].size * 8)) -1) << (8* (ptr[bitfield].offset - i));
            bf_value = (value & mask) >> (8* (ptr[bitfield].offset - i));

            /** Print Bitfield and table**/
            space_available = 14 * ptr[bitfield].size + (ptr[bitfield].size -1);
            padding = (space_available - ( 2 + ptr[bitfield].size))/2;

            for ( j = 0; j < (int)padding; j++)
            {
                putchar(' ');
            }
            int_2_hexstr(bf_value,ptr[bitfield].size,str_value);
            printf("%s",str_value);


            
            for ( j = (int)padding + strlen(str_value); j < (int)space_available; j++)
            {
                putchar(' ');
            }
            putchar('|');
            bitfield ++ ;
        }
        printf("\t0x%02x",i);
        printf("\n| ------------------------------------------------- |\t\t| ------------------------------------------------- |\n");
    }
    

}

/** 
 * @brief searches for a device with a given Bus- ,Slot- and Functionnumber
 * 
 * @param pacc: PCI Acess
 * @param bus: Busnumber
 * @param slot: Slotnumber
 * @param func: Fuctionnumber
 * 
 * @return NULL: No device found
 *         pointer to device
 */
struct pci_dev * search_device(struct pci_access *pacc,u8 bus,u8 slot,u8 func)
{
    struct pci_dev * dev;
    for (dev  = pacc->devices; dev !=NULL ; dev = dev ->next)
    {
        if ((dev->bus == bus) && (dev->dev == slot) && (dev->func == func) )
        {
            return dev;
        }
    }
    return NULL;
}

/** 
 * @brief converts an integer to a hexadecimal string
 * 
 * @param value:int value to convert
 * @param size:length of value in bytes
 * @param destination:destination string
 * 
 */
void int_2_hexstr(u32 value,unsigned int size,char * destination)
{
    const char letters[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
    unsigned int i;

    /** Init string**/
    strcpy(destination,"0x");
    for ( i = 0; i < size; i++)
    {
        strcat(destination,"00");
    }
    
    i = 2 + 2*size - 1;
}

/** 
 * @brief converts a hexadecimal string to an integer
 * 
 * @param hexstring: String to convert
 * 
 * @return converted hexstring as integer
 */
int convert_hexstring(char *hexstring)
{
    int number;
    if (strstr (hexstring,"0x") == NULL)
    {
        number = (int) strtol(hexstring,NULL,16);
    }
    else
    {
        number = (int) strtol(hexstring,NULL,0);
    }
    return number;
}


/** **/
/** **/
/** **/

/** **/


int main(int argc ,char * argv[])
{
    struct pci_access *pacc;
    struct pci_dev *dev;
    u8 bus, slot, func;

    /**Check arguments **/
    if (argc !=4)
    {
        printf("Three Arguments must be passed!\n");
        printf("Usage: %s [bus] [device] [function]\n",argv[0]);
        printf("With:\n");
        printf("\tbus:\tBusnumber of device to print PCI Header\n");
        printf("\tdevice:\tDevicenumber of device to print PCI Header\n");
        printf("\tfunc:\tFunctionnumber of device to print PCI Header\n");
        return -1;
    }

    pacc = pci_alloc();/**Get the pci_access structure **/

    /** Set all options you want -- here we stick with the defaults **/
    pci_init(pacc); /** Initialize the PCI Library**/
    pci_scan_bus(pacc); /** We want to get the list of devices**/
    
    
    /** Get numbers**/
    bus = convert_hexstring(argv[1]);
    slot = convert_hexstring(argv[2]);
    func = convert_hexstring(argv[3]);

    /** Check if device with the passed numbers exist**/
    dev = search_device(pacc,bus,slot,func);

    if (dev == NULL)
    {
        printf("No device found with %x:%x:%x\n",bus,slot,func);
        return -1;
    }

    /** If device exists ,print header**/
    print_pci_header(dev);

    pci_cleanup(pacc);/**Close everything **/
    return 0;
    
}
/** **/

/** **/