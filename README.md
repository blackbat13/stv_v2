# STV2 - StraTegic Verifier 2 

## Usage

To run:

```
cd build
make clean
make
./stv
```
or
```
cd build
./build-run
```

Configuration file:

```
build/config.txt
```

CLI configuration overwrite:  
```sh
# Input model
./stv --file PATH_TO_MODEL 
./stv -f PATH_TO_MODEL 
# Mode
./stv -m 0  # 
./stv -m 1  # generate GlobalModel
./stv -m 2  # run verification
./stv -m 3  # same as 1 && 2
# Flags
# --OUTPUT_GLOBAL_MODEL      stdout data on global model (after expandAllStates)
# --OUTPUT_LOCAL_MODELS      stdout data on local models () 
# --OUTPUT_DOT_FILES         generate .dot files for agent templates, local and global models
# --ADD_EPSILON_TRANSITIONS  generate global models with epsilon transitions
# --OVERWRITE_FORMULA        replace the formula from the model file with a different one
# --COUNTEREXAMPLE           output counterexample path if the formula verification returns an ERR
# --REDUCE                   reduce the amount of states and transitions using a DFS-POR algorithm and select the first correct transition
# --REDUCE_ALL               reduce the amount of states and transitions using a DFS-POR algorithm and select all avaliable transitions
# --FIXPOINT                 enables fixpoint approximation
# --NATURAL_STRATEGY         generate a natural strategy for the given model
```

## Tests

To run tests:

```
cd build
make clean
make sample_test
./sample_test
```
or
```
cd build
./build-test
```

To run larger tests:

```
cd build
make clean
make sample_test
./sample_test
```
or
```
cd build
./build-big-test
```

You might need to run
```
ulimit -s unlimited
```
beforehand

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

## Specification

The specification language was inspired by ISPL (Interpreted Systems Programming Language) from [MCMAS](https://vas.doc.ic.ac.uk/software/mcmas/).   
The detailed syntax for the input format can be derived from *./src/reader/{parser.y,scanner.l}*, which intrinsically make up an EBNF grammar.  
For the most parts, it is simple enough to get intuition just from looking at example's source code and the program's output.

IMPORTANTS NOTES:
1.  the (local) action names must be unique;
2.  the transition relation (from the global model) should be serial;


## Examples and templates

In *./examples* and *./tests/examples* there are several ready-to-use MAS specification files together with a proposed property (captured by ATL formula) for verification.

Often, we would want to reason about different (data-)configurations of the same system.  
Using the templates we can parameterize the system specification, such that we only need to describe its dynamic behaviour.  
A template can be fed with a configuration data to generate a concrete instance of a system.  
Moreover, their use is independent from the tool: one can choose any templating engine (of the myriads available) or even write a custom one from the scratch.

Here, we utilize the [EJS](https://ejs.co/) templating engine.   
It has a CLI support, which is comes in handy for the tests/benchmarks that involve systems in multiple configurations. 

```sh
# EJS feeds the data (as a list of key:val pairs) to the template file to generate the output:
> npm exec -- ejs TEMPLATE_FILE.ejs -i "{PARAM1:VAL1,PARAM2:VAL2,...}" -o OUTPUT_FILE.txt


# Possible generation query for the "trains":
> npm exec -- ejs Trains.ejs -i '{"N_TRAINS":3,"WITH_FORMULA":1}' -o 3Trains1Controller.txt

# Possible generation query for the "simple voting":
> npm exec -- ejs Simple_voting.ejs -i '{"N_VOTERS":2,"N_CANDIDATES":1,"WITH_FORMULA":0}' -o 2Voters1Coercer1Candidate.txt
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