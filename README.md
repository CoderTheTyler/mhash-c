# mhash-c

**[about](#about) | [usage](#usage) | [input](#input) | [arguments](#arguments) | [benchmarks](#benchmarks) | [resources](#resources)**

![empirical-f480-n4000.png](http://tylerdaddio.com/imgs/empirical-f480-n4000.png)


about
-------
Given a database of size `N` and a query vector of dimensionality `d`, the problem of identifying exact or nearly exact matches to the query in the database takes time `Ω(Nd)`. When given another database of `M` such queries, this problem requires `Ω(NMd)` time. For large databases, this problem quickly becomes intractable.

To remedy this, we can employ approximation methods. [Locality-Sensitive Hashing](https://en.wikipedia.org/wiki/Locality-sensitive_hashing) (LSH) is one such method. [MinHashing](http://gatekeeper.dec.com/ftp/pub/dec/SRC/publications/broder/positano-final-wpnums.pdf) is an LSH scheme which defines vector similarity using the [Jaccard similarity coefficient](https://en.wikipedia.org/wiki/Jaccard_index). This project provides a multithreaded implementation of MinHashing in C using the [pthreads](https://computing.llnl.gov/tutorials/pthreads/) library. A great explanation both of LSH and of MinHashing (as well as numerous other fantastic topics) can be found in the amazing text Mining of Massive Datasets<sup>[[1]](#resources)</sup>.


usage
-------
The project is built using the [`make`](http://man7.org/linux/man-pages/man1/make.1.html) utility. Navigate into the project directory and type
```
make
```
to build the project. The following command will identify all pairs of similar vectors in `bench/test.csv` using a single thread.
```
cat bench/test.csv | ./mhash -b 2 -r 10 -t 1 -E
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


arguments
-------
```
-b    <int>     number of bands per signature (n=r*b)         (required)
-r    <int>     size of each band (# rows)                    (required)
-s    <int>     seed for reproducibility                      (default: 0)
-t    <int>     number of threads to use                      (default: 1)
-E              enables exclusive comparison mode, prevents
                observations with the same index being paired
--verbose       enables verbosity, i.e. a bunch of debugging
                garbage will be thrown into standard output
--no-print      pairing results will not be sent to stdout
```


benchmarks
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

We have also investigated timing requirements and peak memory usage of *mhash* over various settings and inputs. These are provided below. All memory results are provided in kilobytes (kB) and all timing results are provided in seconds. All benchmarking experiments were run on an Intel(R) Xeon(R) CPU E7-4850 v4 @ 2.10GHz with 1.5 TB of RAM using 64 "threads" of execution. 

We used the `--no-print` flag when running *mhash* to suppress outputting identified pairs. The running time will be heavily influenced by the number of pairs *mhash* identifies and subsequently outputs, which can be quadratic in the size of the dataset in the worse case. The ouput quickly becomes the bottleneck when more than a number of pairs linear in the size of the input are identified. 

| ![memory_usage-f96.png](http://tylerdaddio.com/imgs/memory_usage-f96.png) | ![memory_usage-f480.png](http://tylerdaddio.com/imgs/memory_usage-f480.png) |
| ------------- | ------------- |
| **_F = 96_** | **_F = 480_** |

| ![time_actual-f96.png](http://tylerdaddio.com/imgs/time_actual-f96.png) | ![time_actual-f480.png](http://tylerdaddio.com/imgs/time_actual-f480.png) |
| ------------- | ------------- |
| **_F = 96_** | **_F = 480_** |


We have provided the necessary tools to repeat this analysis [here](https://github.com/CoderTheTyler/mhash-c/tree/master/bench). Raw data for the above graphs can be found [here](http://tylerdaddio.com/imgs/mhash-c_benchmarks.ods).


resources
-------
[1] [Mining of Massive Datasets](http://www.mmds.org/), J. Leskovec et al.
