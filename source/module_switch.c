#include "module_switch.h"

#include <switch.h>

const JanetAbstractType switch_event_type =
{
    "switch/event", JANET_ATEND_NAME
};

static Janet module_switch_event_wait(int32_t argc, Janet* argv)
{
    janet_arity(argc, 1, 2);
    
    Event* event = (Event*) janet_getabstract(argv, 0, &switch_event_type);
    u64 timeout = UINT64_MAX;
    if(argc == 2)
    {
        Janet jTimeout = argv[1];
        timeout = janet_unwrap_u64(jTimeout);
    }

    Result rc = eventWait(event, timeout);
    if(R_FAILED(rc))
        janet_panicf("failed to wait for event: code %#x", rc);
    
    return janet_wrap_nil();
}

const JanetReg switch_cfuns[] =
{
    {
        "switch/event-wait", module_switch_event_wait,
        "(switch/event-wait event &opt timeout)\n\nWaits until an event is fired."
    },
    {NULL, NULL, NULL}
};
