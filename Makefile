# Makefile para o Simulador de Tomasulo

# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Nome do executável
TARGET = tomasulo_simulator

# Arquivo fonte
SRC = Tomasulo_saidaArquivo.cpp

# Arquivo objeto
OBJ = $(SRC:.cpp=.o)

# Regra padrão
all: $(TARGET)

# Regra de compilação
$(TARGET): $(SRC)
	@echo "Compilando $(SRC)..."
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
	@echo "✓ Compilação concluída! Executável: $(TARGET)"

# Compilação com debug
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "✓ Versão debug compilada!"

# Limpeza
clean:
	@echo "Removendo arquivos temporários..."
	rm -f $(TARGET) $(OBJ) *.o
	@echo "✓ Limpeza concluída!"

# Executar teste padrão
test: $(TARGET)
	@echo "Executando teste padrão..."
	./$(TARGET) input.txt output.txt
	@echo "✓ Teste concluído! Veja output.txt"

# Executar todos os testes
test-all: $(TARGET)
	@echo "Executando todos os testes..."
	@./$(TARGET) tests/input1.txt tests/output1.txt || true
	@./$(TARGET) tests/input2.txt tests/output2.txt || true
	@./$(TARGET) tests/input3.txt tests/output3.txt || true
	@echo "✓ Todos os testes executados!"

# Ajuda
help:
	@echo "Makefile do Simulador de Tomasulo"
	@echo ""
	@echo "Uso:"
	@echo "  make          - Compila o simulador"
	@echo "  make debug    - Compila com símbolos de debug"
	@echo "  make clean    - Remove arquivos compilados"
	@echo "  make test     - Executa teste padrão"
	@echo "  make test-all - Executa todos os testes"
	@echo "  make help     - Mostra esta mensagem"

.PHONY: all clean test test-all debug help
