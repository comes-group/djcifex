# DJ Cifex - the superior CIF de/encoder

This is The New™ reference [CIF][cif spec] encoding and decoding library, written in C.

Decoding is a lot faster than [cif.nim][nim implementation]:
```
$ hyperfine --warmup 5 \
      './build/src/cifex-cli/cifex decode ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dry-run' \
      'decif ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dryRun'

Benchmark 1: ./build/src/cifex-cli/cifex decode ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dry-run
  Time (mean ± σ):     126.1 ms ±   3.4 ms    [User: 76.7 ms, System: 49.1 ms]
  Range (min … max):   124.5 ms … 141.4 ms    23 runs

Benchmark 2: decif ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dryRun
  Time (mean ± σ):     328.3 ms ±   0.5 ms    [User: 205.5 ms, System: 122.3 ms]
  Range (min … max):   327.5 ms … 329.1 ms    10 runs

Summary
  './build/src/cifex-cli/cifex decode ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dry-run' ran
    2.60 ± 0.07 times faster than 'decif ~/Repositories/cif-tests/valid/test-image.large.cif /tmp/benchmark.png --dryRun'
```
Encoding absolutely destroys cif.nim, because its implementation was very inefficient with a lot of dynamic dispatch.
```
$ hyperfine --warmup 5 \
      './build/src/cifex-cli/cifex encode ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif --dry-run' \
      'encif ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif'

Benchmark 1: ./build/src/cifex-cli/cifex encode ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif --dry-run
  Time (mean ± σ):     215.8 ms ±   0.5 ms    [User: 156.5 ms, System: 58.7 ms]
  Range (min … max):   214.8 ms … 216.7 ms    13 runs

Benchmark 2: encif ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif
  Time (mean ± σ):     839.1 ms ±  10.9 ms    [User: 774.3 ms, System: 63.4 ms]
  Range (min … max):   824.8 ms … 860.1 ms    10 runs

Summary
  './build/src/cifex-cli/cifex encode ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif --dry-run' ran
    3.89 ± 0.05 times faster than 'encif ~/Repositories/cif-tests/valid/test-image.large.png /tmp/benchmark.cif'
```

  [cif spec]: https://raw.githubusercontent.com/comes-group/standards/master/english/cif.rst
  [nim implementation]: https://github.com/comes-group/cif

## Compiling

```
$ meson setup build -Dbuildtype=release
$ ninja -C build
```
