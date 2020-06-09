(defn- bit [x] (blshift (int/u64 "1") x))

(def buttons
  @{"A" (bit 0)
    "B" (bit 1)
    "X" (bit 2)
    "Y" (bit 3)
    "LSTICK" (bit 4)
    "RSTICK" (bit 5)
    "L" (bit 6)
    "R" (bit 7)
    "ZL" (bit 8)
    "ZR" (bit 9)
    "PLUS" (bit 10)
    "MINUS" (bit 11)
    "DLEFT" (bit 12)
    "DUP" (bit 13)
    "DRIGHT" (bit 14)
    "DDOWN" (bit 15)})

(defn clearinputs [con]
  "Clears all button and joystick inputs."
  (hiddbg/set-buttons con (int/u64 "0"))
  (hiddbg/set-joystick con :left 0 0)
  (hiddbg/set-joystick con :right 0 0))

(def ssctf
  (peg/compile
    ~{:btn (+ ,;(keys buttons))
      :qbtn (* "KEY_" (<- :btn))
      :btns (+ "NONE" (* :qbtn (any (* ";" :qbtn))))
      :axis (* (? "-") :d+)
      :stick (* (<- :axis) ";" (<- :axis))
      :main (* (<- :d+) " " :btns " " :stick " " :stick)}))

(defn runinput [line controller vsync frame]
  "Runs a single line of a TAS script."
  (def res (peg/match ssctf line))
  (def nframe (scan-number (in res 0)))
  (def btns (reduce bor (int/u64 "0") (map buttons (array/slice res 1 -5))))
  (switch/event-wait vsync)
  (def usedframe (+ frame 1))
  (clearinputs controller)
  (when (> nframe usedframe)
    (each i (range usedframe nframe)
      (switch/event-wait vsync)))
  (hiddbg/set-buttons controller btns)
  (hiddbg/set-joystick controller :left ;(map scan-number (array/slice res -5 -3)))
  (hiddbg/set-joystick controller :right ;(map scan-number (array/slice res -3 -1)))
  nframe)

(defn run-tas [path controller]
  "Runs the TAS script located at path on the controller passed."
  (with [s8 (file/open path :r) file/close]
    (with [disp (vi/display-open) vi/display-close]
      (with [vsync (vi/display-event-vsync disp) switch/event-close]

        (defn frames [n] (each i (range n) (switch/event-wait vsync)))

        (var fileempty false)
        (var frame 0)

        (switch/event-wait vsync)

        (while (not fileempty)
          (def line (file/read s8 :line))
          (if (nil? line)
            (set fileempty true)
            (set frame (runinput line controller vsync frame))))
        (when fileempty
          (switch/event-wait vsync)
          (clearinputs controller))))))
