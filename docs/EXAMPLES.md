# Exemplos de Arquivos de Entrada

## Exemplo 1: Básico (input_basic.txt)

Teste simples com 3 instruções com dependências.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES SUBD 2
CYCLES MULTD 4
CYCLES DIVD 10

UNITS ADDD 1
UNITS MULTD 1

CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F8 F4 F6    # F8 = 2 + 10 = 12
MULTD F10 F8 F8  # F10 = 12 * 12 = 144
SUBD F12 F10 F4  # F12 = 144 - 2 = 142
INSTRUCTIONS_END
```

**Resultado esperado:**
- F8 = 12.00
- F10 = 144.00
- F12 = 142.00
- Ciclos: ~13

---

## Exemplo 2: Paralelismo (input_parallel.txt)

Demonstra execução paralela de instruções independentes.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES MULTD 4

UNITS ADDD 2      # 2 unidades ADD (para paralelismo)
UNITS MULTD 2     # 2 unidades MUL

CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F1 F2 F3     # Independente
ADDD F4 F5 F6     # Independente (executa em paralelo)
MULTD F7 F1 F4    # Depende de F1 e F4
MULTD F8 F2 F5    # Independente (pode executar em paralelo com F7)
INSTRUCTIONS_END
```

**Resultado esperado:**
- As duas ADDs executam simultaneamente
- MUL F7 espera ambas ADDs terminarem
- MUL F8 pode executar em paralelo com F7

---

## Exemplo 3: Dependências em Cadeia (input_chain.txt)

Testa dependências RAW em sequência.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES MULTD 4

UNITS ADDD 1
UNITS MULTD 1

CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F1 F2 F3     # F1 = F2 + F3
MULTD F4 F1 F5    # F4 = F1 * F5 (depende de F1)
ADDD F6 F4 F7     # F6 = F4 + F7 (depende de F4)
MULTD F8 F6 F9    # F8 = F6 * F9 (depende de F6)
INSTRUCTIONS_END
```

**Resultado esperado:**
- Execução totalmente sequencial
- Cada instrução espera a anterior terminar
- IPC baixo devido às dependências

---

## Exemplo 4: Renomeação de Registradores (input_rename.txt)

Demonstra eliminação de dependências WAR e WAW.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES MULTD 4

UNITS ADDD 1
UNITS MULTD 1

CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F1 F2 F3     # F1 = F2 + F3 (primeira escrita em F1)
MULTD F4 F1 F5    # F4 = F1 * F5 (lê F1 da primeira instrução)
ADDD F1 F6 F7     # F1 = F6 + F7 (segunda escrita em F1 - WAW!)
MULTD F8 F1 F9    # F8 = F1 * F9 (lê F1 da segunda instrução)
INSTRUCTIONS_END
```

**Comportamento esperado:**
- Instrução 2 lê o F1 da instrução 1 (via ROB tag)
- Instrução 3 reescreve F1 (nova tag no ROB)
- Instrução 4 lê o F1 da instrução 3 (nova tag)
- Sem conflitos graças à renomeação!

---

## Exemplo 5: Divisão de Latência Longa (input_div.txt)

Testa operações com latências diferentes.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES MULTD 4
CYCLES DIVD 10    # Divisão é muito mais lenta

UNITS ADDD 1
UNITS MULTD 1
UNITS DIVD 1

CONFIG_END

INSTRUCTIONS_BEGIN
DIVD F1 F2 F3     # F1 = F2 / F3 (10 ciclos)
ADDD F4 F5 F6     # F4 = F5 + F6 (independente, executa rápido)
MULTD F7 F8 F9    # F7 = F8 * F9 (independente)
ADDD F10 F1 F11   # F10 = F1 + F11 (depende da divisão lenta)
INSTRUCTIONS_END
```

**Resultado esperado:**
- ADD e MUL terminam rápido
- Instrução 4 fica bloqueada esperando a divisão
- Demonstra impacto de latências longas

---

## Exemplo 6: RS Cheia (input_full_rs.txt)

Testa stalls por falta de estações de reserva.

```
CONFIG_BEGIN

CYCLES ADDD 2

UNITS ADDD 1      # Apenas 1 estação ADD!

CONFIG_END

INSTRUCTIONS_BEGIN
ADDD F1 F2 F3     # Ocupa a única estação ADD
ADDD F4 F5 F6     # STALL! Espera a primeira liberar
ADDD F7 F8 F9     # STALL! Espera vaga
INSTRUCTIONS_END
```

**Resultado esperado:**
- Instrução 1 issue no ciclo 1
- Instrução 2 stall até instrução 1 fazer write result
- Instrução 3 stall até instrução 2 fazer write result

---

## Exemplo 7: LOAD e STORE (input_memory.txt)

Testa operações de memória.

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES LD 2
CYCLES SD 2

UNITS ADDD 1
MEM_UNITS LD 1
MEM_UNITS SD 1

CONFIG_END

INSTRUCTIONS_BEGIN
LD F1 0 R1        # F1 = Mem[R1 + 0]
ADDD F2 F1 F3     # F2 = F1 + F3 (depende do LOAD)
SD F2 0 R2        # Mem[R2 + 0] = F2 (depende do ADD)
INSTRUCTIONS_END
```

**Resultado esperado:**
- LOAD lê da memória (valor simulado: 99.0)
- ADD espera LOAD terminar
- STORE espera ADD terminar
- STORE só escreve na memória no COMMIT

---

## Exemplo 8: ROB Cheio (input_full_rob.txt)

Testa comportamento quando o ROB está cheio (requer muitas instruções).

```
CONFIG_BEGIN

CYCLES ADDD 2
CYCLES MULTD 4

UNITS ADDD 2
UNITS MULTD 2

CONFIG_END

INSTRUCTIONS_BEGIN
# 20 instruções para testar o ROB de 16 entradas
ADDD F1 F2 F3
ADDD F4 F5 F6
MULTD F7 F8 F9
ADDD F10 F11 F12
MULTD F13 F14 F15
ADDD F16 F17 F18
MULTD F19 F20 F21
ADDD F22 F23 F24
MULTD F25 F26 F27
ADDD F28 F29 F30
MULTD F1 F2 F3
ADDD F4 F5 F6
MULTD F7 F8 F9
ADDD F10 F11 F12
MULTD F13 F14 F15
ADDD F16 F17 F18
MULTD F19 F20 F21
ADDD F22 F23 F24
MULTD F25 F26 F27
ADDD F28 F29 F30
INSTRUCTIONS_END
```

**Resultado esperado:**
- Depois de 16 issues, próximas instruções stall
- Só fazem issue quando commits liberam espaço no ROB

---

## Como Usar os Exemplos

```bash
# Copie o conteúdo do exemplo para um arquivo
cat > input_basic.txt << 'EOF'
[conteúdo aqui]
EOF

# Execute o simulador
./tomasulo_simulator input_basic.txt output_basic.txt

# Veja o resultado
cat output_basic.txt
```

---

## Criando Seus Próprios Testes

### Dicas:
1. **Comece simples**: 2-3 instruções para entender o comportamento
2. **Teste dependências**: Varie as combinações RAW, WAR, WAW
3. **Varie latências**: Misture operações rápidas e lentas
4. **Teste limites**: RS cheias, ROB cheio
5. **Verifique paralelismo**: Múltiplas unidades do mesmo tipo

### Valores Iniciais (hardcoded no simulador):
- F4 = 2.0
- F6 = 10.0
- R1 = 1000.0
- R2 = 2000.0
- Demais = 1.0
