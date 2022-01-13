# DJ Cifex - the superior CIF de/encoder

This is The New™ reference [CIF][cif spec] encoding and decoding library, written in C.

Decoding is already quite a bit faster than [cif.nim][nim implementation]:
```
$ hyperfine --warmup 5 './build/src/cifex-cli/cifex images/test.cif /tmp/test.cif --dry-run' 'decif images/test.cif /tmp/test.cif --dryRun'
Benchmark 1: ./build/src/cifex-cli/cifex images/test.cif /tmp/test.cif --dry-run
  Time (mean ± σ):     193.5 ms ±   0.9 ms    [User: 148.5 ms, System: 44.7 ms]
  Range (min … max):   191.9 ms … 195.3 ms    15 runs

Benchmark 2: decif images/test.cif /tmp/test.cif --dryRun
  Time (mean ± σ):     254.6 ms ±   2.6 ms    [User: 160.3 ms, System: 93.9 ms]
  Range (min … max):   252.1 ms … 261.5 ms    11 runs

Summary
  './build/src/cifex-cli/cifex images/test.cif /tmp/test.cif --dry-run' ran
    1.32 ± 0.01 times faster than 'decif images/test.cif /tmp/test.cif --dryRun'
```
Encoding is not implemented yet.

  [cif spec]: https://raw.githubusercontent.com/comes-group/standards/master/english/cif.rst
  [nim implementation]: https://github.com/comes-group/cif

## Compiling

```
$ meson setup build -Dbuildtype=release
$ ninja -C build
```
