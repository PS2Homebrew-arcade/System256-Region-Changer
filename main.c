#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <kernel.h>
#include <tamtypes.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <loadfile.h>
#include <fileXio_rpc.h>
#include <debug.h>

int daemon_status = 0;

/** Reads the i.Link ID of the console.
 * All consoles have an i.Link ID, including those that do not have a physical i.Link port.
 *
 * @param buffer Pointer to the buffer that will contain the data read.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
extern int sceCdRI(u8 *buffer, u32 *result);

/** Writes a new i.Link ID for the console.
 * Starting from Mechacon firmware version 50000, a unlock combination (0x03 0x46 and 0x03 0x47) needs to be executed first.
 *
 * @param buffer Pointer to the buffer that contains the new i.Link ID.
 * @param result Result code.
 * @return 1 on success, 0 on failure.
 */
extern int sceCdWI(const u8 *buffer, u32 *result);

#define DECLIRX(IRX) unsigned char ALIGNED(16) IRX[]
DECLIRX(IOMANX)={
#embed "irx/iomanX.irx"
};

DECLIRX(FILEXIO)= {
#embed "irx/fileXio.irx"
};
unsigned int size_IOMANX = sizeof IOMANX;
unsigned int size_FILEXIO = sizeof FILEXIO;

#define LOADMODULE(_irx, ret) SifExecModuleBuffer(&_irx, size_##_irx, 0, NULL, ret)
#define LOADMODULEFILE(path, ret) SifLoadStartModule(path, 0, NULL, ret)
#define MODULE_OK(id, ret) (id >= 0 && ret != 1)
#define REGION_FMT "{%02X %02X %02X %02X %02X %02X %02X %02X}"
#define REGION_ARGS(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]

void scr_fillhalf(int size, char filler) {
    for (int x=0; x<(80-size)/2;x++) scr_printf("%c", filler);
}

void scr_centerputs(const char* buf, char fillerbyte) {
    scr_fillhalf(strlen(buf), fillerbyte);
    scr_printf("%s", buf);
    scr_fillhalf(strlen(buf), fillerbyte);
    if(strlen(buf) % 2 != 0) scr_printf("\n");
}

#define REGIONSIZE 8
// buf:00 32 1F C7 FA D6 EE F0 00 00 00 00 00 00 00 00
//        32 1F C7 FA D6 EE F0 1C
const u8 REGION_ASIA4[REGIONSIZE] = {0x32, 0x1F, 0xC7, 0xFA, 0xD6, 0xEE, 0xF0, 0x1C}; // 32 1F C7 FA D6 EE F0 1C
const u8 REGION_ASIA5[REGIONSIZE] = {0x41, 0x46, 0x53, 0x2F, 0x1E, 0xFD, 0x0F, 0xE0}; // 41 46 53 2F 1E FD 0F E0
const u8 REGION_JAPAN[REGIONSIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define REGIONCNT 3
const u8* regions[REGIONCNT] = {
    &REGION_ASIA4[0],
    &REGION_ASIA5[0],
    &REGION_JAPAN[0],
};
const char* aliases[REGIONCNT] = {
    "ASIA 4",
    "ASIA 5",
    "JAPAN",
};

#if REGION == 0
#elif REGION == 1
#elif REGION == 2
#elif REGION > REGIONCNT
#error out of bounds region
#else
#error invalid stock region
#endif


u8 current_region[REGIONSIZE] = {0};
u8 new_region[REGIONSIZE] = {0};

int main()
{
    memcpy(new_region, regions[REGION], REGIONSIZE);
    u32 res = -1, resp = -1;
    scr_printf("\n\n");
    scr_centerputs(" SYSTEM256 REGION CHANGER ", '-');
    scr_centerputs("Coded by El_isra", ' ');
    scr_centerputs("https://github.com/PS2Homebrew-arcade/System256-Region-Changer", ' ');

    if (!daemon_status) {
        scr_setfontcolor(0x0000F0);
        scr_printf("\n\n\n\t[ERR]: failed to load rom0:DAEMON\n\taborting region change for safety");
        goto death;
    }

    scr_printf("\n\t DETECTING REGION:");
    res = sceCdRI(current_region, &resp);
    if (res < 0 || resp != 0) {
        scr_setfontcolor(0x0000F0);
        scr_printf("[ERR] Cannot read region (%d,%d)\n", res, resp);
        goto death;
    }
    scr_setfontcolor(0x00F0F0);
    scr_printf(REGION_FMT, REGION_ARGS(current_region));
    scr_setfontcolor(0xFFFFFF);
    int found = -1;
    for (int i = 0; i < REGIONCNT; i++) {
        if (!memcmp(current_region, regions[i], REGIONSIZE)) {
            scr_setfontcolor(0x00F000);
            scr_printf("  MATCH:");
            scr_setfontcolor(0xFFFFFF);
            scr_printf("%s\n", aliases[i]);
            found = i;
        }
    }
    if (found == REGION) {
        scr_printf("\t!!! THIS SYSTEM256 IS ALREADY CONFIGURED ON THE %s REGION\n", aliases[REGION]);
        goto death;
    }
    if (found < 0) found = 0;
    if (found < 0) {
        scr_setfontcolor(0x0000F0);
        scr_printf("\n\t!!! Unknown regional signature\n");
        scr_setfontcolor(0xFFFFFF);

        scr_printf("\tContact developer at:\n");
        scr_printf("\tgithub.com/PS2Homebrew-arcade/SYS256-region-change/issues\n");
        scr_setfontcolor(0x00F0F0);
        scr_printf("\tremember this is only for SYSTEM256\n");
        goto death;
    } else {
        scr_printf("\tsignature applied:" REGION_FMT "\n\n",REGION_ARGS(new_region));
        scr_printf("\t  SYSTEM WILL CHANGE FROM:\n");
        scr_printf("\t     %10s --> %-10s\n", aliases[found], aliases[REGION]);
        for (int i = 30; i > 0; i--) {
            scr_printf("\r\tProceeding in %d seconds...", i);
            sleep(1);
        }
        scr_printf("\n");
        res = sceCdWI(new_region, &resp);
        if (res < 0 || resp != 0) {
            scr_setfontcolor(0x0000F0);
            scr_printf("\t[NG] ");
            scr_setfontcolor(0xFFFFFF);
            scr_printf("Cannot apply new region (%d,%d)\n", res, resp);
        } else {
            scr_printf("\tsceCdWI(): r:%d, res:%d\n", res, resp);
            scr_setfontcolor(0x00F000);
            scr_printf("\t[OK] ");
            scr_setfontcolor(0xFFFFFF);
            scr_printf("Successfully applied new regional signature\n");
        }
    }
    sleep(30);
    return 0;

death:
    sleep(60);
    return 0;
}


void _ps2sdk_memory_init() {
    int id, ret;
    while (!SifIopReset("", 0));
    while (!SifIopSync()) {};
    LOADMODULEFILE("rom0:CDVDFSV", NULL);
    LOADMODULEFILE("rom0:SIO2MAN", NULL);
    LOADMODULEFILE("rom0:MCMAN", NULL);
    id = LOADMODULEFILE("rom0:DAEMON", &ret);
    daemon_status = (id > 0 && ret != 1);
    id = LOADMODULE(IOMANX,  &ret); printf("IOMANX:  %d %d\n", id, ret);
    id = LOADMODULE(FILEXIO, &ret); printf("FILEXIO: %d %d\n", id, ret);
    fileXioInit();
    init_scr();
    scr_setCursor(0);
    printf("%s() ready\n", __FUNCTION__);
}
