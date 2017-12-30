#!/usr/bin/env python3
# # -*- coding: utf-8 -*-

"""

"""

import os
import random
import argparse
import tempfile
import subprocess


""" Globals """
GENSETS = "mh_gensets.py"
MHASH = "../mhash"


""" Parse program arguments """
desc = "Benchmarks mhash timing requirements and memory usage."
parser = argparse.ArgumentParser(description=desc)
parser.add_argument("-I",
                    action="store",
                    type=int,
                    dest="itrs",
                    help="number of iterations to average together",
                    required=True)
parser.add_argument("-N", 
                    action="store",
                    type=int,
                    nargs="+",
                    dest="Ns",
                    help="list of dataset sizes to use when generating datasets",
                    required=True)
parser.add_argument("-J",
                    action="append",
                    type=lambda s: tuple([int(x) for x in s.split(",")]),
                    dest="BRs",
                    help="adds a band:row pair to be used as a setting of mhash",
                    required=True)
parser.add_argument("-F",
                    action="store",
                    type=int,
                    nargs="+",
                    dest="Fs",
                    help="list of feature counts to use when generating datasets",
                    required=True)
parser.add_argument("-P",
                    action="store",
                    type=int,
                    dest="P",
                    help="maximum number of processes to spawn when running mhash",
                    required=True)
parser.add_argument("-s",
                    action="store",
                    type=int,
                    dest="seed",
                    default=None,
                    help="seed for reproducibility",
                    required=False)
parser.add_argument("-o",
                    action="store",
                    type=str,
                    dest="outpath",
                    help="path to output CSV",
                    required=True)
args = parser.parse_args()
itrs = args.itrs
Ns = args.Ns
BRs = args.BRs
Fs = args.Fs
P = args.P
argseed = args.seed
outpath = args.outpath


""" Seed random number generator with argument seed """
random.seed(argseed)


""" Generate all datasets """
print("generating datasets...")
dsets = {(F,N):tempfile.NamedTemporaryFile(mode="rw",delete=False) for F in Fs for N in Ns}
for FN in dsets:
	fname = dsets[FN].name
	dsets[FN].close()
	dsets[FN] = fname
procs = []
for F,N in dsets:
	seed = random.randint(0,2147483648)
	proc = subprocess.Popen(["python", GENSETS, "-f", str(F), "-n", str(N), "-k", "1", "-s", str(seed), "-o", dsets[(F,N)]])
	procs.append(proc)
for proc in procs:
	proc.wait()


""" Run mhash and output results """
print("beginning benchmarking experiments...")
categories = { "actual time":0, "user time":1, "kernel time":2, "memory usage":3 }
with open(outpath, "w") as f:
	seeds = [random.randint(0,2147483648) for i in range(itrs)]
	for F in Fs:
		res = {BR:{N:[0.0]*len(categories) for N in Ns} for BR in BRs}  # Ts[(b,r)][N] = time req in seconds
		
		# Build commands and queue them up to be executed
		pqueue = []
		for N in Ns:
			# Run mhash using all settings of B, R, and seed
			for seed in seeds:
				for BR in BRs:
					B,R = BR
					resfile = tempfile.NamedTemporaryFile(mode="r", delete=False)
					resf = resfile.name
					resfile.close()
					catcmd = ["cat", dsets[(F,N)]]
					proccmd = ["time", "-f", "%e,%U,%S,%M", "-o", resf, MHASH, "-b", str(B), "-r", str(R), "-E", "-s", str(seed), "-t", "1", "--no-print"]
					pqueue.append((catcmd, proccmd, resf, N, BR))
		
		# Run P processes concurrently by pulling them from the queue
		procs = []
		for pq in pqueue:
			catcmd,proccmd,resf,N,BR = pq
			cat = subprocess.Popen(catcmd, stdout=subprocess.PIPE)
			proc = subprocess.Popen(proccmd, stdin=cat.stdout)
			procs.append((proc, resf, N, BR))
			print("{} | {}".format(" ".join(catcmd), " ".join(proccmd)))
			if len(procs) == P or pq == pqueue[-1]:
				for proc,resf,N,BR in procs:
					proc.wait()
					with open(resf, "r") as rf:
						r = [float(x) for x in rf.readline().rstrip().split(",")]
						for c in categories:
							ci = categories[c]
							res[BR][N][ci] += r[ci]
					os.remove(resf)
				procs = []
		
		# Write results to file
		f.write("F,{}\n".format(F))
		for c in categories:
			ci = categories[c]
			f.write(c)
			for B,R in BRs:
				f.write(",{} : {}".format(B,R))
			f.write("\n")
			for N in Ns:
				f.write("{}".format(N))
				for BR in BRs:
					B,R = BR
					f.write(",{}".format(res[BR][N][ci] / float(itrs)))
				f.write("\n")
			f.write("\n")


""" Delete temporary files """
for FN in dsets:
	os.remove(dsets[FN])


""" Done! """
print("done. results can be found at: {}".format(outpath))
