# --- BAGIAN 1: DEFINISI VARIABEL ---
# CC: Compiler yang digunakan
CC = gcc

# CFLAGS: Opsi kompilasi
# -Wall : Tampilkan semua peringatan (Warning All) agar kode rapi
# -I./include : Beritahu gcc untuk mencari file header (.h) di folder include
CFLAGS = -Wall -I./include

# LDFLAGS: Linker flags (untuk library tambahan)
# -lcurl wajib ada jika Anda memakai fitur WhatsApp dengan cURL
LDFLAGS = -lcurl

# TARGET: Nama file aplikasi hasil kompilasi (executable)
TARGET = laundry_app

# SRC: Daftar semua file .c yang Anda buat
SRC = src/main.c src/order.c

# --- BAGIAN 2: ATURAN (RULES) ---

# Aturan Utama: Cara membuat aplikasi
# Formatnya:
# target: dependencies
# [TAB] perintah
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo "Komplit! Aplikasi berhasil dibuat: $(TARGET)"

# Aturan 'run': Untuk compile sekaligus menjalankan aplikasi
run: $(TARGET)
	./$(TARGET)

# Aturan 'clean': Membersihkan file hasil kompilasi (biar bersih)
# Berguna jika ingin compile ulang dari nol

# Khusus Windows (jika pakai Windows, perintah hapusnya beda)
clean_win:
	del $(TARGET).exe
	@echo "File exe dihapus."