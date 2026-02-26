#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slot untuk operasi CRUD (Create, Update)
    void on_saveButton_clicked();

    // Slot untuk operasi CRUD (Delete)
    void on_deleteButton_clicked();

    // Slot dipicu saat pengguna mengklik baris di QTableView (Pemuatan data untuk Update)
    void on_tableView_clicked(const QModelIndex &index);

    // Slot untuk fitur Pencarian Berdasarkan Nama (Filtering real-time)
    void on_searchNameEdit_textChanged(const QString &text);

    // Slot untuk mereset filter pencarian
    void on_resetSearchButton_clicked();

private:
    Ui::MainWindow *ui;
    QSqlTableModel *itemModel;

    // Variabel untuk menyimpan ID barang yang sedang dipilih (Mode Insert/Update)
    int selectedItemID;

    // Fungsi helper untuk mereset semua input field dan status
    void resetInputFields();
};

#endif // MAINWINDOW_H
