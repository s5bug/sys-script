# sys-script

sys-script allows you to run [Janet](https://janet-lang.org/) scripts on your Switch.

## Usage

_TODO: Fully flesh out this section_

sys-script's Title ID is `4200736372697074`.

On startup, your Switch will execute `sdmc:/boot.janet`, recording any output
or errors in `sdmc:/boot.janet.log`.

## Additional Functions

### `switch`

#### `(switch/event-close ev)`

Closes (frees) the event `ev`.

#### `(switch/event-wait ev)`

Waits for the firing of event `ev`.

### `hid`

#### `(hid/scan-input)`

Scans input from HID.

#### `(hid/keyboard-down key)`

Tests if `key` was down during the last scan.

### `hiddbg`

#### `(hiddbg/attach type interface body buttons left-grip right-grip)`

Attaches and returns a new virtual controller. `type` may be one of
`:pro-controller`. `interface` may be one of `:bluetooth`, `:rail`, `:usb`.
`body`, `buttons`, `left-grip`, and `right-grip` are all `(int/u64`s
in little-endian RGBA (`0xAABBGGRR`).

#### `(hiddbg/detach controller)`

Detaches a virtual controller.

#### `(hiddbg/set-buttons controller flags)`

Sets the buttons of `controller` to `flags`.
_TODO: Document how to figure out your button flags_

#### `(hiddbg/set-joystick controller stick x y)`

Sets the joystick `stick` of `controller` to (`x`, `y`). `stick` may be
`:left`, `:right`, or a number.

### `vi`

#### `(vi/default-display)`

Gets a reference to the Switch's default display.

#### `(vi/close-display display)`

Closes a reference to a display.

#### `(vi/vsync-event display)`

Gets a reference to a display's VSync event.
