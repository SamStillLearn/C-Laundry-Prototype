#include "../include/laundry.h"

int main() {
    int choice;

    // Pastikan folder data ada (Opsional, tapi praktik bagus)
    #ifdef _WIN32
        system("mkdir data 2> NUL");
    #else
        system("mkdir -p data");
    #endif

    while (1) {
        clearScreen();
        printf("=====================================\n");
        printf("    APLIKASI MANAJEMEN LAUNDRY v1.0  \n");
        printf("=====================================\n");
        printf("1. Terima Order Baru (Masuk)\n");
        printf("2. Update Status Pengerjaan\n");
        printf("3. Lihat Daftar Antrian\n");
        printf("4. Pengambilan & Pembayaran\n");
        printf("5. Keluar\n");
        printf("=====================================\n");
        printf("Pilihan Anda [1-5]: ");
        
        if (scanf("%d", &choice) != 1) {
            // Error handling jika input bukan angka
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                createOrder();
                break;
            case 2:
                updateStatus();
                break;
            case 3:
                viewOrders();
                break;
            case 4:
                completeOrder();
                break;
            case 5:
                printf("\nTerima kasih telah menggunakan aplikasi ini.\n");
                return 0;
            default:
                printf("\nPilihan tidak valid. Tekan Enter...");
                getchar(); getchar();
        }
    }
    return 0;
}