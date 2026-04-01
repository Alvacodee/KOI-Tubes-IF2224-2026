#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Cara menggunakan: ./arion <input.txt>" << endl;
        return 1;
    }

    string filename = argv[1];
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error: Tidak dapat membuka file " << filename << endl;
        return 1;
    }

    cout << "Berhasil membuka file: " << filename << endl;
    cout << "--- Memulai proses baca karakter ---" << endl;

    char currentChar;
    while (inputFile.get(currentChar)) {
        cout << currentChar; 
    }

    cout << "\n--- Selesai ---" << endl;

    inputFile.close();
    return 0;
}