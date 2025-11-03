#pragma once

#include <QMainWindow>

class QTableWidget;
class QLabel;
class QPushButton;
class QCustomPlot;

class SimulationWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SimulationWindow(QWidget *parent = nullptr);
    ~SimulationWindow() override;

signals:
    // you can emit these later when hooking computation
    void requestRunSimulation();
    void backToMain();

private slots:
    void onLoadParams();
    void onSaveParams();
    void onRunSimulation();
    void onToggleSLD();
    void onBackToMain();

private:
    void setupUi();
    void applyStyle();

    // UI widgets
    QTableWidget *layersTable = nullptr;
    QCustomPlot *plotReflectivity = nullptr;
    QCustomPlot *plotSLD = nullptr;
    QLabel *statusLabel = nullptr;

    QPushButton *btnLoad = nullptr;
    QPushButton *btnSave = nullptr;
    QPushButton *btnRun = nullptr;
    QPushButton *btnSLD = nullptr;
    QPushButton *btnBack = nullptr;

    bool sldVisible = true;
};
