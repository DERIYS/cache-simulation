# Cache Simulation in SystemC

## Overview

This project was developed as the final assignment for TUM's **Fundamentals of Computer Architecture** course.  
It implements a **multi-level cache simulator** in **SystemC**, capable of simulating different cache configurations, mapping strategies, and performance metrics.  
The simulator processes memory access requests from CSV files, handles cache hits and misses, and retrieves data from a simulated main memory when necessary.

---
![alt text](image.png)

---

## Features

- **Multi-level cache hierarchy** (L1, L2, L3) with configurable size, line size, and latency
- **Mapping strategies**:
    - *Direct-Mapped*
    - *Fully Associative*
- **Replacement strategy**: *Least Recently Used (LRU)*
- **Performance analysis**: hit rate, cycle count, and cache miss statistics
- **CLI GUI** for configuration, testing, and debugging
- **CSV-based input** for repeatable simulations

---

## Project Structure

- **`cache.hpp`** – Models the multi-level cache hierarchy, manages timing, and synchronizes all modules.
- **`cache_layer.hpp`** – Implements a single cache level with *Direct-Mapped* and *Fully Associative* mapping, LRU replacement, and STL containers for fast lookups.
- **`main_memory.hpp`** – Simulates main memory, returning entire cache lines to improve spatial locality.
- **`multiplexer.hpp`** – Handles signal distribution between cache levels.
- **`main.c`** – C framework for running the simulation, parsing CLI arguments, and processing CSV requests.

---

## Performance Analysis

Using a generated memory access trace from a **30×30 matrix multiplication**:

- **Requests**: 58,500 total (3,600 writes, 54,900 reads)
- **Hit Rate**: 99.7%
- **Performance**: 539% faster than without cache

---

## How to Build & Run
The simulator requires an .csv file contatining memory requests in the following format:
| Type | Address  | Data |
|------|----------|------|
| W    | 0x0010   | 20   |
| R    | 0x0010   |      |
| W    | 123      | 0x12 |
||...|


There is an example `requests.csv` file in the repository's root that simulates a 10x10 matrix multiplication memory access trace.
### 1. Prerequisites
- [SystemC](https://www.accellera.org/downloads/standards/systemc) installed on your system
- The environment variable `SYSTEMC_HOME` **must** be set to your SystemC installation path  
  Example:
  ```bash
  export SYSTEMC_HOME=/path/to/systemc
1. **Build & run the project**
    ```bash
   make run
2. **Check CLI options**
    ```bash
   ./project -h
3. **To run with CLI GUI** (optional)
   ```bash
   cd app && mkdir build && cd build && cmake .. && make && ./cache_application
---

## Contributors

- **[Safie Emiramzaieva](https://github.com/safie-e)** – Main cache module, inter-module communication, integration, testing

- **[Lev Franko](https://github.com/DERIYS)** – Cache layer logic, search algorithms, LRU replacement, inter-module communication, stress testing

- **[Roman Kupar](https://github.com/roman-kupar)** – CSV parsing, CLI framework, CLI GUI, automated testing, C/C++ integration

## License

This project was developed as part of the **Grundlagenpraktikum: Rechnerarchitektur (IN0005)** course at the Technical University of Munich (TUM).

It is released under the **Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License (CC BY-NC-SA 4.0)**.

You are free to:
- **Share** — copy and redistribute the material in any medium or format
- **Adapt** — remix, transform, and build upon the material

Under the following terms:
- **Attribution** — You must give appropriate credit to the original authors and source.
- **NonCommercial** — You may not use the material for commercial purposes.
- **ShareAlike** — If you remix, transform, or build upon the material, you must distribute your contributions under the same license.

For full legal details, see the [Creative Commons license text](https://creativecommons.org/licenses/by-nc-sa/4.0/).
