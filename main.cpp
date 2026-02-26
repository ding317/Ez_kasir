#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>

#include "mainwindow.h"

/**
 * @brief Fungsi untuk menyiapkan koneksi database SQLite dan membuat tabel.
 */
bool setupDatabase() {
    // Tentukan path database di direktori aplikasi.
    QString dbPath = QCoreApplication::applicationDirPath() + "/local_data.db";

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Gagal membuka database:" << db.lastError().text();
        QMessageBox::critical(nullptr, "Database Error",
                              "Aplikasi tidak dapat terhubung ke database lokal: " + db.lastError().text());
        return false;
    }

    // Perintah CREATE TABLE dengan kolom tambahan (satuan, tanggal_input, tanggal_expired)
    QSqlQuery query;
    QString createTableSql = "CREATE TABLE IF NOT EXISTS barang ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "nama TEXT NOT NULL, "
                             "stok INTEGER NOT NULL, "
                             "satuan TEXT, "
                             "tanggal_input TEXT, "
                             "tanggal_expired TEXT"
                             ")";

    if (!query.exec(createTableSql)) {
        qCritical() << "Gagal membuat tabel 'barang':" << query.lastError().text();
        QMessageBox::critical(nullptr, "Database Error",
                              "Gagal membuat tabel 'barang': " + query.lastError().text());
        return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. Setup Database
    if (!setupDatabase()) {
        return 1;
    }

    // 2. Tampilkan Main Window
    MainWindow w;
    w.show();

    return a.exec();
}
