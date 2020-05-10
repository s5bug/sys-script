#include "module_vi.h"

#include <switch.h>

#include "module_switch.h"

static const JanetAbstractType vi_display_type =
{
    "vi/display", JANET_ATEND_NAME
};

static Janet module_vi_default_display(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 0);

    ViDisplay* disp = (ViDisplay*) janet_abstract(&vi_display_type, sizeof(ViDisplay));

    Result rc = viOpenDefaultDisplay(disp);
    if(R_FAILED(rc))
        janet_panicf("failed to open default display: code %#x", rc);
    
    return janet_wrap_abstract((void*) disp);
}

static Janet module_vi_vsync_event(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);

    ViDisplay* disp = (ViDisplay*) janet_getabstract(argv, 0, &vi_display_type);

    Event* evt = (Event*) janet_abstract(&switch_event_type, sizeof(Event));

    Result rc = viGetDisplayVsyncEvent(disp, evt);
    if(R_FAILED(rc))
        janet_panicf("failed to get display's vsync event: code %#x", rc);
    
    return janet_wrap_abstract((void*) evt);
}

const JanetReg vi_cfuns[] =
{
    {
        "vi/default-display", &module_vi_default_display,
        "(vi/default-display)\n\nOpens the Switch's default display."
    },
    {
        "vi/vsync-event", &module_vi_vsync_event,
        "(vi/vsync-event)\n\nGets the VSync event of the specified display."
    },
    {NULL, NULL, NULL}
};
