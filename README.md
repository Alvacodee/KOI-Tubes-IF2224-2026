# [KODE]-Tubes-IF2224-2026

## Program Description

Program ini adalah sebuah *Lexical Analyzer* sederhana yang dibangun menggunakan bahasa C++. Program berfungsi untuk membaca file teks kode sumber (`.txt`), melakukan proses tokenisasi (memecah teks kode menjadi token-token spesifik), menampilkan hasilnya ke layar, dan secara otomatis menyimpan hasil tersebut ke dalam file baru.

## Requirements

Untuk dapat menjalankan program ini, pastikan perangkat Anda memenuhi kebutuhan berikut:

1. **C++ Compiler**: Anda membutuhkan *compiler* untuk menerjemahkan kode C++ menjadi program yang dapat dijalankan.
   * **Windows**: MinGW (g++) atau MSVC.
   * **Linux (Ubuntu)**: GCC (g++).
2. **File Source Code**: Pastikan semua modul file berada dalam direktori (folder) yang sama:
   * `main.cpp`
   * `token.hpp`
   * `token.cpp`
   * `identifier.hpp`
   * `identifier.cpp`
   * `literal.hpp`
   * `literal.cpp`
   * `operator.hpp`
   * `operator.cpp`
   * `lexer.hpp`
   * `lexer.cpp`

3. **File Input**: Sebuah file berformat `.txt` (contoh: `input.txt`) yang berisi teks atau kode untuk dianalisis oleh Lexer.

## How to Compile and Run
Buka terminal (Command Prompt/PowerShell di Windows, atau Terminal di Linux) dan arahkan ke direktori tempat file Anda berada. Jalankan perintah berikut untuk mengompilasi program:

 * **Linux (WSL)**:

```bash
make all
```


```bash
make run
```



## Division of tasks

| No. | Nama |  Deskripsi Tugas | 
|------|-----|------|
| 1. | Zahran Alvan Putra Winarko | Handle identifier & keyword (variabel, nama, program, var, dll) | 
| 2. | Neswa Eka Anggara | Handle comment, whitespace & gabung semua (flow lexer/DFA) |
| 3. | Daniel Putra Rywandi S | Handle angka & literal (int, real, char, string)  |
| 4. | Muh. Hartawan Haidir | Handle operator & simbol (+, :=, <=, ;, dll) |

## Contributors

| Nama | NIM |
|------|-----|
| Zahran Alvan P W | 135240124 |
| Neswa Eka Anggara	| 13524136 |
| Daniel Putra Rywandi S | 13524143 |
| Muh. Hartawan Haidir | 13524147 |


*Program Studi Teknik Informatika — Sekolah Teknik Elektro dan Informatika*<br>
*Institut Teknologi Bandung — Semester II 2025/2026*