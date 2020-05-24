#include "module_vi.h"

#include <switch.h>

#include "module_switch.h"

static const JanetAbstractType vi_display_type =
{
    "vi/display", JANET_ATEND_NAME
};

static Janet module_vi_display_open(int32_t argc, Janet* argv)
{
    janet_arity(argc, 0, 1);

    ViDisplay* disp = janet_abstract(&vi_display_type, sizeof(ViDisplay));
    memset(disp, 0, sizeof(ViDisplay));

    Result rc;
    if(argc == 0)
    {
        rc = viOpenDefaultDisplay(disp);
    }
    else
    {
        const char* str = janet_getcstring(argv, 0);
        rc = viOpenDisplay(str, disp);
    }
    
    if(R_FAILED(rc))
        janet_panicf("failed to open default display: code %#x", rc);
    
    return janet_wrap_abstract(disp);
}

static Janet module_vi_display_close(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);

    ViDisplay* disp = janet_getabstract(argv, 0, &vi_display_type);

    Result rc = viCloseDisplay(disp);
    if(R_FAILED(rc))
        janet_panicf("failed to close display: code %#x", rc);
    
    return janet_wrap_nil();
}

static Janet module_vi_display_event_vsync(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);

    ViDisplay* disp = janet_getabstract(argv, 0, &vi_display_type);

    Event* evt = janet_abstract(&switch_event_type, sizeof(Event));
    memset(evt, 0, sizeof(Event));

    Result rc = viGetDisplayVsyncEvent(disp, evt);
    if(R_FAILED(rc))
        janet_panicf("failed to get display's vsync event: code %#x", rc);
    
    return janet_wrap_abstract(evt);
}

const JanetReg vi_cfuns[] =
{
    {
        "vi/display-open", module_vi_display_open,
        "(vi/display-open &opt id)\n\nOpens a display."
    },
    {
        "vi/display-close", module_vi_display_close,
        "(vi/display-close disp)\n\nCloses a display."
    },
    {
        "vi/display-event-vsync", module_vi_display_event_vsync,
        "(vi/display-event-vnync disp)\n\nGets the VSync event of the specified display."
    },
    {NULL, NULL, NULL}
};
