# mhash
Multithreaded implementation of MinHashing in C using pthreads.

about
-------
Give a database of size `N` and a query vector of dimensionality `d`, the problem of identifying exact or nearly exact matches to the query in the database takes time `Ω(Nd)`. When given another database of `M` such queries, this problem requires `Ω(NMd)` time. For large databases, this problem quickly becomes intractable.

To remedy this quadratic running time, we can employ approximation methods. Locality-Sensitive Hashing (LSH) is one such method. MinHashing is an LSH scheme which defines vector similarity using the Jaccard similarity coefficient. This project provides a multithreaded implementation of MinHashing in C using the pthreads library.

usage
-------
The project is built using the [`make`](http://man7.org/linux/man-pages/man1/make.1.html) utility. Navigate into the project directory and type
```
make
```
to build the project. The following command will identify all pairs of vectors in `test/test1.csv` using a single thread.
```
cat test/test1.csv | ./bin/mhash -b 2 -r 10 -t 1 -E
```

arguments
-------
```
-b    <int>     number of bands per signature (n=r*b)         (required)
-r    <int>     size of each band (# rows)                    (required)
-s    <int>     PRNG seed for reproducibility                 (default: 0)
-t    <int>     number of threads to use                      (default: 1)
-E              enables exclusive comparison mode, prevents
                observations with the same index being paired
-V              enables verbosity, i.e. a bunch of debugging
                garbage will be thrown into standard output
```

other project(s) used
-------
- [Generic makefile](https://gist.github.com/yifanz/7161040), by Yanick Rochon
