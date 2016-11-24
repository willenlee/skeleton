#include <stdio.h>
#include <openbmc_intf.h>
#include <gpio.h>
#include <openbmc.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <systemd/sd-bus.h>

typedef struct{
    int gpioNumA32Present;
    int gpioNumB9Present;
    int gpioNumPresent;
    int gpioNumLED;
}CableLedStruct;

#define CABLE_SW1 0
#define CABLE_SW2 1
#define CABLE_SW3 2
#define CABLE_SW4 3
#define PORT1A    0
#define PORT1B    1
#define PORT2A    2
#define PORT2B    3
#define MAX_CABLE_SW 4
#define MAX_CABLE_PORT 4

CableLedStruct CableLed[4][4];

/* ------------------------------------------------------------------------- */
int read_cable_status(int cable_sw, int cable_port)
{
    int rc = -1;
    char cable_prsent_path1[128] = {0};
    char cable_prsent_path2[128] = {0};
    int len = 0;
    int A32Present = 0;
    int B9Present = 0;

    //Read A32 Present pin
    len = snprintf(cable_prsent_path1, sizeof(cable_prsent_path1),
			"/sys/class/gpio/gpio%d/value", CableLed[cable_sw][cable_port].gpioNumA32Present);

    FILE *fp1 = fopen(cable_prsent_path1,"rb");
    if(fp1 == NULL)
    {
        return rc;
    }

    rc = fread(&A32Present, sizeof(int), 1, fp1);
    if (rc != 1)
        printf("Error: cable status return length invalid line:%d\n", __LINE__);

    fclose(fp1);

    //Read B9 Present pin
    len = snprintf(cable_prsent_path2, sizeof(cable_prsent_path2),
			"/sys/class/gpio/gpio%d/value", CableLed[cable_sw][cable_port].gpioNumB9Present);    

    FILE *fp2 = fopen(cable_prsent_path2,"rb");
    if(fp2 == NULL)
    {
        return rc;
    }

    rc = fread(&B9Present, sizeof(int), 1, fp2);
    if (rc != 1)
        printf("Error: cable status return length invalid line:%d\n", __LINE__);

    fclose(fp2);

    return (A32Present | B9Present);
}

void set_cable_led(int cable_sw, int cable_port, const char *control)
{
    char cable_led_path[128] = {0};
    int len = 0;

    len = snprintf(cable_led_path, sizeof(cable_led_path),
			"/sys/class/gpio/gpio%d/value", CableLed[cable_sw][cable_port].gpioNumLED);

    FILE *fp = fopen(cable_led_path,"w");
    if(fp == NULL)
	{
		return;
	}

    fwrite(control, strlen(control), 1, fp);
    
    fclose(fp);

    return;
}

void set_present(int cable_sw, int cable_port, const char *control)
{
    char cable_led_path[128] = {0};
    int len = 0;

    len = snprintf(cable_led_path, sizeof(cable_led_path),
			"/sys/class/gpio/gpio%d/value", CableLed[cable_sw][cable_port].gpioNumPresent);

    FILE *fp = fopen(cable_led_path,"w");
    if(fp == NULL)
	{
		return;
	}

    fwrite(control, strlen(control), 1, fp);
    
    fclose(fp);

    return;
}

void check_cable_status()
{
    int i = 0, j = 0;
    int cable_failed = 0;
    
    while(1)
    {
        for(i=0;i<MAX_CABLE_SW;i++)
        {
            for(j=0;j<MAX_CABLE_PORT;j++)
            {
                cable_failed = read_cable_status(i, j);
                if(cable_failed)
                {
                    set_cable_led(i, j, "1"); //set on cable led
                    set_present(i, j, "1");
                }
                else
                {
                    set_cable_led(i, j, "0"); //set off cable led
                    set_present(i, j, "0");
                }
            }
        }
        sleep(1);
    }
}

void init_cable_gpio_mapping()
{
    CableLed[CABLE_SW1][PORT1A].gpioNumA32Present = 256;
    CableLed[CABLE_SW1][PORT1A].gpioNumB9Present = 257;
    CableLed[CABLE_SW1][PORT1B].gpioNumA32Present = 258;
    CableLed[CABLE_SW1][PORT1B].gpioNumB9Present = 259;
    CableLed[CABLE_SW1][PORT1A].gpioNumPresent = 260;
    CableLed[CABLE_SW1][PORT1B].gpioNumPresent = 261;
    CableLed[CABLE_SW1][PORT1A].gpioNumLED = 262;
    CableLed[CABLE_SW1][PORT1B].gpioNumLED = 263;

    CableLed[CABLE_SW1][PORT2A].gpioNumA32Present = 264;
    CableLed[CABLE_SW1][PORT2A].gpioNumB9Present = 265;
    CableLed[CABLE_SW1][PORT2B].gpioNumA32Present = 266;
    CableLed[CABLE_SW1][PORT2B].gpioNumB9Present = 267;    
    CableLed[CABLE_SW1][PORT2A].gpioNumPresent = 268;
    CableLed[CABLE_SW1][PORT2B].gpioNumPresent = 269;
    CableLed[CABLE_SW1][PORT2A].gpioNumLED = 270;
    CableLed[CABLE_SW1][PORT2B].gpioNumLED = 271;

    CableLed[CABLE_SW2][PORT1A].gpioNumA32Present = 272;
    CableLed[CABLE_SW2][PORT1A].gpioNumB9Present = 273;
    CableLed[CABLE_SW2][PORT1B].gpioNumA32Present = 274;
    CableLed[CABLE_SW2][PORT1B].gpioNumB9Present = 275;
    CableLed[CABLE_SW2][PORT1A].gpioNumPresent = 276;
    CableLed[CABLE_SW2][PORT1B].gpioNumPresent = 277;
    CableLed[CABLE_SW2][PORT1A].gpioNumLED = 278;
    CableLed[CABLE_SW2][PORT1B].gpioNumLED = 279;

    CableLed[CABLE_SW2][PORT2A].gpioNumA32Present = 280;
    CableLed[CABLE_SW2][PORT2A].gpioNumB9Present = 281;
    CableLed[CABLE_SW2][PORT2B].gpioNumA32Present = 282;
    CableLed[CABLE_SW2][PORT2B].gpioNumB9Present = 283;
    CableLed[CABLE_SW2][PORT2A].gpioNumPresent = 284;
    CableLed[CABLE_SW2][PORT2B].gpioNumPresent = 285;
    CableLed[CABLE_SW2][PORT2A].gpioNumLED = 286;
    CableLed[CABLE_SW2][PORT2B].gpioNumLED = 287;

    CableLed[CABLE_SW3][PORT1A].gpioNumA32Present = 288;
    CableLed[CABLE_SW3][PORT1A].gpioNumB9Present = 289;
    CableLed[CABLE_SW3][PORT1B].gpioNumA32Present = 290;
    CableLed[CABLE_SW3][PORT1B].gpioNumB9Present = 291;
    CableLed[CABLE_SW3][PORT1A].gpioNumPresent = 292;
    CableLed[CABLE_SW3][PORT1B].gpioNumPresent = 293;
    CableLed[CABLE_SW3][PORT1A].gpioNumLED = 294;
    CableLed[CABLE_SW3][PORT1B].gpioNumLED = 295;

    CableLed[CABLE_SW3][PORT2A].gpioNumA32Present = 296;
    CableLed[CABLE_SW3][PORT2A].gpioNumB9Present = 297;
    CableLed[CABLE_SW3][PORT2B].gpioNumA32Present = 298;
    CableLed[CABLE_SW3][PORT2B].gpioNumB9Present = 299;
    CableLed[CABLE_SW3][PORT2A].gpioNumPresent = 300;
    CableLed[CABLE_SW3][PORT2B].gpioNumPresent = 301;
    CableLed[CABLE_SW3][PORT2A].gpioNumLED = 302;
    CableLed[CABLE_SW3][PORT2B].gpioNumLED = 303;

    CableLed[CABLE_SW4][PORT1A].gpioNumA32Present = 304;
    CableLed[CABLE_SW4][PORT1A].gpioNumB9Present = 305;
    CableLed[CABLE_SW4][PORT1B].gpioNumA32Present = 306;
    CableLed[CABLE_SW4][PORT1B].gpioNumB9Present = 307;
    CableLed[CABLE_SW4][PORT1A].gpioNumPresent = 308;
    CableLed[CABLE_SW4][PORT1B].gpioNumPresent = 309;
    CableLed[CABLE_SW4][PORT1A].gpioNumLED = 310;
    CableLed[CABLE_SW4][PORT1B].gpioNumLED = 311;

    CableLed[CABLE_SW4][PORT2A].gpioNumA32Present = 312;
    CableLed[CABLE_SW4][PORT2A].gpioNumB9Present = 313;
    CableLed[CABLE_SW4][PORT2B].gpioNumA32Present = 314;
    CableLed[CABLE_SW4][PORT2B].gpioNumB9Present = 315;
    CableLed[CABLE_SW4][PORT2A].gpioNumPresent = 316;
    CableLed[CABLE_SW4][PORT2B].gpioNumPresent = 317;
    CableLed[CABLE_SW4][PORT2A].gpioNumLED = 318;
    CableLed[CABLE_SW4][PORT2B].gpioNumLED = 319;
}

void open_gpio()
{
    int i = 0;
    int j = 0;
    char buff_path[256] = "";

    for(i=0;i<MAX_CABLE_SW;i++)
    {
        for(j=0;j<MAX_CABLE_PORT;j++)
        {
            sprintf(buff_path, "echo %d > /sys/class/gpio/export", CableLed[i][j].gpioNumA32Present);
            system(buff_path);

            sprintf(buff_path, "echo %d > /sys/class/gpio/export", CableLed[i][j].gpioNumB9Present);
            system(buff_path);            
            
            sprintf(buff_path, "echo %d > /sys/class/gpio/export", CableLed[i][j].gpioNumPresent);
            system(buff_path);
            
            sprintf(buff_path, "echo %d > /sys/class/gpio/export", CableLed[i][j].gpioNumLED);
            system(buff_path);

            sprintf(buff_path, "echo in > /sys/class/gpio/gpio%d/direction", CableLed[i][j].gpioNumA32Present);
            system(buff_path);

            sprintf(buff_path, "echo in > /sys/class/gpio/gpio%d/direction", CableLed[i][j].gpioNumB9Present);
            system(buff_path);

            sprintf(buff_path, "echo out > /sys/class/gpio/gpio%d/direction", CableLed[i][j].gpioNumPresent);
            system(buff_path);

            sprintf(buff_path, "echo out > /sys/class/gpio/gpio%d/direction", CableLed[i][j].gpioNumLED);
            system(buff_path);
        }
    }
}

int main(gint argc, gchar *argv[])
{
    init_cable_gpio_mapping();
    open_gpio();
    check_cable_status();
	return 0;
}
