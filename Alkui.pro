# ----------------------------------------------------------------------
# Konfigurasi Dasar Proyek
# ----------------------------------------------------------------------

# Nama executable yang akan dihasilkan
TARGET = AplikasiDataBarang

# Template proyek (aplikasi GUI)
TEMPLATE = app

# Versi standar C++ yang digunakan
CONFIG += c++17

# ----------------------------------------------------------------------
# Modul Qt yang Dibutuhkan
# ----------------------------------------------------------------------

QT += core gui widgets
# Modul yang sangat penting: Diperlukan untuk QSqlDatabase, QSqlQuery, dll.
QT += sql


# ----------------------------------------------------------------------
# Source Files
# ----------------------------------------------------------------------

# Daftar semua file C++ source (.cpp) Anda
SOURCES += \
    main.cpp \
    mainwindow.cpp

# Daftar semua file header (.h) Anda
HEADERS += \
    mainwindow.h

# Daftar file UI (.ui) yang dihasilkan oleh Qt Designer
FORMS += \
    mainwindow.ui
