(defn- bit [x] (blshift (int/u64 "1") x))

(def buttons
  @{"A" (int/u64 "1")
    "B" (int/u64 "2")
    "X" (int/u64 "4")
    "Y" (int/u64 "8")
    "L" (int/u64 "64")
    "R" (int/u64 "128")
    "ZL" (int/u64 "256")
    "ZR" (int/u64 "512")
    "DLEFT" (int/u64 "4096")
    "DUP" (int/u64 "8192")
    "DRIGHT" (int/u64 "16384")
    "DDOWN" (int/u64 "32768")
    "PLUS" (int/u64 "1024")
    "MINUS" (int/u64 "2048")})

(defn clearinputs [con]
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
  (def res (peg/match ssctf line))
  (def nframe (scan-number (in res 0)))
  (def btns (reduce bor (int/u64 "0") (map buttons (array/slice res 1 -5))))
  (switch/event-wait vsync)
  (var usedframe (+ frame 1))
  (clearinputs controller)
  (when (> nframe usedframe)
    (each i (range usedframe nframe)
      (switch/event-wait vsync)))
  (hiddbg/set-buttons controller btns)
  (hiddbg/set-joystick controller :left ;(map scan-number (array/slice res -5 -3)))
  (hiddbg/set-joystick controller :right ;(map scan-number (array/slice res -3 -1)))
  nframe)

(defn run-tas [p controller]

  (with [s8 (file/open p :r) file/close]
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
