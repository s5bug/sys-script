#include "module_hiddbg.h"

#include <switch.h>

typedef struct {
    u64 handle;
    HiddbgHdlsState state;
} Controller;

static const JanetAbstractType controller_type =
{
    "hiddbg/controller", JANET_ATEND_NAME
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

    Controller* controller = janet_abstract(&controller_type, sizeof(Controller));
    memset(controller, 0, sizeof(Controller));
    
    Result rc = hiddbgAttachHdlsVirtualDevice(&controller->handle, &info);
    if(R_FAILED(rc))
        janet_panicf("failed to attach virtual device: code %#x", rc);
    
    controller->state.batteryCharge = 4;
    
    rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
        janet_panicf("failed to set state of virtual device: code %#x", rc);

    return janet_wrap_abstract(controller);
}

static Janet module_hiddbg_detach(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);
    Controller* controller = janet_getabstract(argv, 0, &controller_type);
    
    Result rc = hiddbgDetachHdlsVirtualDevice(controller->handle);
    if(R_FAILED(rc))
        janet_panicf("failed to detach virtual device: code %#x", rc);

    return janet_wrap_nil();
}

static Janet module_hiddbg_set_buttons(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 2);
    Controller* controller = janet_getabstract(argv, 0, &controller_type);
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

    controller->state.buttons = buttons;
    Result rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
        janet_panicf("failed to set state of virtual device: code %#x", rc);

    return janet_wrap_nil();
}

static Janet module_hiddbg_set_joystick(int32_t argc, Janet* argv)
{
    janet_arity(argc, 3, 4);
    Controller* controller = janet_getabstract(argv, 0, &controller_type);
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

    controller->state.joysticks[joystickIndex].dx = jx;
    controller->state.joysticks[joystickIndex].dy = jy;
    Result rc = hiddbgSetHdlsState(controller->handle, &controller->state);
    if(R_FAILED(rc))
        janet_panicf("failed to set state of virtual device: code %#x", rc);

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
