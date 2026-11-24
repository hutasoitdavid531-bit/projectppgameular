#include <ncurses.h>   // Pertemuan 10 & 14: Ncurses (Linux/CoCalc). Jika di Windows MinGW, ganti jadi <ncurses/curses.h>
#include <unistd.h>    // Pertemuan 14: usleep() untuk animasi & kecepatan ular
#include <fstream>     // Pertemuan 9: File Text (ofstream, ifstream)
#include <string>      // Pertemuan 2 & 5: Tipe data string
#include <cstdlib>     // Untuk rand() dan srand()
#include <ctime>       // Untuk time(0)

using namespace std;

// Konstanta ukuran arena (Pertemuan 2: Tipe data)
const int LEBAR = 40;
const int TINGGI = 20;
const int MAX_ULAR = 100;
const int MAX_DATA = 100;

// --- PERTEMUAN 8: FUNCTION (Void & Return Value) ---

// Animasi Loading (Pertemuan 14: Animasi Teks & Pewarnaan)
void tampilkanLoading() {
    clear();
    attron(COLOR_PAIR(2));
    mvprintw(8, 12, "Memuat Game Snake...");
    mvprintw(9, 10, "[                    ]");
    attroff(COLOR_PAIR(2));
    refresh();

    // Pertemuan 4: For Loop
    for (int i = 1; i <= 20; i++) {
        attron(COLOR_PAIR(1));
        mvprintw(9, 10 + i, "=");
        attroff(COLOR_PAIR(1));
        refresh();
        usleep(50000); // Jeda 0.05 detik
    }
    usleep(300000);
}

// Fungsi Simpan Skor ke File Text (Pertemuan 9: File Text)
void simpanSkor(string nama, int skor) {
    ofstream fileSkor;
    fileSkor.open("leaderboard.txt", ios::app); // ios::app agar data lama tidak terhapus
    if (!fileSkor.fail()) {
        fileSkor << nama << " " << skor << endl;
    }
    fileSkor.close();
}

// Fungsi Leaderboard: Membaca File, Bubble Sort, & Sequential Search (Pertemuan 7 & 9)
void tampilkanLeaderboard() {
    clear();
    ifstream fileSkor;
    fileSkor.open("leaderboard.txt");

    // Pertemuan 5: Array 1 Dimensi
    string namaList[MAX_DATA];
    int skorList[MAX_DATA];
    int jumlahData = 0;

    if (fileSkor.fail()) {
        mvprintw(5, 5, "Belum ada data skor tersimpan!");
        mvprintw(7, 5, "Tekan tombol apa saja untuk kembali...");
        refresh();
        getch();
        return;
    }

    // Membaca data dari file (Pertemuan 4: While Loop)
    while (fileSkor >> namaList[jumlahData] >> skorList[jumlahData]) {
        jumlahData++;
        if (jumlahData >= MAX_DATA) break; // Pertemuan 4: break
    }
    fileSkor.close();

    // Pertemuan 7: Bubble Sort (Mengurutkan skor dari tertinggi ke terendah / Descending)
    for (int i = 0; i < jumlahData - 1; i++) {
        for (int j = 0; j < jumlahData - i - 1; j++) {
            if (skorList[j] < skorList[j + 1]) {
                // Tukar skor
                int tempSkor = skorList[j];
                skorList[j] = skorList[j + 1];
                skorList[j + 1] = tempSkor;

                // Tukar nama agar tetap sesuai
                string tempNama = namaList[j];
                namaList[j] = namaList[j + 1];
                namaList[j + 1] = tempNama;
            }
        }
    }

    // Tampilkan Top 10 di Window Ncurses (Pertemuan 14: Manajemen Window)
    WINDOW *lbWin = newwin(18, 45, 2, 5);
    box(lbWin, 0, 0);
    mvwprintw(lbWin, 1, 13, "=== LEADERBOARD ===");
    
    int batas = (jumlahData < 10) ? jumlahData : 10; // Pertemuan 3: Ternary Operator
    for (int i = 0; i < batas; i++) {
        mvwprintw(lbWin, 3 + i, 4, "%d. %-15s : %d", i + 1, namaList[i].c_str(), skorList[i]);
    }

    mvwprintw(lbWin, 14, 4, "Tekan 'c' untuk cari nama pemain");
    mvwprintw(lbWin, 15, 4, "Tekan tombol lain untuk kembali");
    wrefresh(lbWin);

    int tombol = getch();
    // Pertemuan 3: Percabangan if
    if (tombol == 'c' || tombol == 'C') {
        echo(); // Pertemuan 10: Aktifkan echo agar ketikan terlihat
        char cariNama[30];
        mvwprintw(lbWin, 16, 4, "Masukkan nama: ");
        wrefresh(lbWin);
        wgetstr(lbWin, cariNama);
        noecho();

        // Pertemuan 7: Sequential Search
        bool ketemu = false;
        for (int i = 0; i < jumlahData; i++) {
            if (namaList[i] == string(cariNama)) {
                mvwprintw(lbWin, 16, 4, "Ditemukan! Peringkat %d (Skor: %d)   ", i + 1, skorList[i]);
                ketemu = true;
                break;
            }
        }
        if (!ketemu) {
            mvwprintw(lbWin, 16, 4, "Pemain '%s' tidak ditemukan!       ", cariNama);
        }
        wrefresh(lbWin);
        getch();
    }

    delwin(lbWin); // Pertemuan 14: Hapus window dari memori
}

// Fungsi Utama Game Snake
void mainkanGame() {
    clear();
    // Input nama pemain sebelum mulai (Pertemuan 14: Input teks dengan kursor)
    echo();
    char namaPemain[30];
    mvprintw(5, 5, "Masukkan Nama Pemain (tanpa spasi): ");
    refresh();
    getstr(namaPemain);
    noecho();

    clear();
    refresh();

    // Membuat window arena game (Pertemuan 14: newwin & box)
    WINDOW *arena = newwin(TINGGI, LEBAR, 2, 4);
    keypad(arena, TRUE);
    nodelay(arena, TRUE); // Agar ular tetap bergerak tanpa menunggu tombol ditekan

    // Pertemuan 5: Array koordinat badan ular
    int ularX[MAX_ULAR], ularY[MAX_ULAR];
    int panjangUlar = 3;
    int arah = KEY_RIGHT; // Arah awal ke kanan
    int skor = 0;
    bool gameOver = false;

    // Posisi awal ular di tengah arena
    for (int i = 0; i < panjangUlar; i++) {
        ularX[i] = (LEBAR / 2) - i;
        ularY[i] = TINGGI / 2;
    }

    // Posisi awal makanan
    int makanX = 1 + rand() % (LEBAR - 2);
    int makanY = 1 + rand() % (TINGGI - 2);

    // Pertemuan 4: While Loop utama game
    while (!gameOver) {
        // 1. Baca Input Keyboard (Pertemuan 10: Special Keys)
        int tombol = wgetch(arena);
        // Pertemuan 3: Switch-Case
        switch (tombol) {
            case KEY_UP:
                if (arah != KEY_DOWN) arah = KEY_UP;
                break;
            case KEY_DOWN:
                if (arah != KEY_UP) arah = KEY_DOWN;
                break;
            case KEY_LEFT:
                if (arah != KEY_RIGHT) arah = KEY_LEFT;
                break;
            case KEY_RIGHT:
                if (arah != KEY_LEFT) arah = KEY_RIGHT;
                break;
            case 'q':
            case 'Q':
                gameOver = true;
                break;
        }

        // 2. Geser posisi badan ular dari ekor ke kepala
        for (int i = panjangUlar - 1; i > 0; i--) {
            ularX[i] = ularX[i - 1];
            ularY[i] = ularY[i - 1];
        }

        // Gerakkan kepala ular sesuai arah
        if (arah == KEY_UP) ularY[0]--;
        else if (arah == KEY_DOWN) ularY[0]++;
        else if (arah == KEY_LEFT) ularX[0]--;
        else if (arah == KEY_RIGHT) ularX[0]++;

        // 3. Cek Tabrakan Dinding (Pertemuan 3: If-Else)
        if (ularX[0] <= 0 || ularX[0] >= LEBAR - 1 || ularY[0] <= 0 || ularY[0] >= TINGGI - 1) {
            gameOver = true;
        }

        // Cek Tabrakan Badan Sendiri
        for (int i = 1; i < panjangUlar; i++) {
            if (ularX[0] == ularX[i] && ularY[0] == ularY[i]) {
                gameOver = true;
                break;
            }
        }

        // 4. Cek Makan Makanan
        if (ularX[0] == makanX && ularY[0] == makanY) {
            skor += 10;
            if (panjangUlar < MAX_ULAR) {
                panjangUlar++;
            }
            makanX = 1 + rand() % (LEBAR - 2);
            makanY = 1 + rand() % (TINGGI - 2);
        }

        // 5. Gambar ulang arena
        werase(arena);
        box(arena, 0, 0);

        // Gambar makanan (Warna Merah)
        wattron(arena, COLOR_PAIR(3));
        mvwprintw(arena, makanY, makanX, "*");
        wattroff(arena, COLOR_PAIR(3));

        // Gambar ular (Warna Hijau)
        wattron(arena, COLOR_PAIR(1));
        for (int i = 0; i < panjangUlar; i++) {
            if (i == 0)
                mvwprintw(arena, ularY[i], ularX[i], "O"); // Kepala
            else
                mvwprintw(arena, ularY[i], ularX[i], "o"); // Badan
        }
        wattroff(arena, COLOR_PAIR(1));

        // Info Skor di atas arena
        mvprintw(1, 4, "Pemain: %s | Skor: %d | Tekan 'q' untuk keluar", namaPemain, skor);
        refresh();
        wrefresh(arena);

        usleep(120000); // Kecepatan ular (0.12 detik per langkah)
    }

    delwin(arena);
    simpanSkor(string(namaPemain), skor);

    // Layar Game Over
    clear();
    attron(COLOR_PAIR(3));
    mvprintw(8, 10, "=== GAME OVER ===");
    attroff(COLOR_PAIR(3));
    mvprintw(10, 10, "Skor Akhir %s: %d", namaPemain, skor);
    mvprintw(12, 10, "Tekan tombol apa saja untuk ke menu...");
    refresh();
    getch();
}

// Fungsi Menu Utama (Pertemuan 14: Menu Interaktif)
int tampilkanMenu() {
    const char *opsi[] = {"Mulai Game", "Leaderboard", "Keluar"};
    int jumlahOpsi = 3;
    int pilihanTerpilih = 0;

    WINDOW *menuWin = newwin(10, 30, 5, 10);
    keypad(menuWin, TRUE);

    // Pertemuan 4: Do-While Loop
    do {
        werase(menuWin);
        box(menuWin, 0, 0);
        mvwprintw(menuWin, 1, 7, "=== SNAKE GAME ===");

        for (int i = 0; i < jumlahOpsi; i++) {
            if (i == pilihanTerpilih) {
                wattron(menuWin, A_REVERSE); // Sorotan menu aktif
            }
            mvwprintw(menuWin, 3 + i, 5, "%s", opsi[i]);
            wattroff(menuWin, A_REVERSE);
        }
        mvwprintw(menuWin, 8, 3, "Gunakan Panah & Enter");
        wrefresh(menuWin);

        int tombol = wgetch(menuWin);
        switch (tombol) {
            case KEY_UP:
                pilihanTerpilih = (pilihanTerpilih == 0) ? jumlahOpsi - 1 : pilihanTerpilih - 1;
                break;
            case KEY_DOWN:
                pilihanTerpilih = (pilihanTerpilih == jumlahOpsi - 1) ? 0 : pilihanTerpilih + 1;
                break;
            case 10: // Tombol Enter
                delwin(menuWin);
                return pilihanTerpilih;
        }
    } while (true);
}

int main() {
    srand(time(0));

    // Inisialisasi Ncurses (Pertemuan 10 & 14)
    initscr();
    cbreak();
    noecho();
    curs_set(0); // Sembunyikan kursor
    keypad(stdscr, TRUE);

    // Inisialisasi Warna (Pertemuan 14)
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Ular & Loading
    init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Teks Judul
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Makanan & Game Over

    tampilkanLoading();

    // Loop Menu Utama
    while (true) {
        clear();
        refresh();
        int pilihan = tampilkanMenu();

        if (pilihan == 0) {
            mainkanGame();
        } else if (pilihan == 1) {
            tampilkanLeaderboard();
        } else if (pilihan == 2) {
            break;
        }
    }

    endwin(); // Mengakhiri mode ncurses
    return 0;
}