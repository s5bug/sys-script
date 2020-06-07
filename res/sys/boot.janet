(def rootpath "sdmc:/script/")

(def logpath (string rootpath "log/"))

(def now (os/date))
(def logname (string/format "%d%02d%02d-%02d%02d%02d")
  (now :year)
  (now :month)
  (now :month-day)
  (now :hours)
  (now :minutes)
  (now :seconds))

(def logfile (string logpath logname ".log"))

(os/touch logfile)
(def log (file/open logfile :a))

(setdyn :out log)
(setdyn :err log)

(def mainpath (string rootpath "main.janet"))
(try (dofile mainpath) ([err fiber] (debug/stacktrace fiber)))

(file/close log)
