/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#include <QIcon>
// #include <QPair>
// #include <QString>
// #include <QVector>
#include "bank.h"
#include "bankwindow.h"

BankWindow::BankWindow(const QVector<QPair<QString, QString> > & inListOfBanks,
                       QPair<bool, bool> formatButton, const QString & bankCodeFromConfig, QWidget * parent):
    QDialog(parent), accountFormatSwitch(formatButton.first), ui(new Ui_BankWindow) {

    ui->setupUi(inListOfBanks, formatButton, bankCodeFromConfig, this);

    connect(ui->selectButton, &QPushButton::clicked, this, &BankWindow::close);
    connect(ui->selectButton, &QPushButton::clicked, this, &BankWindow::returnValue);
    connect(ui->cancelButton, &QPushButton::clicked, this, &BankWindow::close);
    connect(ui->cancelButton, &QPushButton::clicked, this,
            [this]() -> void { this->setResult(-1); });

    connect(ui->headerH, &QHeaderView::sectionClicked, this, &BankWindow::sortSelectedColumn);

    if (accountFormatSwitch)
        connect(ui->selectAccountFormatButton, &QPushButton::clicked,
                this, &BankWindow::changeAccountFormat);
}

// [slot]
void BankWindow::returnValue() {

    const int result =
        (accountFormatSwitch && ui->selectAccountFormatButton->text() == "vnitřní formát") ?
            ui->bankListTable->currentRow()+1000 :  ui->bankListTable->currentRow();
    this->setResult(result);
    return;
}

// [slot]
void BankWindow::changeAccountFormat() const {

    if (ui->selectAccountFormatButton->text() == "vnitřní formát")
        ui->selectAccountFormatButton->setText(QStringLiteral("ediční formát"));
    else
        ui->selectAccountFormatButton->setText(QStringLiteral("vnitřní formát"));

    return;
}

// [slot]
void BankWindow::sortSelectedColumn(const int column) const {

    ui->bankListTable->sortItems(column);
    return;
}

AccountFormatDialog::AccountFormatDialog(bool & format) {

    QIcon * formatIcon = new QIcon(QStringLiteral(":/icons/icons/wallet-open.png"));
    this->setWindowIcon(*formatIcon);
    this->setWindowTitle(QStringLiteral("Formát čísla účtu"));

    QPushButton * internalButton = new QPushButton(QStringLiteral(" vnitřní formát "));
    QPushButton * staticButton = new QPushButton(QStringLiteral(" ediční formát "));
    QPushButton * const activeButton = (format) ? internalButton : staticButton;

    activeButton->setStyleSheet("font:bold");
    activeButton->setAutoDefault(true);
    activeButton->setDefault(true);

    this->addButton(internalButton, QMessageBox::AcceptRole);
    this->addButton(staticButton, QMessageBox::RejectRole);

    this->setTextFormat(Qt::RichText);
    this->setText("<span style=\"font-weight:bold\">" +
                  QStringLiteral("Výběr formátu čísla účtu") + "</span>");
    this->setInformativeText(QStringLiteral("V jakém formátu je číslo bankovního účtu v") +
                             QString(QChar::Nbsp) + QStringLiteral("načítaném výpisu uloženo?"));
    this->setIcon(QMessageBox::Question);
}
