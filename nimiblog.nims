#!/usr/bin/env nim
mode = ScriptMode.Verbose

proc buildAll() =
  echo "Build All"
  exec "nim r -d:release index.nim"
  exec "nim r -d:release projects.nim"
  exec "nim r -d:release now.nim"
  exec "nim r -d:release -d:regendoc articles/index.nim"

buildAll()
