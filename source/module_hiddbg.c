#include "module_hiddbg.h"

#include <switch.h>

#include "errors.h"

static JanetStruct stateToJanet(HiddbgHdlsState* state)
{
    Janet* jBeginJoysticks = janet_tuple_begin(JOYSTICK_NUM_STICKS);
    for(int32_t joystick = 0; joystick < JOYSTICK_NUM_STICKS; joystick++)
    {
        Janet positions[2] =
        {
            janet_wrap_integer(state->joysticks[joystick].dx),
            janet_wrap_integer(state->joysticks[joystick].dy)
        };
        jBeginJoysticks[joystick] = janet_wrap_tuple(janet_tuple_n(positions, 2));
    }
    JanetTuple jJoysticks = janet_tuple_end(jBeginJoysticks);

    JanetKV* jBeginState = janet_struct_begin(4);
    janet_struct_put(jBeginState, janet_ckeywordv("charge"), janet_wrap_integer(state->batteryCharge));
    janet_struct_put(jBeginState, janet_ckeywordv("flags"), janet_wrap_integer(state->flags));
    janet_struct_put(jBeginState, janet_ckeywordv("buttons"), janet_wrap_u64(state->buttons));
    janet_struct_put(jBeginState, janet_ckeywordv("joysticks"), janet_wrap_tuple(jJoysticks));
    return janet_struct_end(jBeginState);
}

static void janetToState(HiddbgHdlsState* state, JanetStruct jStruct)
{
    Janet jCharge = janet_struct_get(jStruct, janet_ckeywordv("charge"));
    Janet jFlags = janet_struct_get(jStruct, janet_ckeywordv("flags"));
    Janet jButtons = janet_struct_get(jStruct, janet_ckeywordv("buttons"));
    Janet jJoysticks = janet_struct_get(jStruct, janet_ckeywordv("joysticks"));

    if(janet_checktype(jCharge, JANET_NUMBER))
    {
        state->batteryCharge = janet_unwrap_integer(jCharge);
    }

    if(janet_checktype(jFlags, JANET_NUMBER))
    {
        state->flags = janet_unwrap_integer(jFlags);
    }

    if(janet_checktype(jButtons, JANET_NUMBER))
    {
        state->buttons = janet_unwrap_u64(jButtons); 
    }

    if(janet_checktype(jJoysticks, JANET_TUPLE))
    {
        JanetTuple jJoysticksTuple = janet_unwrap_tuple(jJoysticks);
        if(janet_tuple_length(jJoysticksTuple) == JOYSTICK_NUM_STICKS)
        {
            for(int32_t joystick = 0; joystick < JOYSTICK_NUM_STICKS; joystick++)
            {
                Janet jJoystickEntry = jJoysticksTuple[joystick];
                if(janet_checktype(jJoystickEntry, JANET_TUPLE))
                {
                    JanetTuple jJoystickEntryTuple = janet_unwrap_tuple(jJoystickEntry);
                    if(janet_tuple_length(jJoystickEntryTuple) == 2)
                    {
                        Janet jJoystickEntryX = jJoystickEntryTuple[0];
                        Janet jJoystickEntryY = jJoystickEntryTuple[1];

                        if(janet_checktype(jJoystickEntryX, JANET_NUMBER))
                        {
                            state->joysticks[joystick].dx = janet_unwrap_integer(jJoystickEntryX);
                        }

                        if(janet_checktype(jJoystickEntryY, JANET_NUMBER))
                        {
                            state->joysticks[joystick].dy = janet_unwrap_integer(jJoystickEntryY);
                        }
                    }
                }
            }
        }
        
    }
}

static Janet module_hiddbg_attach(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 6);
    JanetKeyword deviceType = janet_getkeyword(argv, 0);
    JanetKeyword interfaceType = janet_getkeyword(argv, 1);
    u32 body = janet_getnat(argv, 2);
    u32 buttons = janet_getnat(argv, 3);
    u32 leftGrip = janet_getnat(argv, 4);
    u32 rightGrip = janet_getnat(argv, 5);

    u8 deviceTypeNum = 0;
    Janet deviceTypeKw = janet_wrap_keyword(deviceType);
    if(janet_keyeq(deviceTypeKw, "pro-controller"))
    {
        deviceTypeNum = HidDeviceType_FullKey3;
    }
    else
    {
        janet_panic("unknown device type passed to hiddbg/attach");
    }

    u8 interfaceTypeNum = NpadInterfaceType_Unknown4;
    Janet interfaceTypeKw = janet_wrap_keyword(interfaceType);
    if(janet_keyeq(interfaceTypeKw, "bluetooth"))
    {
        interfaceTypeNum = NpadInterfaceType_Bluetooth;
    }
    else if(janet_keyeq(interfaceTypeKw, "rail"))
    {
        interfaceTypeNum = NpadInterfaceType_Rail;
    }
    else if(janet_keyeq(interfaceTypeKw, "usb"))
    {
        interfaceTypeNum = NpadInterfaceType_USB;
    }
    
    HiddbgHdlsDeviceInfo info = { 0 };
    info.deviceType = deviceTypeNum;
    info.npadInterfaceType = interfaceTypeNum;
    info.singleColorBody = body;
    info.singleColorButtons = buttons;
    info.colorLeftGrip = leftGrip;
    info.colorRightGrip = rightGrip;

    u64 handle;
    Result rc = hiddbgAttachHdlsVirtualDevice(&handle, &info);
    if(R_FAILED(rc))
        janet_panic("failed to attach virtual device");
    
    HiddbgHdlsState state = { 0 };
    state.batteryCharge = 4;
    
    rc = hiddbgSetHdlsState(handle, &state);
    if(R_FAILED(rc))
        janet_panic("failed to set state of virtual device");

    Janet jHandle = janet_wrap_u64(handle);
    Janet jState = janet_wrap_struct(stateToJanet(&state));
    
    Janet jVirtualDeviceArr[2] = {jHandle, jState};
    JanetTuple jVirtualDevice = janet_tuple_n(jVirtualDeviceArr, 2);

    return janet_wrap_tuple(jVirtualDevice);
}

Janet module_hiddbg_set_buttons(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 2);
    JanetTuple handleAndState = janet_gettuple(argv, 0);
    Janet jButtons = argv[1];
    u64 buttons = 0;
    if(janet_checktype(jButtons, JANET_NUMBER))
    {
        buttons = janet_unwrap_u64(jButtons);
    }

    Janet jHandle = handleAndState[0];
    Janet jState = handleAndState[1];
    if(janet_checktype(jHandle, JANET_NUMBER) && janet_checktype(jState, JANET_STRUCT))
    {
        u64 handle = janet_unwrap_u64(jHandle);
        JanetStruct state = janet_unwrap_struct(jState);
        HiddbgHdlsState nState = { 0 };

        janetToState(&nState, state);

        nState.buttons = buttons; // FIXME

        hiddbgSetHdlsState(handle, &nState);
    }

    return janet_wrap_nil();
}

const JanetReg hiddbg_cfuns[] =
{
    {
        "attach", module_hiddbg_attach,
        "(hiddbg/attach type interface body buttons left-grip right-grip)\n\nAttaches and returns a new virtual controller.",
    },
    {
        "set-buttons", module_hiddbg_set_buttons,
        "(hiddbg/set-buttons controller buttons)\n\nSets the buttons of the specified controller."
    },
    {NULL, NULL, NULL}
};
