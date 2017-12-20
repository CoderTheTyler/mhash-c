class Observation:
	"""Stores data for a well set loaded from an OFVF."""
	def __init__ (self, m, fs):
		self.m = m
		self.fs = fs
	
	def jaccard (self, other):
		I = float(len(set(other.fs) & set(self.fs)))
		U = float(len(set(other.fs) | set(self.fs)))
		return I / U

class Dataset:
	def __init__ (self, F):
		self.obvs = []
		self.N = 0
		self.M = 0
		self.F = F
	
	def add_observation (self, m, fs):
		"""Adds a WellSet to the specified class."""
		self.obvs.append( Observation(m, fs) )
		self.M += m
		self.N += 1
	
	def save_to_file (self, f):
		"""Saves the data in this OFVF to the given file f."""
		N = len(self.obvs)
		f.write("{},{},{}\n".format(N, self.M, self.F))
		for i in range(0, N):
			obv = self.obvs[i]
			f.write("{}".format(obv.m))
			for j in range(obv.m):
				f.write(",{}".format(obv.fs[j]))
			f.write("\n")
	
	@classmethod
	def from_file (cls, f):
		"""Loads all datasets from the given file, returns list of datasets."""
		dsets = []
		header = f.readline()
		while header:
			N,M,F = [int(x) for x in header.rstrip().split(",")]
			dset = Dataset(F)
			for i in range(N):
				dat = [int(x) for x in f.readline().rstrip().split(",")]
				dset.add_observation(dat[0], tuple(dat[1:]))
			dsets.append(dset)
			header = f.readline()
		return dsets
