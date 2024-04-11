/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "NuMicro.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
#define UPDATE_AP_PACKET_INFO_SIZE                 (6)
#define UPDATE_AP_PACKET_DATA_SIZE                 (32)
#define UPDATE_AP_PACKET_TOTAL_SIZE                (UPDATE_AP_PACKET_INFO_SIZE+UPDATE_AP_PACKET_DATA_SIZE)


#define APP1_ADDR                                  (FMC_APROM_BASE)
#define APP1_SIZE                                  (60*1024)   
#define APP2_ADDR                                  (60*1024)   // 0xF000
#define APP2_SIZE                                  (60*1024)   

// #define APP1_ADDR                                  (30*1024)   // 0x7800
// #define APP1_SIZE                                  (30*1024)   
// #define APP2_ADDR                                  (60*1024)   // 0xF000
// #define APP2_SIZE                                  (30*1024)   

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned char update_ap_check(void);
