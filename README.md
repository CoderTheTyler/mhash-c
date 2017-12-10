# mhash
Multithreaded implementation of MinHashing in C using pthreads.

about:
-------
Provides an expected linear time algorithm for approximate identification
of similar pairs of bit vectors. Two classes (i.e. groups) of bit vectors
are considered from the standard input. Similar pairs are outputted
to the standard output.

args:
-------
-b    <int>     number of bands per signature (n=r*b)         (required)
-r    <int>     size of each band (# rows)                    (required)
-s    <int>     PRNG seed for reproducibility                 (default: 0)
-t    <int>     number of threads to use                      (default: 1)
-E              enables exclusive comparison mode, prevents
                observations with the same index being paired
-V              enables verbosity, i.e. a bunch of debugging
                garbage will be thrown into standard output
