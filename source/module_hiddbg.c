#include "module_hiddbg.h"

#include <switch.h>

static const JanetAbstractType controller_state_type =
{
    "hiddbg/controller-state", JANET_ATEND_NAME
};

static Janet module_hiddbg_attach(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 6);
    JanetKeyword deviceType = janet_getkeyword(argv, 0);
    JanetKeyword interfaceType = janet_getkeyword(argv, 1);
    u32 body = (u32) janet_getnumber(argv, 2);
    u32 buttons = (u32) janet_getnumber(argv, 3);
    u32 leftGrip = (u32) janet_getnumber(argv, 4);
    u32 rightGrip = (u32) janet_getnumber(argv, 5);

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
        janet_panicf("failed to attach virtual device: code %#x", rc);
    
    HiddbgHdlsState* state = (HiddbgHdlsState*) janet_abstract(&controller_state_type, sizeof(HiddbgHdlsState));
    memset(state, 0, sizeof(HiddbgHdlsState));
    state->batteryCharge = 4;
    
    rc = hiddbgSetHdlsState(handle, state);
    if(R_FAILED(rc))
        janet_panicf("failed to set state of virtual device: code %#x", rc);

    Janet jHandle = janet_wrap_u64(handle);
    Janet jState = janet_wrap_abstract((void*) state);
    
    Janet jVirtualDeviceArr[2] = {jHandle, jState};
    JanetTuple jVirtualDevice = janet_tuple_n(jVirtualDeviceArr, 2);

    return janet_wrap_tuple(jVirtualDevice);
}

static Janet module_hiddbg_detach(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);
    JanetTuple handleAndState = janet_gettuple(argv, 0);
    Janet jHandle = handleAndState[0];
    if(janet_checktype(jHandle, JANET_ABSTRACT))
    {
        u64 handle = janet_unwrap_u64(jHandle);

        Result rc = hiddbgDetachHdlsVirtualDevice(handle);
        if(R_FAILED(rc))
            janet_panicf("failed to detach virtual device: code %#x", rc);
    }
    else
    {
        janet_panicf("bad slot 0, expected tuple of u64 and pointer, got %v", argv[0]);
    }

    return janet_wrap_nil();
}

static Janet module_hiddbg_set_buttons(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 2);
    JanetTuple handleAndState = janet_gettuple(argv, 0);
    Janet jButtons = argv[1];
    u64 buttons = 0;
    if(janet_checktype(jButtons, JANET_ABSTRACT))
    {
        buttons = janet_unwrap_u64(jButtons);
    }
    else
    {
        janet_panicf("bad slot 1, expected u64, got %v", jButtons);
    }

    Janet jHandle = handleAndState[0];
    Janet jState = handleAndState[1];
    if(janet_checktype(jHandle, JANET_ABSTRACT) && janet_checktype(jState, JANET_POINTER))
    {
        u64 handle = janet_unwrap_u64(jHandle);
        HiddbgHdlsState* state = (HiddbgHdlsState*) janet_unwrap_abstract(jState);

        state->buttons = buttons;

        hiddbgSetHdlsState(handle, state);
    }
    else
    {
        janet_panicf("bad slot 0, expected tuple of u64 and pointer, got %v", argv[0]);
    }

    return janet_wrap_nil();
}

static Janet module_hiddbg_set_joystick(int32_t argc, Janet* argv)
{
    janet_arity(argc, 3, 4);
    JanetTuple handleAndState = janet_gettuple(argv, 0);
    Janet jIndex = argv[1];
    size_t joystickIndex = 0;
    if(janet_checktype(jIndex, JANET_NUMBER))
    {
        joystickIndex = (size_t) janet_unwrap_number(jIndex);
    }
    else if(janet_checktype(jIndex, JANET_KEYWORD))
    {
        if(janet_keyeq(jIndex, "left"))
        {
            joystickIndex = 0;
        }
        else if(janet_keyeq(jIndex, "right"))
        {
            joystickIndex = 1;
        }
        else
        {
            janet_panicf("bad joystick keyword, expected :left|:right, got %v", jIndex);
        }
    }
    else
    {
        janet_panicf("bad slot 1, expected number|keyword, got %v", jIndex);
    }
    s32 jx = 0;
    s32 jy = 0;
    if(argc == 3)
    {
        JanetTuple xAndY = janet_gettuple(argv, 2);

        Janet x = xAndY[0];
        Janet y = xAndY[1];
        if(janet_checktype(x, JANET_NUMBER))
        {
            jx = janet_unwrap_integer(x);
        }
        else
        {
            janet_panicf("bad joystick axis position, expected signed 32-bit integer, got %v", jx);
        }
        if(janet_checktype(y, JANET_NUMBER))
        {
            jy = janet_unwrap_integer(y);
        }
        else
        {
            janet_panicf("bad joystick axis position, expected signed 32-bit integer, got %v", jy);
        }
    }
    else if(argc == 4)
    {
        jx = janet_getinteger(argv, 2);
        jy = janet_getinteger(argv, 3);
    }

    Janet jHandle = handleAndState[0];
    Janet jState = handleAndState[1];
    if(janet_checktype(jHandle, JANET_ABSTRACT) && janet_checktype(jState, JANET_POINTER))
    {
        u64 handle = janet_unwrap_u64(jHandle);
        HiddbgHdlsState* state = (HiddbgHdlsState*) janet_unwrap_abstract(jState);

        state->joysticks[joystickIndex].dx = jx;
        state->joysticks[joystickIndex].dy = jy;

        hiddbgSetHdlsState(handle, state);
    }
    else
    {
        janet_panicf("bad slot 0, expected tuple of u64 and pointer, got %v", argv[0]);
    }

    return janet_wrap_nil();
}

const JanetReg hiddbg_cfuns[] =
{
    {
        "hiddbg/attach", module_hiddbg_attach,
        "(hiddbg/attach type interface body buttons left-grip right-grip)\n\nAttaches and returns a new virtual controller.",
    },
    {
        "hiddbg/detach", module_hiddbg_detach,
        "(hiddbg/detach controller)\n\nDetaches a virtual controller.",
    },
    {
        "hiddbg/set-buttons", module_hiddbg_set_buttons,
        "(hiddbg/set-buttons controller buttons)\n\nSets the buttons of the specified controller.",
    },
    {
        "hiddbg/set-joystick", module_hiddbg_set_joystick,
        "(hiddbg/set-joystick controller index x y)\n\nSets a joystick of the specified controller.",
    },
    {NULL, NULL, NULL}
};
