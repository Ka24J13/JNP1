// Pliki naglowkowe
#include <iostream>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
#include <regex>
#include <map>

// Deklaracja uzycia wybranych elementow z przestrzeni std
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::map;
using std::regex;
using std::regex_match;
using std::sregex_iterator;

// Aliasy wielokrotnie uzywanych typow
using line_num_t = size_t;
using gate_t = vector<int>;
using gates_t = vector<vector<int>>;
using output_signals_t = unordered_set<int>;
using input_signals_t = map<int, vector<bool>>;


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
                        regex(R"(\s*NOT(\s+[1-9]+[0-9]*){2}\s*)"),
                        regex(R"(\s*XOR(\s+[1-9]+[0-9]*){3}\s*)"),
                        regex(R"(\s*(AND|NAND|OR|NOR)(\s+[1-9]+[0-9]*){3}(\s+[1-9]+[0-9]*)*\s*)")
                };
        for (const auto &reg : valid_gates_regex) {
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
    bool validate_outputs(const gate_t &gate, output_signals_t &outputs, line_num_t line_num) {
        int output = gate[1];
        if (outputs.find(output) == outputs.end()) {
            outputs.insert(output);
            return true;
        } else {
            cerr << "Error in gate " << line_num << ": signal " << output << " is assigned to multiple outputs."
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
            for (size_t i = 2; i < gate.size(); i++) {
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
    bool isCorrect = true;

    while (getline(cin, line)) {

        line_num++;

        if (!validate_line(line, line_num)) {
            isCorrect = false;
        }
        else {
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
                isCorrect = false;
            }
        }

    }


    if (!isCorrect) exit(1);

    input_signals_t inputs;

    find_input_signals(lines, outputs, inputs);
    vector<bool> signals(inputs.size());

    generate_input_signals(inputs, signals, inputs.size(), 0);

    cout << endl;
    for (const auto &it: inputs) {
        cout << it.first << " ";
    }
    cout << endl;
    for (int i = 0; i < pow(2, inputs.size()); i++) {
        for (const auto &it: inputs) {
            cout << it.second[i] << " ";
        }
        cout << endl;
    }
    cout << endl;


    return 0;
}
