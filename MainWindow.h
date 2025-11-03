#pragma once

#include <QMainWindow>

class QTextBrowser;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSimulate();
    void onFitData();
    void onExit();
    void onHelp();

private:
    void setupUi();
    void applyStyle();

    // UI widgets
    QLabel *logoLabel = nullptr;
    QTextBrowser *disclaimerBrowser = nullptr;
    QPushButton *btnSimulate = nullptr;
    QPushButton *btnFit = nullptr;
    QPushButton *btnExit = nullptr;
    QPushButton *btnHelp = nullptr;

    bool simcurves = false;
};
