#!/usr/bin/env python3
# # -*- coding: utf-8 -*-

"""
Generates K random datasets each with N vectors of dimensionality F. All K 
datasets are written successively to a specified file. If the file already
exists, then these datasets are appended to the end of it.
"""

from sys import argv
import os
import random
from pymhlib import Dataset, Observation
import argparse


""" Parse program arguments """
desc = "Generates random datasets to benchmark mhash."
parser = argparse.ArgumentParser(description=desc)
parser.add_argument("-n", 
                    action="store",
                    type=int,
                    dest="N",
                    help="size of the dataset, i.e. number of vectors",
					required=True)
parser.add_argument("-f", 
                    action="store", 
                    type=int, 
                    dest="F",
                    help="dimensionality of vectors",
					required=True)
parser.add_argument("-k", 
                    action="store", 
                    type=int, 
                    dest="K",
                    help="number of datasets to store in output file",
					required=True)
parser.add_argument("-o", 
                    action="store", 
                    type=str, 
                    dest="outpath",
                    help="path to output file",
					required=True)
parser.add_argument("-s",
                    action="store",
                    type=int,
                    dest="seed",
                    default=None,
                    help="seed for reproducibility",
                    required=False)
args = parser.parse_args()
N = args.N
F = args.F
K = args.K
seed = args.seed
outpath = args.outpath


""" Put K random datasets in an array  """
random.seed(seed)
dsets = [Dataset(F) for x in range(K)]
for k in range(K):
	for i in range(N):
		m = 0
		fs = []
		while m == 0:
			p = random.random()
			for f in range(F):
				if random.random() < p:
					fs.append(f)
					m += 1
		fs = tuple(fs)
		dsets[k].add_observation(m, fs)


""" Save all K datasets to the specified output file """
with open(outpath, "a") as f:
	for k in range(K):
		dsets[k].save_to_file(f)


""" Done! """
print("dataset(s) ready at: " + os.path.abspath(outpath))
