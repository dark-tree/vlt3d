#!/usr/bin/env python3

# import stuff
import os
import argparse
import shutil

# parse cl args
parser = argparse.ArgumentParser(description="GLPH Builder")
parser.add_argument("--clean", help="(re-)generate build system", action="store_true")
parser.add_argument("--run", help="execute given target", nargs='?', default=None, const="main")
parser.add_argument("--debug", help="build in debug mode", action="store_true")
parser.add_argument("--list", help="list available targets", action="store_true")
args = parser.parse_args();

# used for clean logging
def write(msg):
	print(f"\n{msg}")

# list available targets when requesed
if args.list:
	print("Available targets:")
	path = "./demo"

	exclude_files = lambda x : not os.path.isfile(os.path.join(path,x))

	for file in filter(exclude_files, os.listdir(path)):
		print( " * --run=" + file )

	exit()

# add cmake debug mode flag when requesed
options = ("-DCMAKE_BUILD_TYPE=Debug " if args.debug else "")

# set system specific constants
if os.name == "nt":
	main = "main.exe"
	make = "nmake"
	options += "-G \"NMake Makefiles\" "
else:
	main = "main"
	make = "make -j ${nproc}"

# remove buid dir when requesed
if args.clean:
	try:
		shutil.rmtree("build")
	except:
		pass

# call cmake
if not os.path.isdir("build"):
	write("Preparing Target...")

	if not os.path.exists("build"):
		os.mkdir("build")

	os.system(f"cd build && cmake {options} ../")

write("Building Target...")

# remove old binary
try:
	if args.run is not None:
		os.remove(f"build/demo/{args.run}/{main}")
except:
	pass

# build project
os.system(f"cd build && {make}")

# run project when required
if args.run is not None:
	os.chdir(f"build/demo/{args.run}/")

	print(f"Executable: '{os.getcwd()}/{main}'")
	write("Running Target...")
	os.system(f"./{main}")

