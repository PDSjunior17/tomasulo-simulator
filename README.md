# Tomasulo Algorithm Simulator

![C++](https://img.shields.io/badge/C++-11-blue)
![License](https://img.shields.io/badge/license-MIT-green)

> Simulador completo do algoritmo de Tomasulo para escalonamento dinâmico de instruções em processadores superescalares.

## 📋 Sobre o Projeto

Este projeto implementa o algoritmo de Tomasulo, uma técnica de escalonamento dinâmico desenvolvida por Robert Tomasulo em 1967 para o IBM System/360 Model 91. O simulador permite visualizar o funcionamento de execução fora de ordem, renomeação de registradores e resolução de dependências de dados em tempo de execução.

### Principais Características

- ✅ **Execução Fora de Ordem (Out-of-Order Execution)**: Instruções executam assim que seus operandos estão disponíveis
- ✅ **Renomeação Dinâmica de Registradores**: Elimina dependências falsas (WAR e WAW)
- ✅ **Buffer de Reordenação (ROB)**: Garante commits em ordem de programa
- ✅ **Estações de Reserva**: Buffers dedicados para cada tipo de unidade funcional
- ✅ **Common Data Bus (CDB)**: Broadcast de resultados para todas as unidades
- ✅ **Configuração Flexível**: Latências e número de unidades ajustáveis
- ✅ **Visualização Ciclo-a-Ciclo**: Acompanhe o estado completo do simulador

## 🏗️ Arquitetura

O simulador implementa as três fases principais do algoritmo de Tomasulo:

### 1. Issue (Despacho)
- Instruções são despachadas para estações de reserva disponíveis
- Renomeação de registradores acontece nesta fase
- Stall se não houver RS livre ou ROB cheio

### 2. Execute (Execução)
- Operações executam quando todos os operandos estão prontos
- Respeita latências configuráveis para cada tipo de operação
- Múltiplas instruções podem executar em paralelo

### 3. Write Result (Escrita)
- Resultados são transmitidos via CDB
- Todas as unidades esperando por aquele resultado são notificadas
- Estações de reserva são liberadas

### 4. Commit (Confirmação)
- Instruções fazem commit em ordem de programa
- Escritas em registradores/memória acontecem apenas aqui
- ROB entry é liberada

## 🚀 Compilação e Execução

### Pré-requisitos

- Compilador C++ com suporte a C++11 ou superior (g++, clang++)
- Sistema operacional: Linux, macOS ou Windows

### Compilação

```bash
# Compilar o simulador
g++ -std=c++11 Tomasulo_saidaArquivo.cpp -o tomasulo_simulator

# Ou use o Makefile (se disponível)
make
```

### Execução

```bash
# Sintaxe
./tomasulo_simulator <arquivo_entrada.txt> <arquivo_saida.txt>

# Exemplo
./tomasulo_simulator input.txt output.txt
```

A saída será salva no arquivo especificado e uma mensagem será exibida no console.

## 📝 Formato do Arquivo de Entrada

O arquivo de entrada possui duas seções principais:

### 1. Configuração (CONFIG_BEGIN...CONFIG_END)

Define latências de operações e número de unidades funcionais:

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES SUBD 2
CYCLES MULTD 4
CYCLES DIVD 10
CYCLES LD 2
CYCLES SD 2

UNITS ADDD 1
UNITS MULTD 1
UNITS DIVD 1

MEM_UNITS LD 1
MEM_UNITS SD 1

CONFIG_END
```

**Parâmetros:**
- `CYCLES <OPERACAO> <VALOR>`: Latência em ciclos para cada tipo de operação
- `UNITS <OPERACAO> <VALOR>`: Número de estações de reserva para operações aritméticas
- `MEM_UNITS <OPERACAO> <VALOR>`: Número de buffers para operações de memória

### 2. Instruções (INSTRUCTIONS_BEGIN...INSTRUCTIONS_END)

Define o programa a ser simulado:

```
INSTRUCTIONS_BEGIN
ADDD F8 F4 F6    # F8 = F4 + F6
MULTD F10 F8 F8  # F10 = F8 * F8
SUBD F12 F10 F4  # F12 = F10 - F4
INSTRUCTIONS_END
```

**Operações Suportadas:**
- `ADDD Fd, Fs1, Fs2`: Soma de ponto flutuante
- `SUBD Fd, Fs1, Fs2`: Subtração de ponto flutuante
- `MULTD Fd, Fs1, Fs2`: Multiplicação de ponto flutuante
- `DIVD Fd, Fs1, Fs2`: Divisão de ponto flutuante
- `LD Fd, offset(Rs)`: Load da memória
- `SD Fs, offset(Rd)`: Store na memória

**Observações:**
- Registradores são nomeados como `F0-F31` (ponto flutuante) ou `R0-R31` (inteiros)
- Comentários iniciam com `#`
- Linhas em branco são ignoradas

## 📊 Formato da Saída

A saída mostra o estado do simulador a cada ciclo:

```
--- INICIANDO CICLO 4 ---
 > EXECUTED: ADD.D (Tag: 1) - Resultado (12.00) pronto.
 > WRITE RESULT: Tag 1 valor (12.00) no CDB.
  > ISSUED: SUB.D (Dest ROB Tag: 3)

==================================================
CICLO 4
==================================================
--- ESTAÇÕES DE RESERVA ---
ID  Ocupado   Op       Qj     Qk     Vj     Vk     Dest   Ciclos
A1  SIM       SUB.D    2      -      0.00   2.00   3      0
M1  SIM       MUL.D    -      -      12.00  12.00  2      0

--- BUFFER DE REORDENAÇÃO (ROB) ---
ID  Ocupado   Estado        Destino Valor Endereço
1   SIM       Pronto        F8      12.00
2   SIM       Issue         F10     0.00
3   SIM       Issue         F12     0.00

--- STATUS DOS REGISTRADORES (Tags do ROB) ---
Reg  Tag
F8   1
F10  2
F12  3
```

### Interpretação da Saída

**Estações de Reserva:**
- **ID**: Identificador da estação (A=ADD/SUB, M=MUL/DIV, L=LOAD, S=STORE)
- **Ocupado**: Se a estação está em uso
- **Op**: Operação sendo executada
- **Qj/Qk**: Tags das instruções que produzirão os operandos (0 = pronto)
- **Vj/Vk**: Valores dos operandos
- **Dest**: Tag do ROB de destino
- **Ciclos**: Ciclos restantes de execução

**Buffer de Reordenação (ROB):**
- **ID**: Número da entrada no ROB (usado como tag)
- **Ocupado**: Se a entrada está em uso
- **Estado**: Issue, Executando, Pronto, ou Commit
- **Destino**: Registrador que receberá o valor (ou Mem para stores)
- **Valor**: Resultado da operação

**Status dos Registradores:**
- Mostra quais registradores estão esperando valores
- **Tag**: Número do ROB que produzirá o valor

## 🎓 Conceitos Implementados

### Renomeação de Registradores

O simulador implementa renomeação implícita usando as entradas do ROB como "registradores temporários":

```
1. ADD.D F1, F2, F3    # F1.Tag = ROB[1]
2. MUL.D F4, F1, F5    # F4 espera ROB[1], não F1 diretamente
3. SUB.D F1, F6, F7    # F1.Tag = ROB[3] (nova renomeação)
```

Instrução 2 continua "ouvindo" ROB[1], sem conflito com a instrução 3.

### Resolução de Dependências

- **RAW (Read After Write)**: Resolvida via CDB e tags
- **WAR (Write After Read)**: Eliminada pela renomeação
- **WAW (Write After Write)**: Eliminada pela renomeação

### Common Data Bus (CDB)

- Apenas uma transmissão por ciclo (recurso limitado)
- Broadcast simultâneo para todas as unidades
- Atualiza estações de reserva e ROB

## 📈 Exemplos de Uso

### Exemplo 1: Programa Simples

**Entrada (input_simple.txt):**
```
CONFIG_BEGIN
CYCLES ADDD 2
CYCLES MULTD 4
UNITS ADDD 1
UNITS MULTD 1
CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F8 F4 F6
MULTD F10 F8 F8
SUBD F12 F10 F4
INSTRUCTIONS_END
```

**Valores Iniciais Assumidos:**
- F4 = 2.0
- F6 = 10.0

**Resultados Esperados:**
- F8 = 12.0 (2 + 10)
- F10 = 144.0 (12 × 12)
- F12 = 142.0 (144 - 2)

**Execução:**
```bash
./tomasulo_simulator input_simple.txt output_simple.txt
```

**Métricas:**
- Total de ciclos: 13
- IPC: 0.23 (3 instruções / 13 ciclos)

### Exemplo 2: Dependências Complexas

**Entrada (input_complex.txt):**
```
INSTRUCTIONS_BEGIN
ADDD F1 F2 F3    # Independente
ADDD F4 F5 F6    # Independente (pode executar em paralelo)
MULTD F7 F1 F4   # Depende de ambas as ADDs
INSTRUCTIONS_END
```

Este exemplo demonstra paralelismo: as duas ADDs executam simultaneamente.

## 🔧 Decisões de Design

### Inicialização de Registradores

Por padrão, todos os registradores são inicializados com valores arbitrários:
- Registradores FP: 1.0 (exceto fontes das instruções)
- F4 = 2.0
- F6 = 10.0
- R1 = 1000.0 (base para LOAD/STORE)
- R2 = 2000.0 (base para LOAD/STORE)

### Latências Padrão

Se não especificadas no arquivo, as latências assumidas são:
- ADD/SUB: 2 ciclos
- MUL: 4 ciclos
- DIV: 10 ciclos
- LOAD: 2 ciclos
- STORE: 2 ciclos

### Tamanho do ROB

Fixado em 16 entradas (ROB_SIZE = 16).

### Número de Registradores

32 registradores de ponto flutuante (F0-F31).

## 🐛 Tratamento de Erros

O simulador detecta e reporta:
- ❌ Arquivo de entrada não encontrado
- ❌ Arquivo de saída não pode ser criado
- ❌ Instruções inválidas (ignoradas com aviso)
- ❌ Deadlock potencial (simulação abortada após 500 ciclos)

## 📚 Referências

- Hennessy, J. L., & Patterson, D. A. (2017). *Computer Architecture: A Quantitative Approach* (6th ed.). Morgan Kaufmann.
- Tomasulo, R. M. (1967). "An Efficient Algorithm for Exploiting Multiple Arithmetic Units". *IBM Journal of Research and Development*, 11(1), 25-33.
- Slides da disciplina de Arquitetura de Computadores

## 👥 Autores

- **[Lucas Barros, Julia Brito, Paulo Dimas e Talita Justo]** - Desenvolvimento e implementação

## 📄 Licença

Este projeto é disponibilizado sob a licença MIT. Veja o arquivo `LICENSE` para mais detalhes.

## 🙏 Agradecimentos

- Professor [Mateus de Alcantara] pela orientação
- Colegas de turma pelas discussões sobre o algoritmo
- Comunidade open-source pelas ferramentas utilizadas

---

**Desenvolvido como projeto acadêmico para a disciplina de Arquitetura de Computadores**

Data: Novembro/2025
