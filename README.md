# mhash
Multithreaded implementation of MinHashing in C using pthreads.

about
-------
Give a database of size `N` and a query vector of dimensionality `d`, the problem of identifying exact or nearly exact matches to the query in the database takes time `Ω(Nd)`. When given another database of `M` such queries, this problem requires `Ω(NMd)` time. For large databases, this problem quickly becomes intractable.

To remedy this quadratic running time, we can employ approximation methods. [Locality-Sensitive Hashing](https://en.wikipedia.org/wiki/Locality-sensitive_hashing) (LSH) is one such method. [MinHashing](http://gatekeeper.dec.com/ftp/pub/dec/SRC/publications/broder/positano-final-wpnums.pdf) is an LSH scheme which defines vector similarity using the [Jaccard similarity coefficient](https://en.wikipedia.org/wiki/Jaccard_index). This project provides a multithreaded implementation of MinHashing in C using the [pthreads](https://computing.llnl.gov/tutorials/pthreads/) library. A great explanation both of LSH and of MinHashing (as well as numerous other fantastic topics) can be found in the amazing text [Mining of Massive Datasets](http://www.mmds.org/). 

usage
-------
The project is built using the [`make`](http://man7.org/linux/man-pages/man1/make.1.html) utility. Navigate into the project directory and type
```
make
```
to build the project. The following command will identify all pairs of similar vectors in `test/test1.csv` using a single thread.
```
cat test/test1.csv | ./bin/mhash -b 2 -r 10 -t 1 -E
```

input
-------
The program takes one or two databases of vectors as input. Consider the following format:
<pre>
N,D,d
m1,f<sub>1</sub>,f<sub>2</sub>,...,f<sub>m1</sub>
m2,f<sub>m1+1</sub>,f<sub>m2+2</sub>,...,f<sub>m1+m2</sub>
...
mN,f<sub>D-mN+1</sub>,f<sub>D-mN+2</sub>,...,f<sub>D</sub>
</pre>
where `N` is the number of vectors in the database, `D` is the total number of features in all `N` vectors, and `d` is the dimensionality, i.e. # of possible features, of each vector. A single such database can be provided and compared to itself using the `-E` option. If another, distinct database is to be provided, it should be provided via standard input immediately after the first.

For example, `test/test1.csv` is a single database with five vectors each having 10 dimensions/features and a total of 23 features expressed across the 5 vectors (thus the header `5,23,10`). The following five lines contains describe the vectors; the first expresses four features (`0,1,2,3`), the second expresses three features (`5,7,9`), etc. The features **must** be provided in strictly increasing order. 

arguments
-------
```
-b    <int>     number of bands per signature (n=r*b)         (required)
-r    <int>     size of each band (# rows)                    (required)
-s    <int>     seed for reproducibility                      (default: 0)
-t    <int>     number of threads to use                      (default: 1)
-E              enables exclusive comparison mode, prevents
                observations with the same index being paired
-V              enables verbosity, i.e. a bunch of debugging
                garbage will be thrown into standard output
```

sources
-------
- [Mining of Massive Datasets](http://www.mmds.org/), J. Leskovec et al.
- [Generic makefile](https://gist.github.com/yifanz/7161040), Yanick Rochon
