# Tomasulo Algorithm Simulator

A C++ implementation of Tomasulo's algorithm for dynamic instruction scheduling in superscalar processors.

## 📋 Overview

This project simulates the Tomasulo algorithm, a hardware-based dynamic scheduling technique that enables out-of-order execution while preserving data dependencies. Originally developed by Robert Tomasulo in 1967 for the IBM System/360 Model 91, this algorithm remains fundamental to modern processor design.

## ✨ Features

- **Complete Tomasulo Implementation**: Full simulation of reservation stations, register renaming, and common data bus
- **Cycle-by-Cycle Execution**: Step through the simulation one clock cycle at a time
- **Comprehensive Visualization**: Display of reservation stations, register file status, and instruction pipeline
- **Flexible Configuration**: Adjustable number of reservation stations and operation latencies
- **Custom Instruction Sets**: Support for ADD, SUB, MUL, DIV, LOAD, and STORE operations
- **Performance Metrics**: IPC (Instructions Per Cycle) calculation and execution statistics

## 🏗️ Architecture

The simulator implements three main stages of the Tomasulo algorithm:

1. **Issue**: Instructions are dispatched to available reservation stations
2. **Execute**: Operations execute when operands are ready
3. **Write Result**: Results are broadcast via the Common Data Bus (CDB)

## 🚀 Getting Started

### Prerequisites

- C++ compiler with C++11 support or higher (g++, clang++)
- Make (optional, for using Makefile)

### Compilation
```bash
# Using Make
make

# Or directly with g++
g++ -std=c++11 src/*.cpp -o tomasulo_simulator
```

### Running
```bash
./tomasulo_simulator <input_file.txt>
```

### Input File Format
```
ADD R1, R2, R3
MUL R4, R1, R5
LOAD R6, 100
STORE R4, 200
```

## 📊 Output

The simulator displays cycle-by-cycle information including:

- Current cycle number
- Reservation station status (Busy, Op, Vj, Vk, Qj, Qk)
- Register file state (Value, Qi)
- Instruction completion timeline
- Final register values
- Performance statistics

## 🎓 Educational Purpose

This project was developed as part of the Computer Architecture course at [Your University]. It demonstrates understanding of:

- Instruction-level parallelism (ILP)
- Dynamic scheduling techniques
- Register renaming and hazard elimination
- Superscalar processor architecture

## 📚 References

- Hennessy, J. L., & Patterson, D. A. *Computer Architecture: A Quantitative Approach*
- Tomasulo, R. M. (1967). "An Efficient Algorithm for Exploiting Multiple Arithmetic Units"

## 🤝 Contributing

This is an academic project, but suggestions and improvements are welcome! Feel free to:

- Report bugs
- Suggest enhancements
- Submit pull requests

## 📝 License

This project is available under the MIT License - see LICENSE file for details.

## 👤 Author

[Seu Nome]
- GitHub: [@seu-usuario](https://github.com/seu-usuario)
- LinkedIn: [Seu Perfil](https://linkedin.com/in/seu-perfil)

## 🙏 Acknowledgments

- Professor [Nome do Professor] for guidance and instruction
- IBM for the original Tomasulo algorithm design
- Course materials and references from Computer Architecture class
```

---

### Versão Curta (para a descrição do repositório no GitHub)

**Campo "About" do GitHub (160 caracteres max)**:
```
C++ simulator implementing Tomasulo's algorithm for dynamic instruction scheduling in superscalar processors. Educational project.
```

**Ou mais curto ainda**:
```
Tomasulo algorithm simulator in C++ - Out-of-order execution with register renaming
```

**Ou focado em aprendizado**:
```
Computer Architecture project: Tomasulo's algorithm implementation for superscalar processor simulation
```

---

## 3. Tags/Topics Sugeridas

Adicione essas tags ao repositório (aparecem na busca do GitHub):
```
tomasulo
computer-architecture
superscalar
out-of-order-execution
instruction-scheduling
cpu-simulator
register-renaming
cpp
educational
academic-project
processor-simulation
ilp
dynamic-scheduling
