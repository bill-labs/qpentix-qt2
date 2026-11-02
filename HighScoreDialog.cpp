#include "HighScoreDialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qheader.h>

HighScoreDialog::HighScoreDialog(HighScoreManager *manager, QWidget *parent, const char *name)
    : QDialog(parent, name, true),  // true = modal
      scoreManager(manager),
      table(0),
      closeButton(0)
{
    setCaption("High Scores");
    setupUI();
    populateTable();
}

HighScoreDialog::~HighScoreDialog()
{
}

void HighScoreDialog::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 10, 5);

    // Title label
    QLabel *titleLabel = new QLabel("Pentix High Scores", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Create table
    table = new QTable(this);
    table->setNumCols(6);
    table->setNumRows(10);  // Top 10 scores
    // Note: Qt2 doesn't have setReadOnly(), cells are read-only by default
    table->setSelectionMode(QTable::NoSelection);

    // Set column headers
    table->horizontalHeader()->setLabel(0, "Rank");
    table->horizontalHeader()->setLabel(1, "Name");
    table->horizontalHeader()->setLabel(2, "Score");
    table->horizontalHeader()->setLabel(3, "Level");
    table->horizontalHeader()->setLabel(4, "Lines");
    table->horizontalHeader()->setLabel(5, "Date");

    // Set column widths
    table->setColumnWidth(0, 50);   // Rank
    table->setColumnWidth(1, 60);   // Name
    table->setColumnWidth(2, 80);   // Score
    table->setColumnWidth(3, 50);   // Level
    table->setColumnWidth(4, 60);   // Lines
    table->setColumnWidth(5, 100);  // Date

    // Hide vertical header (row numbers)
    table->verticalHeader()->hide();

    // Set minimum size
    table->setMinimumWidth(420);
    table->setMinimumHeight(300);

    mainLayout->addWidget(table);

    // Close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    closeButton = new QPushButton("Close", this);
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(closeButton);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Set dialog size
    resize(450, 400);
}

void HighScoreDialog::populateTable()
{
    if (!scoreManager) {
        return;
    }

    const QValueList<HighScoreEntry> &scores = scoreManager->getScores();

    // Clear table first
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 6; col++) {
            table->setText(row, col, "");
        }
    }

    // Populate with scores
    int row = 0;
    for (QValueList<HighScoreEntry>::ConstIterator it = scores.begin();
         it != scores.end() && row < 10; ++it, ++row) {

        // Rank
        table->setText(row, 0, QString::number(row + 1));

        // Name
        table->setText(row, 1, (*it).name);

        // Score
        table->setText(row, 2, QString::number((*it).score));

        // Level
        table->setText(row, 3, QString::number((*it).level));

        // Lines
        table->setText(row, 4, QString::number((*it).lines));

        // Date - format manually for Qt2
        QString dateStr;
        dateStr.sprintf("%04d-%02d-%02d",
                       (*it).date.year(),
                       (*it).date.month(),
                       (*it).date.day());
        table->setText(row, 5, dateStr);

        // Note: Qt2's QTableItem doesn't support setAlignment() directly
        // Text alignment is controlled by the QTable itself
    }

    // Fill empty rows with dashes
    for (int r = row; r < 10; r++) {
        table->setText(r, 0, QString::number(r + 1));
        table->setText(r, 1, "---");
        table->setText(r, 2, "---");
        table->setText(r, 3, "---");
        table->setText(r, 4, "---");
        table->setText(r, 5, "---");
    }

    table->adjustColumn(0);
    table->adjustColumn(1);
    table->adjustColumn(2);
    table->adjustColumn(3);
    table->adjustColumn(4);
    table->adjustColumn(5);
}

void HighScoreDialog::refreshScores()
{
    scoreManager->loadScores();
    populateTable();
}

#include "HighScoreDialog.moc"
