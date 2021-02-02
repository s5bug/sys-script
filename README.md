# sys-script

sys-script allows you to run [Janet](https://janet-lang.org/) scripts on your Switch.

## Usage

_TODO: Fully flesh out this section_

sys-script's Title ID is `4200736372697074`.

On startup, your Switch will execute `sdmc:/script/main.janet`, outputting logs
in `sdmc:/script/log`.

## Additional Functions

### `switch`

#### `(switch/event-close ev)`

Closes (frees) the event `ev`.

#### `(switch/event-wait ev)`

Waits for the firing of event `ev`.

### `hid`

#### `(hid/keyboard states &opt max)`

Fetches `n` keyboard states, where `n` is `max` if it is present, otherwise it
is `1`. Returns a table that looks like

```lisp
@{
  :sampling-number 0x################
  :modifiers 0x################
  :keys @[
    0x################
    0x################
    0x################
    0x################
  ]
}
```

This table should directly match
[https://switchbrew.github.io/libnx/structHidKeyboardState.html](`HidKeyboardState`).

### `hiddbg`

#### `(hiddbg/attach type interface body buttons left-grip right-grip)`

Attaches and returns a new virtual controller. `type` may be one of
`:pro-controller`. `interface` may be one of `:bluetooth`, `:rail`, `:usb`.
`body`, `buttons`, `left-grip`, and `right-grip` are all numbers in
little-endian RGBA (`0xAABBGGRR`).

#### `(hiddbg/detach controller)`

Detaches a virtual controller.

#### `(hiddbg/set-buttons controller flags)`

Sets the buttons of `controller` to `flags`.
_TODO: Document how to figure out your button flags_

#### `(hiddbg/set-joystick controller stick x y)`

Sets the joystick `stick` of `controller` to (`x`, `y`). `stick` may be
`:left` or `:right`.

### `vi`

#### `(vi/display-open &opt id)`

Opens a display. `id` may be a string. If `id` is missing, the default display
will be opened.

#### `(vi/display-close disp)`

Closes `disp`.

#### `(vi/display-event-vsync disp)`

Returns `disp`'s VSync event.
