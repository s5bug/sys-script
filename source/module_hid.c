#include "module_hid.h"

#include <switch.h>

static Janet module_hid_keyboard_states(int32_t argc, Janet* argv) {
    janet_arity(argc, 0, 1);

    uint64_t max = 1;
    if(argc == 1) {
        max = janet_unwrap_u64(argv[0]);
    }

    HidKeyboardState states[max];
    uint64_t len = hidGetKeyboardStates(states, max);

    JanetArray* resultArray = janet_array(len);
    for(int i = 0; i < len; i++) {
        JanetTable* table = janet_table(3);

        janet_table_put(table, janet_wrap_keyword(janet_cstring("sampling-number")), janet_wrap_u64(states[i].sampling_number));
        janet_table_put(table, janet_wrap_keyword(janet_cstring("modifiers")), janet_wrap_u64(states[i].modifiers));

        JanetArray* keyArray = janet_array(4);

        for(int k = 0; k < 3; k++) {
            janet_array_push(keyArray, janet_wrap_u64(states[i].keys[k]));
        }
        
        janet_table_put(table, janet_wrap_keyword(janet_cstring("keys")), janet_wrap_array(keyArray));

        janet_array_push(resultArray, janet_wrap_table(table));
    }

    return janet_wrap_array(resultArray);
}

const JanetReg hid_cfuns[] =
{
    {
        "hid/keyboard-states", module_hid_keyboard_states,
        "(hid/keyboard-states &opt max)\n\nReturns max number of keyboard states, or 1 if max is not present."
    },
    {NULL, NULL, NULL}
};
