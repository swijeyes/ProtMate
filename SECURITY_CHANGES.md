# ProtMate 1.1.6 - security hardening

| # | Issue (src/pmate.cpp) | Severity | Fix |
|---|---|---|---|
| 1 | `pdbqt()`: filename concatenated into `system("cut ... file > out")` - arbitrary command execution (e.g. `x; rm -rf ~ #`) | Critical | Conversion done in-process (same `cut -c 1-60,70-79` semantics); no shell |
| 2 | `rname()`: directory / prefixes concatenated into `system("mv ...")` - command injection | Critical | `rename(2)`; prefixes validated (no `/`), directory must exist; never clobbers an existing file |
| 3 | `open()`: filename concatenated into `system("rasmol " + x)` - command injection | Critical | `fork`/`execvp` with an argv array; filename validated |
| 4 | `p5()`: arrays sized by line count but filled per number pair (and one extra `while(in)` pass) - heap buffer overflow from file contents | High | `std::vector` grown per pair actually read (`while (in >> a >> v)`), capped |
| 5 | `ext3()`: user-supplied start/end residue indexes unchecked - heap out-of-bounds read; buffer leaked | High | Bounds checked against sequence length; `std::vector` |
| 6 | `p4()`: unchecked reading count -> `new float[n]` with negative/huge n (crash), leaked buffers | Medium | 1..100000 range check; `std::vector` |
| 7 | Predictable temp files in cwd (`.tml`, `.date`, `.t`, `.tmp`) written via shell redirection - symlink/race attacks can clobber files | Medium | Replaced with `getpwuid`, `strftime`, `uname`, in-process PATH lookup - no temp files |
| 8 | Unqualified `system("clear"/"ls"/"ps"/"who"/"pwd")` - PATH hijacking | Medium | ANSI clear, `getcwd`, absolute `/bin/ls`, `/bin/ps`, `/usr/bin/who` via exec |
| 9 | Output files opened blindly - writes through planted symlinks, silent overwrites (incl. `conf.txt`) | Medium | `openOutFile()`: refuses symlinks/non-regular files, confirms overwrite |
| 10 | EOF / bad numeric input left `cin` failed -> 100% CPU infinite loop | Medium | `select()` exits on EOF; `pause()` recovers the stream |
| 11 | `fact()`: recursion depth = user input (stack overflow), float->int UB, silent overflow | Medium | Iterative, input limited to 0..20, `unsigned long long` |
| 12 | `stof()` throws on non-numeric input -> abort (`melt`) | Low | `parseFloat()` with error reporting |
| 13 | Custom `roundf()` clashed with libc `roundf`; `static_cast<int>` overflow UB | Low | Renamed `round2()`, done in `double` with `trunc` |
| 14 | Unbounded recursion: `about()` and `ls` called `select()` | Low | Return to main loop instead |
| 15 | Names written into XPLOR scripts / Vina config unchecked | Low | `isSafeFilename()` whitelist, rejects leading `-` |
| 16 | Sequence-file bytes echoed to terminal (escape-sequence injection) | Low | Non-printables shown as `?` |
| 17 | If installed setuid/setgid, `sys` etc. would grant a privileged shell | Info | Privileges dropped at startup; `sys` disabled when running as root |

Also fixed while there: the formula printed `S(oxygen)` instead of `S(sulphur)`; `overlay` option 3 closed the wrong stream; frames >= 1000 in `rname` reused a stale name; Vina log filename message.

Unchanged by design: `sys` still runs the user's own command through the shell (that is its purpose, with the user's own privileges). `vina`/`rasmol` are located via absolute `PATH` entries.

Build: `make` (adds FORTIFY_SOURCE, stack protector, PIE).
