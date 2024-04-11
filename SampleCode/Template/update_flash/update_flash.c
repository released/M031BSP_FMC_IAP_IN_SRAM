/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "misc_config.h"

#include "update_flash.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_UPDATE_FLASH_CTL;
#define FLAG_UPDATE_FLASH_REVERSE0	                 		(flag_UPDATE_FLASH_CTL.bit0)
#define FLAG_UPDATE_FLASH_REVERSE1                   		(flag_UPDATE_FLASH_CTL.bit1)
#define FLAG_UPDATE_FLASH_REVERSE2                  		(flag_UPDATE_FLASH_CTL.bit2)
#define FLAG_UPDATE_FLASH_REVERSE3                			(flag_UPDATE_FLASH_CTL.bit3)
#define FLAG_UPDATE_FLASH_REVERSE4                 			(flag_UPDATE_FLASH_CTL.bit4)
#define FLAG_UPDATE_FLASH_REVERSE5                			(flag_UPDATE_FLASH_CTL.bit5)
#define FLAG_UPDATE_FLASH_REVERSE6                  		(flag_UPDATE_FLASH_CTL.bit6)
#define FLAG_UPDATE_FLASH_REVERSE7                  		(flag_UPDATE_FLASH_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/


uint32_t size_addr = APP2_ADDR;

#define VECMAP_SRAM_BASE                    				0x20000000UL
extern void *__Vectors;                         /* see startup file */
#define VECTPR_SRAM_LEN                                    (0x200) // 0x200
uint8_t u8Vectors_SRAM[VECTPR_SRAM_LEN] __attribute__((section("VectorSection")));

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
extern int IsDebugFifoEmpty(void);
extern volatile uint32_t u32FMCChecksum;
extern uint8_t verify_application_chksum(uint32_t target_addr , uint32_t target_size , uint32_t checksum_addr);

int32_t update_ap_OnSRAM(void)
{
	int32_t resp_e = 0;
	int32_t resp_w = 0;
    uint32_t u32Addr = 0;
    uint32_t u32Data = 0;
    uint32_t u32Tmp = 0;
    uint32_t u32Cnt = 0;

    /*
        .erase AP1 page
        .copy AP2 data , write to AP1 

    */
    // SystemCoreClock_OnSRAM = clk_src;
	// SYS_UnlockReg_OnSRAM();
    // FMC_Open_OnSRAM();
	// FMC_ENABLE_AP_UPDATE_OnSRAM();
    
    printf("START update finish\r\n");    
    printf(".");

    /* Mask all interrupt before changing VECMAP to avoid wrong interrupt handler fetched */    
    __set_PRIMASK(1);

	SYS_UnlockReg();
    FMC_Open();
	FMC_ENABLE_AP_UPDATE();

    u32Addr = APP1_ADDR;

    #if 0   // erase all
    for(u32Cnt = 0; u32Cnt < APP1_SIZE ; u32Cnt += FMC_FLASH_PAGE_SIZE)
    {
        resp_e = FMC_Erase(u32Addr + u32Cnt);

        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Erase failed!\r\n");
            return -1;
        }

        #if 1
        printf("[OnSRAM]addr:0x%08X erase done(%d)\r\n" ,u32Addr + u32Cnt,resp_e);
        #endif  
    }
    #endif

    for(u32Cnt = 0; u32Cnt < APP1_SIZE ; u32Cnt += 4)
    {        

        #if 1   // erase page
        if( ((u32Addr + u32Cnt)%FMC_FLASH_PAGE_SIZE) == 0)
        {
            #if 0
            printf("[OnSRAM-erase start]addr:0x%08X\r\n" ,u32Addr + u32Cnt);
            #endif 

            resp_e = FMC_Erase(u32Addr + u32Cnt);

            if (g_FMC_i32ErrCode != 0)
            {
                printf("FMC_Erase failed!\r\n");
                return -1;
            }

            #if 0
            printf("[OnSRAM-erase done]addr:0x%08X (%d)\r\n" ,u32Addr + u32Cnt,resp_e);
            #endif            
        }
        #else
        if((u32Addr & (FMC_FLASH_PAGE_SIZE - 1)) == 0)
        {
            // resp_e = FMC_Erase(u32Addr);

            #if 1
            printf("[OnSRAM]addr:0x%08X erase done(%d)\r\n" ,u32Addr,resp_e);
            #endif
        }
        #endif

        u32Data = FMC_Read(APP2_ADDR + u32Cnt);
        #if 0   //debug
        printf("[OnSRAM-r1]addr:0x%08X ,r:0x%08X\r\n",u32Addr,u32Data);
        #endif

	    resp_w = FMC_Write(u32Addr + u32Cnt,u32Data);
        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Write failed!\r\n");
            return -1;
        }
        #if 0   //debug
        printf("[OnSRAM-w]addr:0x%08X ,w:0x%08X ,status:0x%02X\r\n",u32Addr,u32Data,resp_w);
        #endif

        // read back compare        
        u32Tmp = FMC_Read(u32Addr + u32Cnt);
        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Read failed!\r\n");
            return -1;
        }
        
        if (u32Data != u32Tmp)
        {
            printf("[Read/Write FAIL]\r\n");
            return -1;
        }

        #if 1   //debug
        if (u32Tmp != u32Data)
        {
            printf("[OnSRAM-r2]addr:0x%08X ,w:0x%08X ,r:0x%08X,status:0x%02X ,%s\r\n",u32Addr + u32Cnt ,u32Data ,u32Tmp ,resp_w  ,(u32Tmp == u32Data) ? ("OK") : ("NG") );        
        }
        

        // printf("[OnSRAM-r2]addr:0x%08X ,w:0x%08X ,r:0x%08X,status:0x%02X ,%s\r\n",u32Addr + u32Cnt ,u32Data ,u32Tmp ,resp_w  ,(u32Tmp == u32Data) ? ("OK") : ("NG") );
        #endif
        
    }

    printf("\r\n");

    printf("update finish , reset chip\r\n");
    UART_WAIT_TX_EMPTY(UART0);

    FMC_DISABLE_AP_UPDATE();
    FMC_Close();
    SYS_LockReg();

    __set_PRIMASK(0);

	SYS_UnlockReg();
    FMC_Open();
    SYS_ResetChip();

    return resp_w;
}


void Remap_ISR_OnSRAM(void)
{
    if (FMC_GetVECMAP() != VECMAP_SRAM_BASE)
    {
        printf("Before setting VECMAP=0x%x\n\r", FMC_GetVECMAP());
        UART_WAIT_TX_EMPTY(UART0);

        /* Mask all interrupt before changing VECMAP to avoid wrong interrupt handler fetched */
        __set_PRIMASK(1);

        memcpy(u8Vectors_SRAM, &__Vectors, VECTPR_SRAM_LEN);

        /* Unlock protected register */
        SYS_UnlockReg();

        /* Enable FMC ISP function */
        FMC_Open();

        /* set VECMAP to SRAM */
        FMC_SetVectorPageAddr((uint32_t)&u8Vectors_SRAM);


        /* Disable FMC ISP function */
        FMC_Close();

        /* Lock protected Register */
        SYS_LockReg();

        __set_PRIMASK(0);

        printf("After setting VECMAP=0x%x\n\r", FMC_GetVECMAP());
    }
}



unsigned char update_ap_check(void)
{
	uint8_t resp = 0;
    uint32_t addr_start = APP2_ADDR; 
    uint32_t size = APP2_SIZE;  //0x1C000;
	uint32_t ap2_checksum_value = 0;
	uint32_t ap2_checksum_addr = 0;

    SYS_UnlockReg();
    FMC_Open();
    
	ap2_checksum_addr = APP1_SIZE + APP2_SIZE - 4;
	resp = verify_application_chksum(addr_start , size - 4 , ap2_checksum_addr );

	/*
		. check AP2 checksum correction
			- if correnct , compare to AP1
				- if checksum not same as AP1 , update
				- if checksum same as AP1 , continue to main code
			- if not correct , means AP2 data corruption , continue to main code
	*/

	if (resp)
	{
		ap2_checksum_value = FMC_Read(ap2_checksum_addr);

    	printf("%s:area 1:0x%08X,area 2::0x%08X\r\n" , __FUNCTION__ , u32FMCChecksum,ap2_checksum_value);

		if (ap2_checksum_value != u32FMCChecksum)
		{			
			Remap_ISR_OnSRAM();
			update_ap_OnSRAM();
		}
		else
		{
			printf("area 2 checksum same as area 1\r\n");
		}
	}

    FMC_Close();
    SYS_LockReg();

    // printf("%s = 0x%08X\r\n" , __FUNCTION__ , resp);
	
    #if defined (APP1)
	printf("RUN APP_#1\r\n");
    #elif defined (APP2)
	printf("RUN APP_#2\r\n");
    #endif

	return resp;

}
