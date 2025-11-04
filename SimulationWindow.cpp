#include "SimulationWindow.h"
#include "version.h" // optional if you want to include version in title

#include "qcustomplot.h" // ensure qcustomplot.h is in project folder

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

SimulationWindow::SimulationWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("%1 – Simulation Mode").arg(QStringLiteral(APP_NAME)));
    resize(1100, 700);

    setupUi();
    applyStyle();
}

SimulationWindow::~SimulationWindow() = default;

void SimulationWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *rootV = new QVBoxLayout(central);
    rootV->setContentsMargins(10,10,10,10);
    rootV->setSpacing(8);

    // --- Top toolbar ---
    QLabel *interfaceLabel = new QLabel("Interface:");
    QComboBox *interfaceCombo = new QComboBox;
    interfaceCombo->addItems({"Air-Liquid", "Solid-Liquid", "Liquid-Solid", "Liquid-Liquid", "Air-Solid"});
    interfaceCombo->setCurrentIndex(0); // default

    QHBoxLayout *interfaceLayout = new QHBoxLayout;
    interfaceLayout->addWidget(interfaceLabel);
    interfaceLayout->addWidget(interfaceCombo);
    interfaceLayout->addStretch();
    rootV->addLayout(interfaceLayout);


    QLabel *rhoLabel = new QLabel("<b>ρ<sub>air</sub></b> (10⁻⁶ Å⁻²):");
    QLineEdit *rhoEdit = new QLineEdit("0.0"); // default SLD of air
    rhoEdit->setFixedWidth(80);


    QLabel *qResLabel = new QLabel("Q-resolution (%):");
    QLineEdit *qResEdit = new QLineEdit;
    qResEdit->setFixedWidth(60);
    qResEdit->setText("0.5"); // default value

    interfaceLayout->addWidget(rhoLabel);
    interfaceLayout->addWidget(rhoEdit);
    interfaceLayout->addWidget(qResLabel);
    interfaceLayout->addWidget(qResEdit);

    QHBoxLayout *toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);

    btnLoad = new QPushButton("Load Params");
    btnSave = new QPushButton("Save Params");
    btnRun  = new QPushButton("Run Simulation");
    btnSLD  = new QPushButton("Show/Hide SLD");
    btnBack = new QPushButton("Back to Main");

    // smaller help-style for back if needed
    btnBack->setFixedWidth(120);

    toolbar->addWidget(btnLoad);
    toolbar->addWidget(btnSave);
    toolbar->addSpacing(10);
    toolbar->addWidget(btnRun);
    toolbar->addWidget(btnSLD);
    toolbar->addStretch(1);
    toolbar->addWidget(btnBack);

    rootV->addLayout(toolbar);


    // --- Main splitter: left (table) / right (plots) ---
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    // Left: layers table
    QWidget *leftPanel = new QWidget;
    QVBoxLayout *leftV = new QVBoxLayout(leftPanel);
    leftV->setContentsMargins(6,6,6,6);
    leftV->setSpacing(6);

    QLabel *layersLabel = new QLabel("<b>Layer parameters</b>");
    layersTable = new QTableWidget;
    layersTable->setColumnCount(5);
    QStringList headers = { "Layer", "t (Å)", "\u03C3 (Å)", "\u03C1 (10⁻⁶ Å⁻²)", "vfw (%)" };
    layersTable->setHorizontalHeaderLabels(headers);
    layersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layersTable->verticalHeader()->setVisible(false);
    layersTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    layersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layersTable->setMinimumWidth(380);

    // Add a few sample rows so UI doesn't look empty
    layersTable->setRowCount(6);
    for (int r=0; r<6; ++r) {
        layersTable->setItem(r,0, new QTableWidgetItem(QString("Layer %1").arg(r+1)));
        layersTable->setItem(r,1, new QTableWidgetItem("10"));
        layersTable->setItem(r,2, new QTableWidgetItem("3"));
        layersTable->setItem(r,3, new QTableWidgetItem("2.07"));
        layersTable->setItem(r,4, new QTableWidgetItem("0"));
    }

    leftV->addWidget(layersLabel);
    leftV->addWidget(layersTable);
    leftV->addStretch();

    mainSplitter->addWidget(leftPanel);

    // Right: two stacked plots (QCustomPlot)
    QWidget *rightPanel = new QWidget;
    QVBoxLayout *rightV = new QVBoxLayout(rightPanel);
    rightV->setContentsMargins(6,6,6,6);
    rightV->setSpacing(8);

    QLabel *plotRLabel = new QLabel("<b>Reflectivity R(Q)</b>");
    plotReflectivity = new QCustomPlot;
    plotReflectivity->setMinimumHeight(300);
    // placeholder axes
    plotReflectivity->addGraph();
    plotReflectivity->xAxis->setLabel("Q (Å⁻¹)");
    plotReflectivity->yAxis->setLabel("R(Q)");
    plotReflectivity->xAxis->setRange(0, 0.5);
    plotReflectivity->yAxis->setRange(1e-8, 1);

    QLabel *plotSLDLabel = new QLabel("<b>SLD Profile</b>");
    plotSLD = new QCustomPlot;
    plotSLD->setMinimumHeight(250);
    plotSLD->addGraph();
    plotSLD->xAxis->setLabel("Depth (Å)");
    plotSLD->yAxis->setLabel("SLD (10⁻⁶ Å⁻²)");
    plotSLD->xAxis->setRange(0, 300);
    plotSLD->yAxis->setRange(0, 6);

    rightV->addWidget(plotRLabel);
    rightV->addWidget(plotReflectivity, 1);
    rightV->addWidget(plotSLDLabel);
    rightV->addWidget(plotSLD, 1);

    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    rootV->addWidget(mainSplitter, 1);

    // --- Bottom status label ---
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: #555;");
    rootV->addWidget(statusLabel);

    // --- Connections (placeholders) ---
    connect(btnLoad, &QPushButton::clicked, this, &SimulationWindow::onLoadParams);
    connect(btnSave, &QPushButton::clicked, this, &SimulationWindow::onSaveParams);
    connect(btnRun,  &QPushButton::clicked, this, &SimulationWindow::onRunSimulation);
    connect(btnSLD,  &QPushButton::clicked, this, &SimulationWindow::onToggleSLD);
    connect(btnBack, &QPushButton::clicked, this, &SimulationWindow::onBackToMain);
    connect(interfaceCombo, &QComboBox::currentTextChanged, this, [rhoLabel](const QString &text){
        if (text.contains("Air")) rhoLabel->setText("<b>ρ<sub>air</sub></b> (10⁻⁶ Å⁻²):");
        else if (text.contains("Solid")) rhoLabel->setText("<b>ρ<sub>solid</sub></b> (10⁻⁶ Å⁻²):");
        else if (text.contains("Liquid")) rhoLabel->setText("<b>ρ<sub>liquid</sub></b> (10⁻⁶ Å⁻²):");
    });

}

void SimulationWindow::applyStyle()
{
    // simple modern styles — adapt as desired
    QString style = R"(
        QWidget { background-color: #ffffff; color: #222; font-family: 'Segoe UI', Arial, sans-serif; }
        QTableWidget { background: #fbfbfb; border: 1px solid #eaeaea; }
        QPushButton {
            background-color: #0078D7;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 12px;
            font-weight: 600;
        }
        QPushButton:hover { background-color: #2894FF; }
        QLabel { color: #333; }
    )";
    qApp->setStyleSheet(style);
}

void SimulationWindow::onLoadParams()
{
    QString fname = QFileDialog::getOpenFileName(this, tr("Load parameters"), QDir::currentPath(), tr("Parameter files (*.json *.txt *.mat);;All Files (*)"));
    if (fname.isEmpty()) return;
    // TODO: implement parameter parsing. For now:
    statusLabel->setText(QString("Loaded parameters: %1").arg(fname));
    QMessageBox::information(this, tr("Load Parameters"), tr("Parameter file chosen:\n%1").arg(fname));
}

void SimulationWindow::onSaveParams()
{
    QString fname = QFileDialog::getSaveFileName(this, tr("Save parameters"), QDir::currentPath(), tr("JSON files (*.json);;All Files (*)"));
    if (fname.isEmpty()) return;
    statusLabel->setText(QString("Saved parameters: %1").arg(fname));
    QMessageBox::information(this, tr("Save Parameters"), tr("Saving not implemented yet.\nFile chosen:\n%1").arg(fname));
}

void SimulationWindow::onRunSimulation()
{
    // placeholder: in future, call the C++ engine to compute reflectivity and SLD
    statusLabel->setText("Running simulation...");
    qDebug() << "Run simulation requested (placeholder).";

    // Example plotting of fake data so UI shows something:
    QVector<double> x(200), y(200);
    for (int i=0; i<200; ++i) {
        x[i] = i * 0.002;
        y[i] = pow(10.0, -6.0 - 6.0 * x[i]); // fake decaying reflectivity
    }
    plotReflectivity->graph(0)->setData(x, y);
    plotReflectivity->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plotReflectivity->replot();

    QVector<double> z(100), sld(100);
    for (int i=0; i<100; ++i) {
        z[i] = i * 3.0;
        sld[i] = 2.0 + 0.02 * i; // fake linear SLD
    }
    plotSLD->graph(0)->setData(z, sld);
    plotSLD->replot();

    statusLabel->setText("Simulation completed (placeholder).");
}

void SimulationWindow::onToggleSLD()
{
    sldVisible = !sldVisible;
    plotSLD->setVisible(sldVisible);
    statusLabel->setText(sldVisible ? "SLD visible" : "SLD hidden");
}

void SimulationWindow::onBackToMain()
{
    // Close this window and open the main menu again.
    // We will simply close this window; the MainWindow can be recreated by the caller if needed.
    close();
    emit backToMain();
}
