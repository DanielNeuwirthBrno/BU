/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

/* Application:     BankAccountToMoney (bu.exe)
 * Version:         1.1 (build-23)
 *
 * Author:          Daniel Neuwirth
 * E-mail:          d.neuwirth@tiscali.cz
 * Modified:        2018-12-08
 *
 * IDE/framework:   Qt 5.7.0
 * Compiler:        MinGW 5.3.0 32-bit
 * Language:        C++11
 */

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char * argv[])
{
    Q_INIT_RESOURCE(resource);

    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
