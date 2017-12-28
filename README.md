# mhash-c
multithreaded implementation of MinHashing; written in C using pthreads.


about
-------
Given a database of size `N` and a query vector of dimensionality `d`, the problem of identifying exact or nearly exact matches to the query in the database takes time `Ω(Nd)`. When given another database of `M` such queries, this problem requires `Ω(NMd)` time. For large databases, this problem quickly becomes intractable.

To remedy this, we can employ approximation methods. [Locality-Sensitive Hashing](https://en.wikipedia.org/wiki/Locality-sensitive_hashing) (LSH) is one such method. [MinHashing](http://gatekeeper.dec.com/ftp/pub/dec/SRC/publications/broder/positano-final-wpnums.pdf) is an LSH scheme which defines vector similarity using the [Jaccard similarity coefficient](https://en.wikipedia.org/wiki/Jaccard_index). This project provides a multithreaded implementation of MinHashing in C using the [pthreads](https://computing.llnl.gov/tutorials/pthreads/) library. A great explanation both of LSH and of MinHashing (as well as numerous other fantastic topics) can be found in the amazing text Mining of Massive Datasets<sup>[[1]](#resources)</sup>.. 


performance
-------
The expected probability of any two vectors with similarity `s` being paired when using `b` bands and `r` rows is <code>1-(1-s<sup>r</sup>)<sup>b</sup></code><sup>[[1]](#resources)</sup>. A successful implementation must provide this theoretical guarantee. We verify this is the case with our implementation as follows:

1. Generate a dataset `D` by sampling from the space of all possible vectors uniformly at random.
2. Run *mhash* on `D` with various settings of `b` and `r`.
3. Compute the proportion of called pairs at each similarity and place pairs into buckets (this is done because the distribution of similarities is discrete).
4. Graph both the theoretical curve and this empirical curve. The latter curve should approximate the former.

One would expect a larger dataset with higher dimensional vectors to yield a better approximation to the theoretical curve for all settings of `b` and `r`. The graphs below seem to support this intuition. `(T)` denotes a theoretical curve computed using the above formula.

| ![empirical-f96-n1000.png](http://tylerdaddio.com/imgs/empirical-f96-n1000.png) | ![empirical-f480-n4000.png](http://tylerdaddio.com/imgs/empirical-f480-n4000.png) |
| ------------- | ------------- |
| **_F = 96, N = 1000_** | **_F = 480, N = 4000_** |

We have provided the necessary tools to repeat this analysis [here](https://github.com/CoderTheTyler/mhash-c/tree/master/test).


usage
-------
The project is built using the [`make`](http://man7.org/linux/man-pages/man1/make.1.html) utility. Navigate into the project directory and type
```
make
```
to build the project. The following command will identify all pairs of similar vectors in `test/test1.csv` using a single thread.
```
cat test/test.csv | ./mhash -b 2 -r 10 -t 1 -E
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


usage
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


resources
-------
[1] [Mining of Massive Datasets](http://www.mmds.org/), J. Leskovec et al.
