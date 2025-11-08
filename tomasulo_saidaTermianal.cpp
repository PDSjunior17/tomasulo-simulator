#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>

// --- Definições Globais e Mapeamentos ---
const int NUM_FP_REGISTERS = 32;
const int ROB_SIZE = 16;

enum OpCode
{
    ADD_D,
    SUB_D,
    MUL_D,
    DIV_D,
    L_D,
    S_D,
    UNKNOWN
};

enum ROBestado
{
    Issue,
    executando,
    escreveresult,
    Commit
};

// Tabela de mapeamento para leitura de arquivo
const std::map<std::string, OpCode> OpCodeMap = {
    {"ADDD", ADD_D}, {"SUBD", SUB_D}, {"MULTD", MUL_D}, {"DIVD", DIV_D}, {"LD", L_D}, {"SD", S_D}};

OpCode stringToOpCode(const std::string &opStr)
{
    auto it = OpCodeMap.find(opStr);
    if (it != OpCodeMap.end())
    {
        return it->second;
    }
    return UNKNOWN;
}

int registerNameToIndex(const std::string &regName)
{
    if (regName.empty())
        return -1;
    if (regName[0] == 'F' || regName[0] == 'f' || regName[0] == 'R' || regName[0] == 'r')
    {
        try
        {
            return std::stoi(regName.substr(1));
        }
        catch (...)
        {
            return -1;
        }
    }
    return -1;
}

// --- CLASSE INSTRUÇÃO ---
class Instrucao
{
public:
    OpCode op;
    int dest_reg;
    int src1_reg;
    int src2_reg;
    int imediato;

    Instrucao(OpCode operation, int dest, int src1, int src2, int imm = 0)
        : op(operation), dest_reg(dest), src1_reg(src1), src2_reg(src2), imediato(imm) {}

    Instrucao() : op(ADD_D), dest_reg(-1), src1_reg(-1), src2_reg(-1), imediato(0) {}

    std::string getOpName() const
    {
        switch (op)
        {
        case ADD_D:
            return "ADD.D";
        case SUB_D:
            return "SUB.D";
        case MUL_D:
            return "MUL.D";
        case DIV_D:
            return "DIV.D";
        case L_D:
            return "L.D";
        case S_D:
            return "S.D";
        default:
            return "UNKNOWN";
        }
    }
};

// --- CLASSE ESTAÇÃO DE RESERVA ---
class EstacaoReserva
{
public:
    int id;
    bool ocupado = false;
    OpCode op;
    float Vj = 0.0;
    float Vk = 0.0;
    int Qj = 0;
    int Qk = 0;
    int Dest = 0;
    long long A = 0;
    int ciclosfaltantes = 0;

    EstacaoReserva(int rs_id) : id(rs_id) {}

    void clear()
    {
        ocupado = false;
        op = UNKNOWN;
        Vj = Vk = 0.0;
        Qj = Qk = Dest = 0;
        A = 0;
        ciclosfaltantes = 0;
    }
};

// --- CLASSE MODO REGISTRADOR (STATUS) ---
class ModoRegistrador
{
private:
    std::vector<int> status; // Tag do ROB (0 = Valor pronto)
public:
    ModoRegistrador() { status.resize(NUM_FP_REGISTERS, 0); }
    int getTag(int reg_index) const
    {
        if (reg_index >= 0 && reg_index < NUM_FP_REGISTERS)
            return status[reg_index];
        return 0;
    }
    void setTag(int reg_index, int rob_tag)
    {
        if (reg_index >= 0 && reg_index < NUM_FP_REGISTERS)
            status[reg_index] = rob_tag;
    }
    void clearTag(int reg_index, int rob_tag)
    {
        if (reg_index >= 0 && reg_index < NUM_FP_REGISTERS && status[reg_index] == rob_tag)
        {
            status[reg_index] = 0;
        }
    }
};

// --- CLASSE BUFFER DE REORDENAÇÃO (ROB) ---
class ROB_Entry
{
public:
    bool ocupado = false;
    ROBestado estado = Issue;
    int reddestido = -1; // Registrador destino (ou fonte, no caso do S.D)
    float valor = 0.0; // Valor a ser escrito (no reg ou na mem)
    bool enderecocerto = false;
    long long enderecoMemoria = 0;
    OpCode op = UNKNOWN;

    void clear()
    {
        ocupado = false;
        estado = Issue;
        reddestido = -1;
        valor = 0.0;
        enderecocerto = false;
        enderecoMemoria = 0;
        op = UNKNOWN;
    }
};

class BufferReordenacao
{
private:
    std::vector<ROB_Entry> entries;
    int cabeca = 0;
    int calda = 0;

public:
    BufferReordenacao() { entries.resize(ROB_SIZE); }
    bool isFull() const { return entries[calda].ocupado; }
    bool isEmpty() const { return !entries[cabeca].ocupado && cabeca == calda; }

    int issue(const Instrucao &inst)
    {
        if (isFull())
            return -1;
        entries[calda].clear();
        entries[calda].ocupado = true;
        entries[calda].op = inst.op;
        entries[calda].reddestido = inst.dest_reg; // Para L.D e Arith, é o destino. Para S.D, é o *valor fonte*.
        entries[calda].estado = Issue;

        int novatag = calda + 1;
        calda = (calda + 1) % ROB_SIZE;
        return novatag;
    }

    ROB_Entry &getEntry(int rob_tag)
    {
        return entries[(rob_tag - 1)];
    }

    ROB_Entry &getcabecaEntry()
    {
        return entries[cabeca];
    }

    int getHeadIndex() const
    {
        return cabeca;
    }
    int getCaldaIndex() const
    {
        return calda;
    }

    void advancecabeca()
    {
        entries[cabeca].clear();
        cabeca = (cabeca + 1) % ROB_SIZE;
    }
};

// --- CLASSE SIMULADOR TOMASULO ---
class TomasuloSimulator
{
public:
    std::vector<EstacaoReserva> rs_add;
    std::vector<EstacaoReserva> rs_mult;
    std::vector<EstacaoReserva> rs_load; 
    std::vector<EstacaoReserva> rs_store; 

    std::vector<float> fp_registers_values;
    ModoRegistrador reg_status;
    BufferReordenacao rob;
    std::queue<Instrucao> instruction_queue;

    std::map<OpCode, int> cycle_times;
    std::map<OpCode, int> unit_counts;
    std::map<int, float> cdb_broadcast; // Simula o CDB com Tag->Valor no ciclo atual

    int clock_cycle = 0;
    int instructions_committed = 0;

    TomasuloSimulator() : fp_registers_values(NUM_FP_REGISTERS, 0.0) {}

    void loadConfiguration(std::ifstream &inputFile);
    void loadInstructions(std::ifstream &inputFile);
    void runSimulation();

    void issue();
    void execute();
    void writeResult();
    void commit();

    void printSimulatorStatus();
};

void TomasuloSimulator::loadConfiguration(std::ifstream &inputFile)
{
    std::string line, key, typeStr;
    int value;
    while (std::getline(inputFile, line) && line.find("CONFIG_BEGIN") == std::string::npos)
        ;

    while (std::getline(inputFile, line) && line.find("CONFIG_END") == std::string::npos)
    {
        std::stringstream ss(line);
        ss >> key >> typeStr >> value;
        OpCode op = stringToOpCode(typeStr);

        if (key == "CYCLES" && op != UNKNOWN)
        {
            cycle_times[op] = value;
        }
        else if ((key == "UNITS" || key == "MEM_UNITS") && op != UNKNOWN)
        {
            if (op == ADD_D || op == SUB_D)
            {
                for (int i = 0; i < value; ++i)
                    rs_add.emplace_back(i + 1);
                unit_counts[op] = value;
            }
            else if (op == MUL_D || op == DIV_D)
            {
                for (int i = 0; i < value; ++i)
                    rs_mult.emplace_back(i + 1);
                unit_counts[op] = value;
            }
            // --- Adiciona L_D e S_D em buffers separados ---
            else if (op == L_D)
            {
                for (int i = 0; i < value; ++i)
                    rs_load.emplace_back(i + 1);
                unit_counts[op] = value;
            }
            else if (op == S_D)
            {
                for (int i = 0; i < value; ++i)
                    rs_store.emplace_back(i + 1);
                unit_counts[op] = value;
            }
        }
    }
}

void TomasuloSimulator::loadInstructions(std::ifstream &inputFile)
{
    std::string line, opStr, destStr, src1Str, src2Str;
    while (std::getline(inputFile, line) && line.find("INSTRUCTIONS_BEGIN") == std::string::npos)
        ;

    while (std::getline(inputFile, line) && line.find("INSTRUCTIONS_END") == std::string::npos)
    {
        std::stringstream ss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (ss >> token && token.find('#') == std::string::npos)
        {
            tokens.push_back(token);
        }

        if (tokens.size() < 3)
            continue;

        opStr = tokens[0];
        OpCode op = stringToOpCode(opStr);
        if (op == UNKNOWN)
            continue;

        int dest_reg = registerNameToIndex(tokens[1]); 
        int src1_reg = (tokens.size() > 2) ? registerNameToIndex(tokens[2]) : -1; 
        int src2_reg = (tokens.size() > 3) ? registerNameToIndex(tokens[3]) : -1; 
        int imediato = 0;

        if (op == L_D || op == S_D)
        {
            try { imediato = std::stoi(tokens[2]); } catch(...) { imediato = 0; }
            src1_reg = src2_reg; // O registrador base (R1)
            src2_reg = -1;
        }

        instruction_queue.push(Instrucao(op, dest_reg, src1_reg, src2_reg, imediato));
    }
}

// --- FASES DE EXECUÇÃO ---

void TomasuloSimulator::issue()
{
    if (instruction_queue.empty() || rob.isFull())
        return;

    Instrucao current_inst = instruction_queue.front();
    EstacaoReserva *target_rs = nullptr;

    // Lógica para RS_ADD/SUB
    if (current_inst.op == ADD_D || current_inst.op == SUB_D)
    {
        for (auto &rs : rs_add)
        {
            if (!rs.ocupado)
            {
                target_rs = &rs;
                break;
            }
        }
    }
    // Lógica para RS_MULT/DIV
    else if (current_inst.op == MUL_D || current_inst.op == DIV_D)
    {
        for (auto &rs : rs_mult)
        {
            if (!rs.ocupado)
            {
                target_rs = &rs;
                break;
            }
        }
    }
    // Lógica para RS_LOAD
    else if (current_inst.op == L_D)
    {
        for (auto &rs : rs_load)
        {
            if (!rs.ocupado)
            {
                target_rs = &rs;
                break;
            }
        }
    }
    // Lógica para RS_STORE
    else if (current_inst.op == S_D)
    {
        for (auto &rs : rs_store)
        {
            if (!rs.ocupado)
            {
                target_rs = &rs;
                break;
            }
        }
    }

    if (target_rs == nullptr)
        return; // Stall: RS cheia

    int rob_tag = rob.issue(current_inst);

    target_rs->clear();
    target_rs->ocupado = true;
    target_rs->op = current_inst.op;
    target_rs->Dest = rob_tag;

    // Renomeação do Fonte 1:
    if (current_inst.src1_reg != -1)
    {
        int qj_tag = reg_status.getTag(current_inst.src1_reg);
        if (qj_tag != 0)
        {
            target_rs->Qj = qj_tag;
        }
        else
        {
            target_rs->Vj = fp_registers_values[current_inst.src1_reg];
            target_rs->Qj = 0;
        }
    }

    // Renomeação do Fonte 2 (Apenas para Arith e Store)
    if (current_inst.op == ADD_D || current_inst.op == SUB_D || current_inst.op == MUL_D || current_inst.op == DIV_D)
    {
        // Fonte 2 (Arith)
        int qk_tag = reg_status.getTag(current_inst.src2_reg);
        if (qk_tag != 0)
        {
            target_rs->Qk = qk_tag;
        }
        else
        {
            target_rs->Vk = fp_registers_values[current_inst.src2_reg];
            target_rs->Qk = 0;
        }
    }
    else if (current_inst.op == S_D)
    {
        // Fonte 2 (Store) -> É o *valor* (F2 em S.D F2, 0(R1))
        // O parser salvou F2 em 'dest_reg'
        int qk_tag = reg_status.getTag(current_inst.dest_reg);
        if (qk_tag != 0)
        {
            target_rs->Qk = qk_tag;
        }
        else
        {
            target_rs->Vk = fp_registers_values[current_inst.dest_reg];
            target_rs->Qk = 0;
        }
    }

    // Atualiza o Status do Registrador Destino
    if (current_inst.op != S_D && current_inst.dest_reg != -1)
    {
        reg_status.setTag(current_inst.dest_reg, rob_tag);
    }

    instruction_queue.pop();
    std::cout << "  > ISSUED: " << current_inst.getOpName() << " (Dest ROB Tag: " << rob_tag << ")\n";
}

void TomasuloSimulator::execute()
{

    // Lógica para operações aritméticas (ADD/SUB/MUL/DIV)
    auto process_arith_rs = [&](std::vector<EstacaoReserva> &rs_set) {
        for (auto &rs : rs_set)
        {
            if (!rs.ocupado || rs.op == L_D || rs.op == S_D) continue;

            if (rs.Qj == 0 && rs.Qk == 0) // Ambos operandos prontos
            {
                ROB_Entry &rob_entry = rob.getEntry(rs.Dest);

                if (rob_entry.estado == Issue)
                { // Inicia execução
                    rs.ciclosfaltantes = cycle_times[rs.op] - 1;
                    rob_entry.estado = executando;
                }
                else if (rob_entry.estado == executando)
                { // Continua execução
                    if (rs.ciclosfaltantes > 0)
                    {
                        rs.ciclosfaltantes--;
                    }
                }

                // FINAL DA EXECUÇÃO (CORREÇÃO DE LÓGICA DE CÁLCULO)
                if (rs.ciclosfaltantes == 0 && rob_entry.estado == executando)
                {
                    // **CORREÇÃO: O cálculo DEVE ocorrer aqui, no final da execução.**
                    float result = 0.0;
                    if (rs.op == ADD_D)
                        result = rs.Vj + rs.Vk;
                    else if (rs.op == SUB_D)
                        result = rs.Vj - rs.Vk;
                    else if (rs.op == MUL_D)
                        result = rs.Vj * rs.Vk;
                    else if (rs.op == DIV_D)
                        result = (rs.Vk != 0) ? (rs.Vj / rs.Vk) : 0.0;

                    // Armazena o resultado no ROB
                    rob_entry.valor = result;
                    rob_entry.estado = escreveresult;
                    
                    std::cout << "  > EXECUTED: " << Instrucao(rs.op, -1, -1, -1).getOpName()
                              << " (Tag: " << rs.Dest << ") - Resultado (" << result << ") pronto.\n";
                }
            }
        }
    };

    // --- Lógica para Operações de L.D (Load) ---
    auto process_load_rs = [&](std::vector<EstacaoReserva> &rs_set) {
        for (auto &rs : rs_set)
        {
            if (rs.ocupado && rs.op == L_D)
            {
                ROB_Entry &rob_entry = rob.getEntry(rs.Dest);

                // Load só pode iniciar se R1 (src1_reg/Vj) estiver pronto (Qj=0)
                if (rs.Qj == 0)
                {
                    if (rob_entry.estado == Issue)
                    { // Inicia execução (Cálculo de Endereço/Acesso à Memória)
                        rs.ciclosfaltantes = cycle_times[rs.op] - 1;
                        rob_entry.estado = executando;

                        // Simulação de cálculo de endereço (Placeholder)
                        rob_entry.enderecocerto = true;
                    }
                    else if (rob_entry.estado == executando)
                    { // Continua execução
                        if (rs.ciclosfaltantes > 0)
                        {
                            rs.ciclosfaltantes--;
                        }
                    }

                    // Final da Execução (Acesso à memória concluído)
                    if (rs.ciclosfaltantes == 0 && rob_entry.estado == executando)
                    {
                        // **NOTA:** Aqui assumimos um valor fixo 99.0 para simular a leitura da memória.
                        rob_entry.valor = 99.0; 
                        rob_entry.estado = escreveresult;
                        std::cout << "  > EXECUTED (LOAD): " << Instrucao(rs.op, -1, -1, -1).getOpName()
                                  << " (Tag: " << rs.Dest << ") - Resultado pronto.\n";
                    }
                }
            }
        }
    };

    // --- Lógica para Operações de S.D (Store) ---
    auto process_store_rs = [&](std::vector<EstacaoReserva> &rs_set) {
        for (auto &rs : rs_set)
        {
            if (rs.ocupado && rs.op == S_D)
            {
                ROB_Entry &rob_entry = rob.getEntry(rs.Dest);

                // 1. Cálculo do Endereço (Depende de Qj)
                if (rs.Qj == 0 && !rob_entry.enderecocerto)
                {
                    // (Simulação de cálculo de endereço)
                    rob_entry.enderecoMemoria = (long long)rs.Vj + 0; // (0 = imediato)
                    rob_entry.enderecocerto = true;
                    rob_entry.estado = executando; // Endereço calculado, agora espera o valor
                    std::cout << "  > EXECUTED (STORE): Endereço calculado (Tag: " << rs.Dest << ")\n";
                }
            }
        }
    };

    // Chamada das lógicas de execução
    process_arith_rs(rs_add);
    process_arith_rs(rs_mult);
    process_load_rs(rs_load);
    process_store_rs(rs_store);
}

void TomasuloSimulator::writeResult()
{
    cdb_broadcast.clear(); // Limpa o CDB do ciclo anterior
    std::vector<EstacaoReserva *> completed_rs_broadcast; // (LD, Arith)
    std::vector<EstacaoReserva *> completed_rs_store;// (SD)

    // --- 1. Coleta RSs prontas para o CDB (Arith e Load) ---
    auto check_and_broadcast_cdb = [&](std::vector<EstacaoReserva> &rs_set) {
        for (auto &rs : rs_set)
        {
            if (!rs.ocupado) continue;
            
            ROB_Entry &rob_entry = rob.getEntry(rs.Dest);

            // Verifica se a instrução terminou a execução e está pronta para o CDB
            if (rs.ocupado && rob_entry.estado == escreveresult && cdb_broadcast.find(rs.Dest) == cdb_broadcast.end())
            {
                // **CORREÇÃO:** Pega o resultado já calculado na fase execute()
                float result = rob_entry.valor;

                // --- Atualiza CDB ---
                cdb_broadcast[rs.Dest] = result;

                completed_rs_broadcast.push_back(&rs);
                std::cout << "  > WRITE RESULT: Tag " << rs.Dest << " valor (" << result << ") no CDB.\n";
            }
        }
    };

    check_and_broadcast_cdb(rs_add);
    check_and_broadcast_cdb(rs_mult);
    check_and_broadcast_cdb(rs_load); 

    // --- 2. Broadcast para OUTRAS RSs (incluindo Store Buffers) ---
    auto broadcast_to_rs = [&](std::vector<EstacaoReserva> &rs_set) {
        for (auto &rs : rs_set)
        {
            if (rs.ocupado)
            {
                for (const auto &item : cdb_broadcast)
                {
                    int tag = item.first;
                    float value = item.second;

                    if (rs.Qj == tag)
                    {
                        rs.Vj = value;
                        rs.Qj = 0;
                    }
                    if (rs.Qk == tag)
                    {
                        rs.Vk = value;
                        rs.Qk = 0;
                    }
                }
            }
        }
    };

    broadcast_to_rs(rs_add);
    broadcast_to_rs(rs_mult);
    broadcast_to_rs(rs_load);
    broadcast_to_rs(rs_store); // S.D precisa ouvir o CDB para seu valor (Qk)

    // --- 3. Verifica Stores (que NÃO transmitem no CDB) ---
    for (auto &rs : rs_store)
    {
        ROB_Entry &rob_entry = rob.getEntry(rs.Dest);

        // Qj=0 (endereço calculado no execute) E Qk=0 (valor chegou via CDB)
        // E o endereço já foi calculado (estado = executando)
        if (rs.ocupado && rs.Qj == 0 && rs.Qk == 0 && rob_entry.estado == executando)
        {
            rob_entry.estado = escreveresult;
            // S.D F2, 0(R1). O valor (Vk) veio de F2.
            rob_entry.valor = rs.Vk; // Guarda o valor a ser escrito no ROB (para o Commit usar)
            completed_rs_store.push_back(&rs);
            std::cout << "  > WRITE RESULT (STORE): Tag " << rs.Dest << " pronto para Commit.\n";
        }
    }


    // --- 4. Libera as RSs que terminaram ---
    for (auto *rs_ptr : completed_rs_broadcast)
    {
        rs_ptr->clear();
    }
    for (auto *rs_ptr : completed_rs_store)
    {
        rs_ptr->clear();
    }
}

void TomasuloSimulator::commit()
{
    ROB_Entry &head_entry = rob.getcabecaEntry();

    if (head_entry.ocupado && head_entry.estado == escreveresult)
    {
        int rob_tag = rob.getHeadIndex() + 1;
        
        // 1. TRATAMENTO DE INSTRUÇÕES QUE ESCREVEM EM REGISTRADORES (L.D, ADD.D, etc.)
        if (head_entry.op != S_D && head_entry.reddestido != -1)
        {
            // Escrita no Registrador FP
            fp_registers_values[head_entry.reddestido] = head_entry.valor;

            // Limpeza da Tag
            reg_status.clearTag(head_entry.reddestido, rob_tag);

            std::cout << "  > COMMITTED: " << Instrucao(head_entry.op, -1, -1, -1).getOpName()
                      << " -> F" << head_entry.reddestido << " = " << head_entry.valor << "\n";
        }
        // 2. TRATAMENTO DE INSTRUÇÕES S.D
        else if (head_entry.op == S_D)
        {
            // Para S.D, a escrita na memória acontece aqui no Commit
            std::cout << "  > COMMITTED (MEM): " << Instrucao(head_entry.op, -1, -1, -1).getOpName()
                      << " -> Escrita Mem[" << head_entry.enderecoMemoria << "] = " 
                      << head_entry.valor << " realizada.\n";
        }

        // 3. FINALIZA O COMMIT
        rob.advancecabeca();
        instructions_committed++;
    }
}

// --- FUNÇÕES DE SAÍDA E MAIN ---

void TomasuloSimulator::printSimulatorStatus()
{
    std::cout << "\n==================================================\n";
    std::cout << "CICLO " << clock_cycle << "\n";
    std::cout << "==================================================\n";

    // 1. ESTAÇÕES DE RESERVA
    std::cout << "--- ESTAÇÕES DE RESERVA ---\n";
    std::cout << std::left << std::setw(4) << "ID" << std::setw(10) << "Ocupado" << std::setw(9) << "Op"
              << std::setw(7) << "Qj" << std::setw(7) << "Qk" << std::setw(7) << "Vj" << std::setw(7) << "Vk"
              << std::setw(7) << "Dest" << "Ciclos\n";

    auto print_rs_set = [&](const std::vector<EstacaoReserva> &rs_set, const std::string &prefix) {
        for (const auto &rs : rs_set)
        {
            if (rs.ocupado)
            {
                std::string qj_str = (rs.Qj == 0) ? "-" : std::to_string(rs.Qj);
                std::string qk_str = (rs.Qk == 0) ? "-" : std::to_string(rs.Qk);
                std::string op_str = Instrucao(rs.op, -1, -1, -1).getOpName();

                std::cout << std::left << std::setw(4) << (prefix + std::to_string(rs.id))
                          << std::setw(10) << "SIM"
                          << std::setw(9) << op_str
                          << std::setw(7) << qj_str
                          << std::setw(7) << qk_str
                          << std::setw(7) << std::fixed << std::setprecision(2) << rs.Vj
                          << std::setw(7) << std::fixed << std::setprecision(2) << rs.Vk
                          << std::setw(7) << rs.Dest
                          << rs.ciclosfaltantes << "\n";
            }
        }
    };
    print_rs_set(rs_add, "A");
    print_rs_set(rs_mult, "M");
    print_rs_set(rs_load, "L"); 
    print_rs_set(rs_store, "S"); 

    // 2. BUFFER DE REORDENAÇÃO (ROB)
    std::cout << "\n--- BUFFER DE REORDENAÇÃO (ROB) ---\n";
    std::cout << std::left << std::setw(4) << "ID" << std::setw(10) << "Ocupado" << std::setw(14) << "Estado"
              << std::setw(8) << "Destino" << "Valor" << " Endereço\n";

    // Iterar pela lista de forma circular (Head até Tail)
    int current_index = rob.getHeadIndex();
    bool first_pass = true;
    int count = 0; 
    
    while(count < ROB_SIZE)
    {
        ROB_Entry &entry = rob.getEntry(current_index + 1); 
        if (!entry.ocupado && !first_pass)
            break; 

        if (entry.ocupado)
        {
            std::string estado_str;
            switch (entry.estado)
            {
            case Issue:         estado_str = "Issue";       break;
            case executando:    estado_str = "Executando";  break;
            case escreveresult: estado_str = "Pronto";      break;
            case Commit:        estado_str = "Commit";      break;
            }
            
            std::string dest_str = "Mem";
            if(entry.op != S_D) {
                 dest_str = (entry.reddestido != -1) ? ("F" + std::to_string(entry.reddestido)) : "-";
            }

            std::cout << std::left << std::setw(4) << (current_index + 1)
                      << std::setw(10) << "SIM"
                      << std::setw(14) << estado_str
                      << std::setw(8) << dest_str
                      << std::setw(8) << std::fixed << std::setprecision(2) << entry.valor;
            if(entry.op == L_D || entry.op == S_D) {
                std::cout << entry.enderecoMemoria;
            }
            std::cout << "\n";
        }
        
        if (current_index == (rob.getCaldaIndex() - 1 + ROB_SIZE) % ROB_SIZE && entry.ocupado)
             break; // Chegou na cauda
        
        current_index = (current_index + 1) % ROB_SIZE;
        first_pass = false;
        count++;
    }


    // 3. STATUS DOS REGISTRADORES
    std::cout << "\n--- STATUS DOS REGISTRADORES (Tags do ROB) ---\n";
    std::cout << std::left << std::setw(5) << "Reg" << "Tag\n";
    for (int i = 0; i < NUM_FP_REGISTERS; ++i)
    {
        if (reg_status.getTag(i) != 0)
        {
            std::cout << std::left << std::setw(5) << ("F" + std::to_string(i)) << reg_status.getTag(i) << "\n";
        }
    }
}

void TomasuloSimulator::runSimulation()
{
    int total_instructions = instruction_queue.size();
    if (total_instructions == 0) {
        std::cout << "Nenhuma instrução válida encontrada.\n";
        return;
    }

    for (int i = 0; i < NUM_FP_REGISTERS; ++i)
    {
        fp_registers_values[i] = 1.0; 
    }

    // Inicializa valores arbitrários nos registradores
    fp_registers_values[8] = 5.0;// F8 (Será sobrescrito)
    fp_registers_values[4] = 2.0;// F4 (Fonte: 2.0)
    fp_registers_values[1] = 1000.0; // F1 (R1 Base 1)
    fp_registers_values[2] = 2000.0; // F2 (R2 Base 2)
    fp_registers_values[6] = 10.0; // F6 (Fonte: 10.0)

    // Confirmação de inicialização
    std::cout << "Valores iniciais de F4 e F6: F4=" << fp_registers_values[4] << ", F6=" << fp_registers_values[6] << "\n";

    // Inicia o ciclo de clock em 0
    clock_cycle = 0;

    while (instructions_committed < total_instructions)
    {
        clock_cycle++;
        std::cout << "\n--- INICIANDO CICLO " << clock_cycle << " ---\n";

        // Ordem do ciclo: Commit -> Write Result -> Execute -> Issue
        commit();
        writeResult();
        execute();
        issue();

        printSimulatorStatus();

        if(clock_cycle > 500) {
             std::cout << "Simulação excedeu 500 ciclos (possível deadlock). Abortando.\n";
             break;
        }
    }

    // Imprime o estado final dos registradores
    std::cout << "\n\n=== SIMULAÇÃO CONCLUÍDA em " << clock_cycle << " CICLOS ===\n";
    std::cout << "\n--- VALORES FINAIS DOS REGISTRADORES FP ---\n";
    for (int i = 0; i < NUM_FP_REGISTERS; ++i)
    {
            std::cout << "F" << i << ": " << std::fixed << std::setprecision(2) << fp_registers_values[i] << "\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_arquivo_de_instrucoes.txt>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open())
    {
        std::cerr << "Erro: Não foi possível abrir o arquivo " << argv[1] << "\n";
        return 1;
    }

    TomasuloSimulator simulator;

    // 1. Carregar Configuração
    simulator.loadConfiguration(inputFile);

    // Reinicia a leitura do arquivo para a seção de instruções
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    // 2. Carregar Instruções
    simulator.loadInstructions(inputFile);

    if (simulator.instruction_queue.empty())
    {
        std::cout << "Nenhuma instrução válida encontrada. Simulação encerrada.\n";
        return 0;
    }

    // 3. Executar Simulação
    simulator.runSimulation();

    return 0;
}