# KOI-Tubes-IF2224-2026

## Program Description

Program ini adalah implementasi **Compiler Pipeline** lengkap untuk bahasa pemrograman *Arion* — sebuah bahasa Pascal-like yang dirancang untuk keperluan akademik. Pipeline mencakup:

1. **Milestone 1 — Lexical Analyzer**: Membaca source code `.txt`, melakukan tokenisasi, dan menghasilkan daftar token.
2. **Milestone 2 — Parser**: Membangun *Parse Tree* dari daftar token menggunakan metode *recursive descent parsing*.
3. **Milestone 3 — Semantic Analyzer**: Membangun *Decorated AST* dari Parse Tree, melakukan pengecekan tipe, dan membangun *Symbol Table*.
4. **Milestone 4 — Intermediate Code Generator & Interpreter**: Mengubah Decorated AST menjadi *Three-Address Code (TAC)* lalu mengeksekusinya menggunakan *Stack Machine Interpreter*.

---

## Requirements

Untuk menjalankan program ini, pastikan perangkat Anda memenuhi kebutuhan berikut:

1. **C++ Compiler** dengan dukungan standar C++:
   - **Windows**: MinGW (g++) atau MSVC
   - **Linux (Ubuntu/WSL)**: GCC (g++)
2. **Make**: Untuk menjalankan build otomatis via `makefile`
3. **File Input**: File `.txt` berisi source code Arion

---

## How to Compile and Run

```bash
make run
```

---

## Milestone 4 — Intermediate Code Generator & Interpreter

### Deskripsi

Milestone 4 menambahkan dua komponen baru ke pipeline:

**Intermediate Code Generator (ICG)**
Mengubah Decorated AST hasil Milestone 3 menjadi *Three-Address Code (TAC)* — representasi instruksi sekuensial berbasis stack yang platform-independent. Menggunakan pendekatan *DFS-based traversal* pada AST.

**Stack Machine Interpreter**
Membaca daftar instruksi TAC dan mengeksekusinya menggunakan *Stack Machine* dengan siklus **Fetch → Decode → Execute**. Memori program direpresentasikan sebagai flat array dengan manajemen frame berbasis *base pointer stack*.

### Instruksi TAC yang Didukung

| Instruksi | Format | Keterangan |
|-----------|--------|------------|
| `INT` | `INT 0 m` | Alokasi frame memori ukuran `m` |
| `LIT` | `LIT 0 v` | Push literal `v` ke stack |
| `LOD` | `LOD 0 a` | Load nilai dari address `a`, push ke stack |
| `STO` | `STO 0 a` | Pop dari stack, simpan ke address `a` |
| `OPR` | `OPR 0 o` | Jalankan operasi `o` |
| `JMP` | `JMP 0 l` | Unconditional jump ke baris `l` |
| `JPC` | `JPC 0 l` | Jump ke baris `l` jika kondisi **false** |
| `CAL` | `CAL 0 l` | Panggil fungsi/prosedur di baris `l` |
| `RET` | `RET 0 0` | Return dari fungsi/prosedur |

### Operasi OPR yang Didukung

| No. | Operasi | Keterangan |
|-----|---------|------------|
| 1 | `NEG` | Negasi unary |
| 2 | `ADD` | Penjumlahan |
| 3 | `SUB` | Pengurangan |
| 4 | `MUL` | Perkalian |
| 5 | `DIV` | Pembagian integer |
| 6 | `MOD` | Modulus |
| 7 | `EQL` | Sama dengan (`=`) |
| 8 | `NEQ` | Tidak sama dengan (`<>`) |
| 9 | `LSS` | Kurang dari (`<`) |
| 10 | `GEQ` | Lebih dari atau sama dengan (`>=`) |
| 11 | `GTR` | Lebih dari (`>`) |
| 12 | `LEQ` | Kurang dari atau sama dengan (`<=`) |
| 13 | `WRT` | `write` — cetak tanpa newline |
| 14 | `WRTLN` | `writeln` — cetak dengan newline |

### Struktur Frame Memori

Setiap pemanggilan fungsi/prosedur mengalokasikan sebuah frame dengan layout:

```
base+0 : Static Link
base+1 : Dynamic Link  (sp caller)
base+2 : Return Address (pc setelah CAL)
base+3 : Variabel lokal pertama
base+4 : Variabel lokal kedua
  ...
```

Sehingga program dengan `n` variabel lokal menggunakan `INT 0 (3 + n)`.

### Deteksi Vulnerability (Bonus)

Interpreter dilengkapi deteksi kerentanan runtime:

| Vulnerability | Kondisi |
|---------------|---------|
| Stack Overflow | Kedalaman frame melebihi batas |
| Stack Underflow | Pop dari stack kosong |
| Out-of-Bounds Access | Akses alamat di luar stack aktif |
| Invalid Jump Target | Lompat ke baris yang tidak ada |
| Numerical Overflow | Hasil operasi melebihi `INT_MAX` |
| Numerical Underflow | Hasil operasi kurang dari `INT_MIN` |

---

## File Structure

```
KOI-Tubes-IF2224-2026/
├── src/
│   ├── main.cpp
│   ├── lexer.hpp / lexer.cpp
│   ├── parser.hpp / parser.cpp
│   ├── parser_declaration.cpp
│   ├── expression.cpp
│   ├── semantic.hpp / semantic.cpp
│   ├── semantic_decl.cpp
│   ├── symbol_table.hpp / symbol_table.cpp
│   ├── ast_node.hpp / ast_node.cpp
│   ├── parse_tree_node.hpp / parse_tree_node.cpp
│   ├── parse_tree_reader.hpp / parse_tree_reader.cpp
│   ├── intermediate_code.hpp / intermediate_code.cpp    # Milestone 4
│   ├── interpreter.hpp / interpreter.cpp                # Milestone 4
│   ├── token.hpp / token.cpp
│   ├── identifier.hpp / identifier.cpp
│   └── operator.hpp / operator.cpp
├── test/
│   ├── milestone1/
│   ├── milestone2/
│   ├── milestone3/
│   ├── milestone4/                                      # Milestone 4
│   ├── hello_parse_tree.txt
│   ├── hello_semantic.txt
│   ├── hello_solusi.txt
│   └── hello.txt
├── makefile
└── README.md
```

---

## Division of Tasks — Milestone 4

| No. | Nama | Deskripsi Tugas |
|-----|------|-----------------|
| 1. | Nama 1 | ICG Core: class `IntermediateCodeGenerator`, entry point program, generator deklarasi fungsi/prosedur (`CAL`, `RET`, manajemen stack frame) |
| 2. | Nama 2 | ICG Control Flow: generator IF/ELSE, WHILE, FOR, REPEAT-UNTIL, sistem backpatching label (`JMP`, `JPC`), integrasi `main.cpp` |
| 3. | Nama 3 | ICG Expressions & Statements: generator ekspresi (BinOp, UnOp, literal), Assign, write/writeln, variable load/store (`LIT`, `LOD`, `STO`, `OPR`) |
| 4. | Zahran Alvan Putra Winarko | Interpreter (Stack Machine): manajemen frame memori runtime, siklus Fetch→Decode→Execute, semua instruksi TAC, semua operasi OPR, deteksi vulnerability (bonus) |

---

## Contributors

| Nama | NIM |
|------|-----|
| Zahran Alvan P W | 13524124 |
| Neswa Eka Anggara | 13524136 |
| Daniel Putra Rywandi S | 13524143 |
| Muh. Hartawan Haidir | 13524147 |

*Program Studi Teknik Informatika — Sekolah Teknik Elektro dan Informatika*<br>
*Institut Teknologi Bandung — Semester II 2025/2026*