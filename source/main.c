// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

// Include Janet, the script runtime
#include "janet.h"

// Include custom modules
#include "module_switch.h"
#include "module_hid.h"
#include "module_hiddbg.h"
#include "module_vi.h"

// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.
#define INNER_HEAP_SIZE 0x400000
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char   nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void)
{
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

// Init/exit services, update as needed.
static const SocketInitConfig socketConfig =
{
    .bsdsockets_version = 1,

    .tcp_tx_buf_size = 1024,
    .tcp_rx_buf_size = 256,
    .tcp_tx_buf_max_size = 0,
    .tcp_rx_buf_max_size = 0,

    .udp_tx_buf_size = 0x2400,
    .udp_rx_buf_size = 0xA500,

    .sb_efficiency = 2,
};

void __attribute__((weak)) __appInit(void)
{
    Result rc;

    // Initialize default services.
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    if (hosversionGet() == 0)
    {
        rc = setsysInitialize();
        if (R_SUCCEEDED(rc))
        {
            SetSysFirmwareVersion fw;
            rc = setsysGetFirmwareVersion(&fw);
            if (R_SUCCEEDED(rc))
                hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
            setsysExit();
        }
    }

    rc = hidInitialize();
    if (R_FAILED(rc))
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    fsdevMountSdmc();

    rc = hiddbgInitialize();
    if (R_FAILED(rc))
        fatalThrow(rc);
    
    rc = viInitialize(ViServiceType_System);
    if (R_FAILED(rc))
        fatalThrow(rc);

    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalThrow(rc);
    
    rc = socketInitialize(&socketConfig);
    if (R_FAILED(rc))
        fatalThrow(rc);
}

void __attribute__((weak)) userAppExit(void);

void __attribute__((weak)) __appExit(void)
{
    // Cleanup default services.
    socketExit();
    timeExit();
    viExit();
    hiddbgExit();
    fsdevUnmountAll();
    fsExit();
    hidExit();
    smExit();
}

// Main program entrypoint
int main(int argc, char* argv[])
{
    // Initialization code can go here.
    hidInitializeKeyboard();

    Result rc = hiddbgAttachHdlsWorkBuffer();
    if (R_FAILED(rc))
        fatalThrow(rc);

    janet_init();

    // Your code / main loop goes here.
    // If you need threads, you can use threadCreate etc.
    JanetTable* env = janet_core_env(NULL);
    janet_cfuns(env, NULL, switch_cfuns);
    janet_cfuns(env, NULL, hid_cfuns);
    janet_cfuns(env, NULL, hiddbg_cfuns);
    janet_cfuns(env, NULL, vi_cfuns);

    int result = janet_dostring(env, "(dofile \"sdmc:/script/sys/boot.janet\")\n", "sys-script", NULL);
    if(result != 0) {
        fatalThrow(MAKERESULT(0x7A5, result));
    }

    // TODO: Figure out the pipe situation
    // janet_loop();

    // Deinitialization and resources clean up code can go here.
    janet_deinit();

    rc = hiddbgReleaseHdlsWorkBuffer();

    return 0;
}
