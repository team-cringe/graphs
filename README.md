# Finite Graph Theory and its Applications

Semester project for the _Graph Theory_ course in _St. Petersburg State University_.

## Description

Using data from the nonprofit mapping project _OpenStreetMap_, build a routing graph of one of Russian megalopolises.

For our project we have chosen the city of _[Nizhny Novgorod](https://pbs.twimg.com/media/BG48ENgCEAAIDl9.jpg)_.

_M_ random infrastructure facilities (_e.g., hospitals_) and _N_ random houses in the chosen city are selected.

Each task should have a description in either `assessment.cpp` or `planning.cpp`.

## Dependencies

Dynamic libraries:

* [osmcode/libosmium](https://github.com/osmcode/libosmium) 
* [mapbox/protozero](https://github.com/mapbox/protozero)

Static libraries:

* [p-ranav/argparse](https://github.com/p-ranav/argparse)
* [nlohmann/json](https://github.com/nlohmann/json)

## Build

```bash
$ mkdir build; cd build
$ cmake -Release ..
$ make
```

With _Release_ build type, maximum optimization level is set.

Successfully built with _Clang 10_ and _GCC 10_ under _Linux_.

## Usage

Default launch:

```bash
$ graphs 15 30
```

File with map could be specified explicitly (_default is NNMap.pbf_):

```bash
$ graphs 15 30 --file NNMap.pbf
```

The first and the second arguments denote number of houses and facilities accordingly.

With the first launch, map is created (_approx. 3 minutes_). With succeeding launches, cache is used (_saved at .cache_).
Map could be re-cached (_e.g., if different map is used_):

```bash
$ graphs 15 30 --recache
```

