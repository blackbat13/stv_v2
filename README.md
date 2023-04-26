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

## Misc

With `OUTPUT_DOT_FILES` flag the program outputs .dot files for each agent's local model (as LTS), where:  
* states are labelled with its location name
* shared transitions are denoted by blue colour

Use Graphviz [(link)](https://www.graphviz.org/) to view in other format (eps, pdf, jpeg, etc.):
```sh
# Analogously for other formats
dot -Tpng lts_of_AGENT.dot > lts_of_AGENT.png
```