#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QDate>
#include <QItemSelectionModel>

// --- KONSTRUKTOR MAINWINDOW ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , selectedItemID(0) // Mode Insert default
{
    ui->setupUi(this);

    // --- SETUP QCOMBOBOX SATUAN (SOLUSI MASALAH SATUAN KOSONG) ---
    ui->satuanComboBox->addItem("Pcs");
    ui->satuanComboBox->addItem("Unit");
    ui->satuanComboBox->addItem("Liter");
    ui->satuanComboBox->addItem("Kg");
    ui->satuanComboBox->addItem("Box");
    ui->satuanComboBox->addItem("rim");


    // --- 1. SETUP MODEL DAN VIEW DATA ---
    QSqlDatabase db = QSqlDatabase::database();

    itemModel = new QSqlTableModel(this, db);
    itemModel->setTable("barang");

    // Set Header
    itemModel->setHeaderData(1, Qt::Horizontal, "Nama Barang");
    itemModel->setHeaderData(2, Qt::Horizontal, "Stok");
    itemModel->setHeaderData(3, Qt::Horizontal, "Satuan");
    itemModel->setHeaderData(4, Qt::Horizontal, "Tanggal Input");
    itemModel->setHeaderData(5, Qt::Horizontal, "Kadaluarsa");

    itemModel->select();

    // Hubungkan Model ke QTableView
    ui->tableView->setModel(itemModel);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->hideColumn(0); // Sembunyikan kolom 'id'
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Atur QDateEdit default
    ui->inputDateEdit->setDate(QDate::currentDate());

    // --- 2. KONEKSI SINYAL/SLOT ---
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::on_saveButton_clicked);
    connect(ui->delateButton, &QPushButton::clicked, this, &MainWindow::on_deleteButton_clicked);
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::on_tableView_clicked);
    connect(ui->searchNameEdit, &QLineEdit::textChanged, this, &MainWindow::on_searchNameEdit_textChanged);
    connect(ui->resetSearchButton, &QPushButton::clicked, this, &MainWindow::on_resetSearchButton_clicked);

    resetInputFields();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- FUNGSI HELPER ---

void MainWindow::resetInputFields()
{
    ui->namaLineEdit->clear();
    ui->stokSpinBox->setValue(0);
    ui->satuanComboBox->setCurrentIndex(0);
    ui->inputDateEdit->setDate(QDate::currentDate());
    ui->expiredDateEdit->setDate(QDate::currentDate().addMonths(6));

    selectedItemID = 0;
    ui->saveButton->setText("Simpan Data Baru");
}


// --- SLOT IMPLEMENTASI CRUD ---

// Insert atau Update Data (SOLUSI PARAMETER COUNT MISMATCH)
void MainWindow::on_saveButton_clicked()
{
    QString nama = ui->namaLineEdit->text().trimmed();
    int stok = ui->stokSpinBox->value();
    QString satuan = ui->satuanComboBox->currentText();
    QString tglInput = ui->inputDateEdit->date().toString(Qt::ISODate);
    QString tglExpired = ui->expiredDateEdit->date().toString(Qt::ISODate);

    if (nama.isEmpty() || stok <= 0) {
        QMessageBox::warning(this, "Input Tidak Valid", "Nama dan Stok harus diisi dengan benar.");
        return;
    }

    QSqlQuery query;
    bool success = false;

    if (selectedItemID > 0) {
        // --- MODE UPDATE ---
        query.prepare("UPDATE barang SET nama = :nama, stok = :stok, satuan = :satuan, "
                      "tanggal_input = :tglIn, tanggal_expired = :tglExp WHERE id = :id");
        query.bindValue(":id", selectedItemID); // Binding ID harus di sini
    } else {
        // --- MODE INSERT ---
        query.prepare("INSERT INTO barang (nama, stok, satuan, tanggal_input, tanggal_expired) "
                      "VALUES (:nama, :stok, :satuan, :tglIn, :tglExp)");
    }

    // --- Binding umum (Dilakukan sekali setelah prepare) ---
    query.bindValue(":nama", nama);
    query.bindValue(":stok", stok);
    query.bindValue(":satuan", satuan);
    query.bindValue(":tglIn", tglInput);
    query.bindValue(":tglExp", tglExpired);

    success = query.exec(); // Eksekusi query

    if (success) {
        QMessageBox::information(this, "Sukses", selectedItemID > 0 ? "Data berhasil diperbarui!" : "Data barang berhasil disimpan!");
        itemModel->select();
        resetInputFields();
    } else {
        QMessageBox::critical(this, "Kesalahan Database", "Gagal melakukan operasi: " + query.lastError().text());
    }
}

// Hapus Data (SOLUSI CRASH)
void MainWindow::on_deleteButton_clicked() {
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) {
        QMessageBox::warning(this, "Hapus Data", "Pilih baris di tabel yang ingin dihapus terlebih dahulu.");
        return;
    }

    if (QMessageBox::question(this, "Konfirmasi", "Anda yakin ingin menghapus data yang dipilih?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // Ambil index baris yang dipilih
    QModelIndex index = selection->selectedRows().first();
    int rowToDelete = index.row();

    // --- Hapus menggunakan QSqlTableModel (Lebih aman dari crash) ---
    if (itemModel->removeRow(rowToDelete)) {
        if (itemModel->submitAll()) {
            QMessageBox::information(this, "Hapus Data", "Data berhasil dihapus.");
        } else {
            QMessageBox::critical(this, "Error Database", "Gagal menghapus data dari database.");
            itemModel->revertAll(); // Batalkan penghapusan jika submit gagal
        }
    } else {
        QMessageBox::critical(this, "Error Model", "Gagal menghapus baris dari model.");
    }

    itemModel->select(); // Selalu refresh
    resetInputFields();
}

// Memuat Data ke Form Input Saat Baris Diklik
void MainWindow::on_tableView_clicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    int row = index.row();

    if (itemModel->rowCount() > row) {
        // Ambil ID dari kolom 0
        selectedItemID = itemModel->data(itemModel->index(row, 0)).toInt();

        // Muat data ke input fields (Asumsi kolom 1=nama, 2=stok, 3=satuan, 4=tgl_input, 5=tgl_expired)
        ui->namaLineEdit->setText(itemModel->data(itemModel->index(row, 1)).toString());
        ui->stokSpinBox->setValue(itemModel->data(itemModel->index(row, 2)).toInt());

        // Muat Satuan
        QString currentSatuan = itemModel->data(itemModel->index(row, 3)).toString();
        int comboIndex = ui->satuanComboBox->findText(currentSatuan);
        if (comboIndex >= 0) {
            ui->satuanComboBox->setCurrentIndex(comboIndex);
        } else {
            // Jika satuan dari DB tidak ada di list Combo Box, tambahkan sementara
            ui->satuanComboBox->addItem(currentSatuan);
            ui->satuanComboBox->setCurrentText(currentSatuan);
        }

        ui->inputDateEdit->setDate(QDate::fromString(itemModel->data(itemModel->index(row, 4)).toString(), Qt::ISODate));
        ui->expiredDateEdit->setDate(QDate::fromString(itemModel->data(itemModel->index(row, 5)).toString(), Qt::ISODate));

        ui->saveButton->setText(QString("Update ID: %1").arg(selectedItemID));
    }
}


// --- SLOT IMPLEMENTASI FILTERING ---

// Pencarian Berdasarkan Nama Barang (Real-time)
void MainWindow::on_searchNameEdit_textChanged(const QString &text) {
    QString filter;
    QString searchText = text.trimmed();

    if (searchText.isEmpty()) {
        filter = "cari?";
    } else {
        // Filter case-insensitive
        filter = QString("nama LIKE '%%1%'").arg(searchText);
    }

    itemModel->setFilter(filter);
    itemModel->select();
}

// Reset Pencarian
void MainWindow::on_resetSearchButton_clicked() {
    ui->searchNameEdit->clear();
    itemModel->setFilter("");
    itemModel->select();
}
