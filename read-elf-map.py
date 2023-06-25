#!/usr/bin/env python3
from collections import defaultdict

MEM_TOTAL_BYTES = 264 * 1024

PATH = "pico-radio.elf.map"
MIN_MEM = 500
PATH_LAST = 4

def process(line, header, D):
	size = 0
	fname = ""
	line = line.split()
	
	if header:
		if len(line) >= 3:
			size = int(line[2], 16)
			#print(line, size)
		if len(line) >= 4:
			fname = line[3]
	else:
		if len(line) >= 2:
			try:
				size = int(line[1], 16)
			except ValueError:
				pass
		if len(line) >= 3:
			fname = line[2]
	
	if len(fname) > 0:
		D[fname] += size
		
	return size

bss = False
discarded = False
valid = False

S = 0
D = defaultdict(lambda: 0)

for line in open(PATH):
	line = line.rstrip()
	ls = line.strip()
		
	if ls == "Discarded input sections":
		discarded = True
	elif ls == "Linker script and memory map":
		valid = True
	elif len(line) > 0 and not line.startswith(" ") and not line.startswith("."):
		discarded = False
		valid = False
	
	if valid:
		if line.startswith(" .bss"):
			S += process(line, True, D)
			bss = True
		elif ls.startswith(".") or ls.startswith("*"):
			# other section
			bss = False
		elif bss:
			S += process(line, False, D)

print()
Ds = sum(D.values())
if S != Ds:
	print(f"S diffrent from sum(D): {S-Ds}")

print("Most memory-intensive files:")
for key in sorted(D.keys(), key=lambda x: D[x], reverse=True):
	if D[key] > MIN_MEM:
		name = "/".join(key.split("/")[-PATH_LAST:])
		print(f"{D[key]:10} {name}")

print()
print(f"RAM consumed (static): {S / 1024:6.2f}kB / {MEM_TOTAL_BYTES // 1024}kB:  {S/MEM_TOTAL_BYTES*100:6.2f}%")
