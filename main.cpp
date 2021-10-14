// Pliki naglowkowe
#include <iostream>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
#include <map>
#include <cmath>
#include <set>

// Deklaracja uzycia wybranych elementow z przestrzeni std
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;
using std::unordered_set;
using std::set;
using std::unordered_map;
using std::map;
using std::regex;
using std::regex_match;
using std::sregex_iterator;
using std::pair;

// Aliasy wielokrotnie uzywanych typow
using line_num_t = size_t;
using gate_t = vector<int>;
using gates_t = vector<vector<int>>;
using output_signals_t = unordered_set<int>;
using signal_t = pair<int, vector<bool>>;
// zwykla mapa jest potrzebna, gdyz sygnaly maja byc printowane w kolejnosci ich numerow
using input_signals_t = map<int, vector<bool>>;

//Stale
static const int NAME_POS = 0; // w danej bramce nazwa bramki bedzie miec index 0
static const int OUTPUT_POS = 1; // w danej bramce nr sygnalu wyjsciowego bedzie miec index 1
static const int INPUT_POS = 2; // w danej bramce nr sygnalow wejsciowych beda sie zaczynac od indexu 2


// Funkcje pomocnicze
namespace {

    /**
     * Sprawdza poprawnosc linii, tj. czy reprezentuje poprawna bramke
     * @param line aktualnie czytana linia inputu
     * @param line_num numer aktualnie czytanej linii inputu
     * @return true, jesli linia reprezentuje poprawna bramke, false wpp.
     */
    bool validate_line(string &line, line_num_t line_num) {

        static const vector<regex> valid_gates_regex =
                {
                        regex(R"(\s*NOT(\s+([1-9]){1}([0-9]){0,8}){2}\s*)"),
                        regex(R"(\s*XOR(\s+([1-9]){1}([0-9]){0,8}){3}\s*)"),
                        regex(R"(\s*(AND|NAND|OR|NOR)(\s+([1-9]){1}([0-9]){0,8}){3}(\s+([1-9]){1}([0-9]){0,8})*\s*)")
                };
        for (const auto &reg: valid_gates_regex) {
            if (regex_match(line, reg)) {
                return true;
            }
        }

        cerr << "Error in line " << line_num << ": " << line << endl;
        return false;
    }

    /**
     * Sprawdza, czy dany sygnal wyjsciowy nie jest przypisany do wiecej niz jednej bramki
     * @param gate aktualnie czytana bramka
     * @param outputs zbior sygnalow wyjsciowych
     * @param line_num numer aktualnie czytanej linii inputu
     * @return true, jesli sygnal wyjsciowy z 'gate' nie znajdowal sie do jest pory w 'outputs', false wpp.
     */
    bool validate_outputs(const gate_t &gate,
                          output_signals_t &outputs,
                          line_num_t line_num) {

        int output = gate[OUTPUT_POS];
        if (outputs.find(output) == outputs.end()) {
            outputs.insert(output);
            return true;
        } else {
            cerr << "Error in line "
                    << line_num
                    << ": signal "
                    << output
                    << " is assigned to multiple outputs."
                    << endl;

            return false;
        }
    }

    /**
     * Funkcja znajduje numery sygnalow, ktore sa jedynie sygnalami wejsciowymi, i wypelnia nimi 'inputs'
     * @param gates wszystkie bramki
     * @param outputs zbior z numerami wszystkich sygnalow wyjsciowych w calym ukladzie
     * @param inputs mapa do przechowywania sygnalow wejsciowych
     */
    void find_input_signals(const gates_t &gates,
                            const output_signals_t &outputs,
                            input_signals_t &inputs) {
        for (auto gate: gates) {
            for (size_t i = INPUT_POS; i < gate.size(); i++) {
                if (outputs.find(gate[i]) == outputs.end()) {
                    vector<bool> v;
                    inputs.insert(make_pair(gate[i], v));
                }
            }
        }
    }

    /**
     * Tworzy permutacje 0 i 1 dla sygnalow wejsciowych ukladu
     * @param inputs mapa z sygnalami wejsciowymi
     * @param signals wektor, gdzie przechowywane beda 0 i 1 do wpisania do 'inputs'
     * @param len zmienna informujaca o liczbie sygnalow wejsciowych
     * @param index zmienna pomocnicza do uzycia w wywolaniach rekurencyjnych
     */
    void generate_input_signals(input_signals_t &inputs,
                                vector<bool> &signals,
                                size_t len,
                                size_t index) {
        if (index == len) {
            int i = 0;
            for (auto &it: inputs) {
                it.second.push_back(signals[i]);
                i++;
            }
            return;
        }

        signals[index] = false;
        generate_input_signals(inputs, signals, len, index + 1);

        signals[index] = true;
        generate_input_signals(inputs, signals, len, index + 1);
    }

    /**
     * Funkcja wypisuje tablice prawdy dla ukladu
     * @param signals poszczegolne sygnaly w ukladzie
     */
    void print_signals(const input_signals_t &signals) {
        for (size_t i = 0; i < signals.begin()->second.size(); i++) {
            for (const auto &it: signals) {
                cout << it.second[i];
            }
            cout << endl;
        }
    }

    /**
     * zaminenia wektor na zbiór
     */
    set<vector<int>> vector_to_set(const gates_t &gates) {
        set<vector<int>> gates_set;
        for (auto &gate: gates) {
            gates_set.insert(gate);
        }
        return gates_set;
    }

    /**
     * sprawdza czy bramkę można zasymulować
     * @param gate bramka
     * @param signals znane sygnały
     * @return true jeżeli można, false wpp.
     */
    bool is_simulatable(const vector<int> &gate,
                        const input_signals_t &signals) {

        for (auto gate_input_it = gate.begin() + INPUT_POS;
             gate_input_it != gate.end();
             ++gate_input_it) {

            if (!signals.contains(*gate_input_it)) return false;
        }
        return true;
    }

    /**
     * funkcje ponizej symuluja dzialanie poszczegolnych bramek
     * @param gate bramka
     * @param signals sygnaly wejsciowe do danej bramki
     * @return sygnal wyjsciowy
     */
    signal_t g_AND(const gate_t &gate, input_signals_t &signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            bool current_output = true;
            for (auto gate_input_it = gate.begin() + INPUT_POS;
                 gate_input_it != gate.end();
                 ++gate_input_it) {

                if (!signals[*gate_input_it].at(i)) {
                    current_output = false;
                    break;
                }
            }
            output.push_back(current_output);
        }
        return {gate[OUTPUT_POS], output};
    }

    signal_t g_XOR(const gate_t &gate, input_signals_t &signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            if (signals[gate[INPUT_POS]].at(i)
                    != signals[gate[INPUT_POS + 1]].at(i)) {

                output.push_back(true);
            } else {
                output.push_back(false);
            }
        }
        return {gate[OUTPUT_POS], output};
    }

    signal_t g_NOT(const gate_t &gate, input_signals_t &signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            output.push_back(!signals[gate[INPUT_POS]].at(i));
        }
        return {gate[OUTPUT_POS], output};
    }

    signal_t g_NAND(const gate_t &gate, input_signals_t signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            bool current_output = false;
            for (auto gate_input_it = gate.begin() + INPUT_POS;
                 gate_input_it != gate.end();
                 ++gate_input_it) {

                if (!signals[*gate_input_it].at(i)) {
                    current_output = true;
                    break;
                }
            }
            output.push_back(current_output);
        }
        return {gate[OUTPUT_POS], output};
    }

    signal_t g_OR(const gate_t &gate, input_signals_t signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            bool current_output = false;
            for (auto gate_input_it = gate.begin() + INPUT_POS;
                 gate_input_it != gate.end();
                 ++gate_input_it) {

                if (signals[*gate_input_it].at(i)) {
                    current_output = true;
                    break;
                }
            }
            output.push_back(current_output);
        }
        return {gate[OUTPUT_POS], output};
    }

    signal_t g_NOR(const gate_t &gate, input_signals_t signals) {
        vector<bool> output;
        for (size_t i = 0;
                i < signals.at(*(gate.begin() + INPUT_POS)).size();
                i++) {

            bool current_output = true;
            for (auto gate_input_it = gate.begin() + INPUT_POS;
                 gate_input_it != gate.end();
                 ++gate_input_it) {

                if (signals[*gate_input_it].at(i)) {
                    current_output = false;
                    break;
                }
            }
            output.push_back(current_output);
        }
        return {gate[OUTPUT_POS], output};
    }

    /**
     * symuluje bramkę
     * @param gate bramka
     * @param input_signals sygnaly wejsciowe do danej bramki
     * @return sygnal wyjsciowy
     */
    signal_t simulate_one(const vector<int> &gate,
                          input_signals_t &input_signals) {

        switch (gate[NAME_POS]) {
            case 0:
                return g_NOT(gate, input_signals);
            case 1:
                return g_XOR(gate, input_signals);
            case 2:
                return g_AND(gate, input_signals);
            case 3:
                return g_NAND(gate, input_signals);
            case 4:
                return g_OR(gate, input_signals);
            default:
                return g_NOR(gate, input_signals);
        }
    }

    /**
     * przeprowadza symulację dla wszystkich kombinacji sygnałów wejściowych
     * @param gates wszystkie bramki
     * @return false jeżeli układ sekwencyjny, true wpp.
     */
    bool simulate_all(const gates_t &gates, input_signals_t &inputs) {

        set<vector<int>> to_simulate = vector_to_set(gates);

        do {
            bool progress = false;

            for (auto gate_it = to_simulate.begin();
                 gate_it != to_simulate.end();) {

                if (is_simulatable(*gate_it, inputs)) {
                    inputs.insert(simulate_one(*gate_it, inputs));
                    to_simulate.erase(gate_it++);      // już zasymulowana
                    progress = true;
                } else gate_it++;
            }

            if (!progress) {
                cerr <<
                "Error: sequential logic analysis has not yet been implemented."
                << endl;

                return false;    // układ sekwencyjny
            }
        } while (!to_simulate.empty());

        return true;
    }

}  // Koniec anonimowej przestrzeni nazw


int main() {

    static const unordered_map<string, int> gates_names =
            {
                    {"NOT",  0},
                    {"XOR",  1},
                    {"AND",  2},
                    {"NAND", 3},
                    {"OR",   4},
                    {"NOR",  5}
            };

    string line;
    gates_t lines;
    output_signals_t outputs;
    line_num_t line_num = 0;
    bool is_correct = true;

    while (getline(cin, line)) {

        line_num++;

        if (!validate_line(line, line_num)) {
            is_correct = false;
        } else {
            stringstream s(line);
            gate_t gate;
            string word;

            s >> word;
            gate.push_back(gates_names.find(word)->second);

            while (s >> word) {
                gate.push_back(stoi(word));
            }
            lines.push_back(gate);

            if (!validate_outputs(gate, outputs, line_num)) {
                is_correct = false;
            }
        }

    }

    if (!is_correct) exit(1);

    input_signals_t inputs;

    find_input_signals(lines, outputs, inputs);
    vector<bool> signals(inputs.size());

    generate_input_signals(inputs, signals, inputs.size(), 0);

    if (simulate_all(lines, inputs)) {
        print_signals(inputs);
    }

    return 0;
}
