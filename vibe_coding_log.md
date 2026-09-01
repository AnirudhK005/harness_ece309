# Vibe Coding Log

Architecture: simple CLI in C.
- Input method: fgets, max 127 chars.
- History: circular buffer history[5][128], stores last 5 user turns.
- Precedence: math (entire input) -> hello (contains) -> history (exact) -> echo.
- Math operators supported: + - * / % ^ (% uses fmod).

Iteration: 1
User: alpha
Decision: echo
Program: You said: alpha

Iteration: 2
User: hello world
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 3
User: 3 * 4
Decision: math
Program: Result: 12

Iteration: 4
User: hello 2 + 3
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 5
User: 2 + 3 hello
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 6
User: 2 + 3
Decision: math
Program: Result: 5

Iteration: 7
User: 2 + 3 is nice
Decision: echo
Program: You said: 2 + 3 is nice

Iteration: 8
User: foo
Decision: echo
Program: You said: foo

Iteration: 9
User: exit
Decision: exit
Program: Program exiting.



**ASAN+LSAN run output**
dyld[12950]: Library not loaded: @rpath/libclang_rt.asan_osx_dynamic.dylib
  Referenced from: <0D3276C1-346F-3EDB-9B88-6F171206858C> /Users/anirudhkopparthi/harness_ece309/harness_asan
  Reason: tried: '/Users/anirudhkopparthi/harness_ece309/libclang_rt.asan_osx_dynamic.dylib' (no such file), '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib' (no such file), '/System/Volumes/Preboot/Cryptexes/OS/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib' (no such file), '/Users/anirudhkopparthi/harness_ece309/libclang_rt.asan_osx_dynamic.dylib' (no such file), '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib' (no such file), '/System/Volumes/Preboot/Cryptexes/OS/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/17/lib/darwin/libclang_rt.asan_osx_dynamic.dylib' (no such file)


**leaks --atExit output**
> You said: alpha
> Hello! Nice to meet you.
> Result: 12
> Hello! Nice to meet you.
> Hello! Nice to meet you.
> Result: 5
> You said: 2 + 3 is nice
> You said: foo
> Exiting...
Process 12953 is not debuggable. Due to security restrictions, leaks can only show or save contents of readonly memory of restricted processes.

Process:         harness [12953]
Path:            /Users/USER/*/harness
Load Address:    0x104114000
Identifier:      harness
Version:         0
Code Type:       ARM64
Platform:        macOS
Parent Process:  leaks [12952]
Target Type:     live task

Date/Time:       2026-09-01 17:32:43.549 -0400
Launch Time:     2026-09-01 17:32:43.241 -0400
OS Version:      macOS 26.5.2 (25F84)
Report Version:  7
Analysis Tool:   /usr/bin/leaks

Physical footprint:         2320K
Physical footprint (peak):  2320K
Idle exit:                  untracked
----

leaks Report Version: 4.0, multi-line stacks
Process 12953: 193 nodes malloced for 22 KB
Process 12953: 0 leaks for 0 total leaked bytes.



**MallocStackLogging leaks output**
leaks(12960) MallocStackLogging: could not tag MSL-related memory as no_footprint, so those pages will be included in process footprint - No such file or directory (2)
leaks(12960) MallocStackLogging: recording malloc (and VM allocation) stacks using lite mode
> You said: alpha
> Hello! Nice to meet you.
> Result: 12
> Hello! Nice to meet you.
> Hello! Nice to meet you.
> Result: 5
> You said: 2 + 3 is nice
> You said: foo
> Exiting...
Process 12961 is not debuggable. Due to security restrictions, leaks can only show or save contents of readonly memory of restricted processes.

Process:         harness [12961]
Path:            /Users/USER/*/harness
Load Address:    0x102520000
Identifier:      harness
Version:         0
Code Type:       ARM64
Platform:        macOS
Parent Process:  leaks [12960]
Target Type:     live task

Date/Time:       2026-09-01 17:32:44.965 -0400
Launch Time:     2026-09-01 17:32:44.956 -0400
OS Version:      macOS 26.5.2 (25F84)
Report Version:  7
Analysis Tool:   /usr/bin/leaks

Physical footprint:         2304K
Physical footprint (peak):  2304K
Idle exit:                  untracked
----

leaks Report Version: 4.0, multi-line stacks
Process 12961: 193 nodes malloced for 22 KB
Process 12961: 0 leaks for 0 total leaked bytes.



**ASAN run output (post-Xcode update)**
==13821==AddressSanitizer: detect_leaks is not supported on this platform.
