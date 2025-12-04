#include "../include/laundry.h"

// Helper: Mendapatkan Tanggal Sekarang
void getCurrentDate(char *buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}

// Helper: Konversi Enum ke String untuk Tampilan
const char* getStatusString(Status s) {
    switch (s) {
        case PENDING: return "MENUNGGU";
        case WASHING: return "DICUCI";
        case IRONING: return "DISETRIKA";
        case READY:   return "SIAP DIAMBIL";
        case COMPLETED: return "SELESAI";
        default: return "UNKNOWN";
    }
}

// Helper: Bersihkan Layar (Multi-platform)
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// --- FITUR 1: TERIMA ORDER BARU ---
void createOrder() {
    Order newOrder;
    FILE *file = fopen(FILE_NAME, "ab"); // Append Binary

    if (!file) {
        printf("Error: Gagal membuka database!\n");
        return;
    }

    clearScreen();
    printf("=== TERIMA ORDER BARU ===\n");
    
    // Input Data
    printf("Nama Pelanggan : "); scanf(" %[^\n]", newOrder.name);
    printf("No HP (62xxx)  : "); scanf("%s", newOrder.phone);
    printf("Berat (Kg)     : "); scanf("%f", &newOrder.weight);

    printf("\nJenis Layanan:\n1. Cuci Kering (Rp 4000/kg)\n2. Cuci Komplit (Rp 6000/kg)\n3. Express (Rp 10000/kg)\nPilih: ");
    scanf("%d", &newOrder.service_type);

    // Generate ID Unik (Format: LND-Timestamp)
    sprintf(newOrder.id, "LND-%ld", time(NULL));
    
    // Set Default Status & Date
    newOrder.status = PENDING;
    getCurrentDate(newOrder.date_in);

    // Hitung Harga
    float pricePerKg = 0;
    if (newOrder.service_type == 1) pricePerKg = 4000;
    else if (newOrder.service_type == 2) pricePerKg = 6000;
    else pricePerKg = 10000;
    
    newOrder.total_price = newOrder.weight * pricePerKg;

    // Simpan ke File
    fwrite(&newOrder, sizeof(Order), 1, file);
    fclose(file);

    printf("\n[SUCCESS] Order berhasil disimpan!\n");
    printf("ID Order: %s | Total: Rp %.2f\n", newOrder.id, newOrder.total_price);
    
    // Integrasi WhatsApp (Metode Buka Browser)
    char msg[200];
    sprintf(msg, "Halo *%s*, laundry Anda diterima.%%0A"
                 "ID: *%s*%%0A"
                 "Total: Rp %.0f%%0A"
                 "Status: MENUNGGU.", 
            newOrder.name, newOrder.id, newOrder.total_price);
    sendWhatsApp(newOrder.phone, msg);

    printf("\nTekan Enter kembali ke menu...");
    getchar(); getchar();
}

// --- FITUR 2: LIHAT SEMUA DATA ---
void viewOrders() {
    FILE *file = fopen(FILE_NAME, "rb");
    Order o;

    if (!file) {
        printf("Belum ada data transaksi.\n");
        return;
    }

    clearScreen();
    printf("=== DAFTAR ANTRIAN LAUNDRY ===\n");
    printf("%-15s %-20s %-10s %-15s %-15s\n", "ID Order", "Nama", "Berat", "Status", "Total (Rp)");
    printf("------------------------------------------------------------------------------\n");

    while (fread(&o, sizeof(Order), 1, file)) {
        // Hanya tampilkan yang belum diambil (COMPLETED tidak ditampilkan agar list rapi)
        if (o.status != COMPLETED) {
            printf("%-15s %-20s %-5.1f Kg   %-15s Rp %-10.0f\n", 
                o.id, o.name, o.weight, getStatusString(o.status), o.total_price);
        }
    }
    printf("------------------------------------------------------------------------------\n");
    fclose(file);
    printf("\nTekan Enter kembali ke menu...");
    getchar(); getchar();
}

// --- FITUR 3: UPDATE STATUS PENGERJAAN ---
void updateStatus() {
    FILE *file = fopen(FILE_NAME, "rb");
    FILE *temp = fopen(TEMP_FILE, "wb");
    Order o;
    char searchID[20];
    int found = 0;

    if (!file || !temp) return;

    clearScreen();
    printf("=== UPDATE STATUS PENGERJAAN ===\n");
    printf("Masukkan ID Order: "); scanf("%s", searchID);

    while (fread(&o, sizeof(Order), 1, file)) {
        if (strcmp(o.id, searchID) == 0) {
            found = 1;
            printf("\nData Ditemukan: %s (%s)\n", o.name, getStatusString(o.status));
            printf("Update Status ke:\n1. DICUCI\n2. DISETRIKA\n3. SIAP DIAMBIL\nPilih: ");
            int pil;
            scanf("%d", &pil);
            
            if (pil == 1) o.status = WASHING;
            else if (pil == 2) o.status = IRONING;
            else if (pil == 3) o.status = READY;
            
            // Kirim Notifikasi Update
            char msg[200];
            sprintf(msg, "Halo %s.%%0A"
                "Status laundry Anda sekarang: _%s._", o.name, getStatusString(o.status));
            sendWhatsApp(o.phone, msg);
        }
        fwrite(&o, sizeof(Order), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove(FILE_NAME);
    rename(TEMP_FILE, FILE_NAME);

    if (found) printf("\n[SUCCESS] Status berhasil diupdate!\n");
    else printf("\n[ERROR] ID tidak ditemukan.\n");
    
    getchar(); getchar();
}

// --- FITUR 4: TRANSAKSI PENGAMBILAN (KASIR) ---
void completeOrder() {
    FILE *file = fopen(FILE_NAME, "rb");
    FILE *temp = fopen(TEMP_FILE, "wb");
    Order o;
    char searchID[20];
    int found = 0;
    float bayar, kembali;

    if (!file || !temp) return;

    clearScreen();
    printf("=== PENGAMBILAN LAUNDRY ===\n");
    printf("Masukkan ID Order: "); scanf("%s", searchID);

    while (fread(&o, sizeof(Order), 1, file)) {
        if (strcmp(o.id, searchID) == 0 && o.status != COMPLETED) {
            found = 1;
            printf("\nRincian Tagihan:\n");
            printf("Nama: %s\nTotal: Rp %.2f\n", o.name, o.total_price);
            
            printf("Input Uang Bayar: Rp ");
            scanf("%f", &bayar);

            if (bayar >= o.total_price) {
                kembali = bayar - o.total_price;
                printf("Kembalian: Rp %.2f\n", kembali);
                o.status = COMPLETED; // Tandai selesai
                printf("[SUCCESS] Transaksi Selesai. Terima kasih!\n");
                
                char msg[200];
                sprintf(msg, "Terima kasih %s.%%0A" "Laundry sudah diambil. *Lunas.*", o.name);
                sendWhatsApp(o.phone, msg);
            } else {
                printf("[ERROR] Uang kurang! Transaksi dibatalkan.\n");
            }
        }
        fwrite(&o, sizeof(Order), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove(FILE_NAME);
    rename(TEMP_FILE, FILE_NAME);

    if (!found) printf("\nID tidak ditemukan atau sudah diambil.\n");
    getchar(); getchar();
}

// --- FITUR NOTIFIKASI WHATSAPP (METODE BROWSER) ---
void sendWhatsApp(char* phone, char* message) {
    char url[512];
    char command[600];
    char cleanMsg[300];
    
    // Ganti spasi dengan %20 manual (Sederhana)
    int j = 0;
    for(int i = 0; message[i] != '\0'; i++) {
        if(message[i] == ' ') {
            cleanMsg[j++] = '%'; cleanMsg[j++] = '2'; cleanMsg[j++] = '0';
        } else {
            cleanMsg[j++] = message[i];
        }
    }
    cleanMsg[j] = '\0';

    sprintf(url, "https://wa.me/%s?text=%s", phone, cleanMsg);
    
    printf("\n[SYSTEM] Membuka WhatsApp untuk kirim pesan...\n");
    
    #ifdef _WIN32
        sprintf(command, "start \"\" \"%s\"", url);
    #else
        sprintf(command, "xdg-open \"%s\"", url);
    #endif
    
    system(command);
}