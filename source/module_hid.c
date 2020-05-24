#include "module_hid.h"

#include <switch.h>

static Janet module_hid_scan_input(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 0);

    hidScanInput();

    return janet_wrap_nil();
}

static Janet module_hid_keyboard_down(int32_t argc, Janet* argv)
{
    janet_fixarity(argc, 1);
    int32_t scancode = janet_getinteger(argv, 0);

    return janet_wrap_boolean(hidKeyboardDown(scancode));
}

const JanetReg hid_cfuns[] =
{
    {
        "hid/scan-input", module_hid_scan_input,
        "(hid/scan-input)\n\nInitiates a scan of HID input."
    },
    {
        "hid/keyboard-down", module_hid_keyboard_down,
        "(hid/keyboard-down key)\n\nReturns whether key was pressed down before the last scan."
    },
    {NULL, NULL, NULL}
};
