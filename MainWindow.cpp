#include "MainWindow.h"
#include "version.h"   // must define APP_NAME and APP_VERSION (see note)

#include <QApplication>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include "SimulationWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Title uses version info from version.h
    setWindowTitle(QString("%1 v%2").arg(APP_NAME).arg(APP_VERSION));

    // reasonable size
    resize(600, 650);
    setupUi();
    applyStyle();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainV = new QVBoxLayout(central);
    mainV->setContentsMargins(18, 18, 18, 18);
    mainV->setSpacing(8);

    // --- Logo (try resource first, then file) ---
    logoLabel = new QLabel;
    QPixmap pix(":/Aurore_Logo_v7.png"); // from resources.qrc
    if (pix.isNull()) {
        // fallback to file in working directory
        pix.load("Aurore_Logo_v7.png");
    }
    if (!pix.isNull()) {
        logoLabel->setPixmap(pix.scaledToHeight(160, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
        mainV->addWidget(logoLabel);
    }

    // --- Title + subtitle ---
 //   QLabel *title = new QLabel(QString("<h2>%1</h2>").arg(APP_NAME));
 //   title->setAlignment(Qt::AlignCenter);
 //   mainV->addWidget(title);

    QLabel *subtitle = new QLabel("Neutron Reflectometry — data analysis & simulation");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color: #555; margin-bottom: 3px;");
    mainV->addWidget(subtitle);

    // --- Disclaimer / long text area (scrollable) ---
    disclaimerBrowser = new QTextBrowser;
    disclaimerBrowser->setReadOnly(true);
    disclaimerBrowser->setOpenExternalLinks(true);

    // Converted/cleaned disclaimer text (from your MATLAB version)
    QString disclaimerHtml = R"(
<div style="line-height:1.4; font-family:'Segoe UI','Helvetica Neue',Arial,sans-serif; color:#222;">

  <h3 style="text-align:center; margin:0; padding:0;">Disclaimer</h3>

  <p style="text-align:center; margin:5px 5px;">
    This software is released under a
    <b>Creative Commons Attribution Non-Commercial License v2.0</b>.<br>
    Use of the software should be acknowledged by citing the following reference:
  </p>

  <p style="text-align:center; margin:6px 8px;">
    <b>Y. Gerelli, </b>
    <i>Journal of Applied Crystallography 49 (2016) </i>,
    <a href="https://doi.org/10.1107/S1600576716000108"
       style="color:#0078D7; text-decoration:none;">
       DOI: 10.1107/S1600576716000108
    </a>
  </p>

  <p style="text-align:center; margin:1px 1px;">
    No guarantee is provided that the results produced by the program are correct.
    It is the responsibility of the user to verify all results.
    Downloading and using the program implies acceptance of this condition.
  </p>

  <p style="text-align:center; margin:10px 16px;">
    The program and its documentation are &copy; Copyright of Yuri Gerelli.
  </p>

  <h4 style="text-align:center; margin-top:16px;">Acknowledgement</h4>

  <p style="text-align:center; margin:8px 16px;">
    I am grateful to Dr Giuseppe Allodi
    (Department of Physics, University of Parma)
    for providing the <b>FMINUIT</b> minimization routine.
  </p>

</div>

    )";

    disclaimerBrowser->setHtml(disclaimerHtml);
    disclaimerBrowser->setMinimumHeight(220);
    mainV->addWidget(disclaimerBrowser, /*stretch=*/1);

    // --- Bottom area: contact + buttons ---
    QHBoxLayout *bottomH = new QHBoxLayout;
    bottomH->setSpacing(5);


    // Buttons: Simulate, Fit, Exit, Help
    btnSimulate = new QPushButton("Simulate R(Q)");
    btnFit      = new QPushButton("Fit Data");
    btnExit     = new QPushButton("Exit");
    btnHelp     = new QPushButton("?");

    // Button sizing/layout
   // btnSimulate->setMinimumWidth(180);
    btnFit->setFixedSize(160,44);
    btnExit->setFixedSize(120,44);
    btnHelp->setFixedSize(44,44);
    btnSimulate->setFixedSize(180,44);

    bottomH->addWidget(btnSimulate);
    bottomH->addWidget(btnFit);
    bottomH->addWidget(btnExit);
    bottomH->addWidget(btnHelp);

    mainV->addLayout(bottomH);

    // footer centered

    QLabel *contact = new QLabel("Please address comments and report bugs to <a href='mailto:yuri.gerelli@cnr.it'>yuri.gerelli@cnr.it</a>");
    contact->setTextFormat(Qt::RichText);
    contact->setOpenExternalLinks(true);
    contact->setAlignment(Qt::AlignCenter);
    contact->setStyleSheet("color: #666; font-style: italic; margin-top:6px;");
    mainV->addWidget(contact);

    // --- Connections ---
    connect(btnSimulate, &QPushButton::clicked, this, &MainWindow::onSimulate);
    connect(btnFit, &QPushButton::clicked, this, &MainWindow::onFitData);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::onExit);
    connect(btnHelp, &QPushButton::clicked, this, &MainWindow::onHelp);
}

void MainWindow::applyStyle()
{
    // Modern white theme and polished buttons via stylesheet
    QString style = R"(
        QWidget {
            background-color: #ffffff;
            color: #222222;
            font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
            font-size: 10pt;
        }
        QTextBrowser {
            background-color: #fbfbfb;
            border: 1px solid #e6e6e6;
            padding: 5px;
            border-radius: 6px;
        }
        QPushButton {
            background-color: #0078D7;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 10px;
            font-weight: 600;
            font-size: 10pt;
        }
        QPushButton:hover { background-color: #2894FF; }
        QPushButton:pressed { background-color: #005A9E; }
        QPushButton#Exit {
            background-color: #e04b4b;
        }
        QPushButton#Exit:hover { background-color: #ff6b6b; }
    )";

    qApp->setStyleSheet(style);

    // Give the Exit button a special object name for a red style
    if (btnExit) btnExit->setObjectName("Exit");
}

void MainWindow::onSimulate()
{
    this->hide();
    SimulationWindow *sim = new SimulationWindow();

    connect(sim, &SimulationWindow::backToMain, this, [this, sim]() {
        sim->close();
        this->show();
    });
    sim->show();
}

void MainWindow::onFitData()
{
    simcurves = false;
    QMessageBox::information(this, "Fit Data", "Fit data requested (simcurves = false).\n\nHook Load_params / fitting GUI here.");
    // close(); // uncomment if you want to close the startup window
}

void MainWindow::onExit()
{
    qApp->quit();
}

void MainWindow::onHelp()
{
    // Try to open the manual next to the executable first; fallback to resource not provided
    QString manualPath = QCoreApplication::applicationDirPath() + QDir::separator() + "Aurora_Manual.pdf";
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(manualPath))) {
        QMessageBox::warning(this, "Help", QString("Could not open manual at:\n%1").arg(manualPath));
    }
}

