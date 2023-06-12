# STV2 - StraTegic Verifier 2 

## Usage

To run:

```
cd build
make clean
make
./stv
```

Configuration file:

```
build/config.txt
```

## Tests

To run tests:

```
cd build
make clean
make sample_test
./sample_test
```

## Performance estimation

Ubuntu/WSL:  

```sh
# Minimal
> /usr/bin/time -f "%M\t%e" ./stv
# %M - maximum resident set size in KB
# %e - elapsed real time (wall clock) in seconds

# More detailed
> /usr/bin/time -f "time result\ncmd:%C\nreal %es\nuser %Us \nsys  %Ss \nmemory:%MKB \ncpu %P" ./stv
# %C   command line and arguments
# %e   elapsed real time (wall clock) in seconds
# %U   user time in seconds
# %S   system (kernel) time in seconds
# %M   maximum resident set size in KB
# %P   percent of CPU this job got

# Full (verbose)
> /usr/bin/time -v ./stv
```

## Misc

With `OUTPUT_DOT_FILES` flag the program outputs *.dot* files for templates, local and global models where:  
* nodes are labelled with its location name (comma-separated for the global state)
* shared transitions are denoted by blue colour

Use Graphviz [(link)](https://www.graphviz.org/) to view in other format (eps, pdf, jpeg, etc.):
```sh
# Analogously for other formats
dot -Tpng lts_of_AGENT.dot > lts_of_AGENT.png
```

For the smaller graphs use *dot2png.sh* script, which converts all *.dot* files from a current folder to *.png*.  
For bigger ones use `svg` format (may be viewed in Inkscape) and *dot2svg.sh*.