/*******************************************************************************
 Copyright 2018 Daniel Neuwirth
 This program is distributed under the terms of the GNU General Public License.
*******************************************************************************/

#ifndef ICONLABEL_H
#define ICONLABEL_H

// IconLabelWidget class, IconAndMouseLabelWidget class (custom QLabel widgets)

#include <QEvent>
#include <QLabel>
#include <QMap>
#include <QMouseEvent>
#include <QPixmap>
#include <QString>
#include <QWidget>

class IconLabelWidget: public QLabel {

    Q_OBJECT

    public:
        enum ResultType: int8_t { UNKNOWN = -1, ERROR = 0, OK = 1, BUSY = 2, READ = 3, WRITE = 4 };

        const QMap<ResultType, QString> Result {
            { UNKNOWN, QStringLiteral(":/icons/icons/list-remove-blue.png") },
            { ERROR, QStringLiteral(":/icons/icons/edit-delete.png") },
            { OK, QStringLiteral(":/icons/icons/dialog-ok-apply.png") },
            { BUSY, QStringLiteral(":/icons/icons/download-later.png") },
            { READ, QStringLiteral(":/icons/icons/svn-commit.png") },
            { WRITE, QStringLiteral(":/icons/icons/svn-update.png") }
        };

        explicit IconLabelWidget(QWidget * parent = nullptr):
            QLabel(parent) { setIcon(UNKNOWN); setToolTip(QStringLiteral("N/A")); }
        /* explicit IconLabelWidget(const QString & string, QWidget * parent = nullptr):
            QLabel(string, parent) {} */
        explicit IconLabelWidget(const ResultType state, QWidget * parent = nullptr):
            QLabel(parent) { setIcon(state); }
        ~IconLabelWidget() {}

        static inline ResultType convertToResultType(const bool state)
            { return static_cast<ResultType>(state); }

        inline void setIcon(const ResultType state) {

            const QPixmap * const icon = new QPixmap(Result[state]);
            this->setPixmap(icon->scaled(16,16));
            delete icon;
        }
};

class IconAndMouseLabelWidget: public IconLabelWidget {

    Q_OBJECT

    public:
        explicit IconAndMouseLabelWidget(QWidget * parent = nullptr):
            IconLabelWidget(parent) {}
        /* explicit IconAndMouseLabelWidget(const QString & string, QWidget * parent = nullptr):
            IconLabelWidget(string, parent) {} */
        ~IconAndMouseLabelWidget() {}

        void mousePressEvent(QMouseEvent * event) {
            if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick)
                emit iconClicked();
            else
                QLabel::mousePressEvent(event);
        }

     signals:
        void iconClicked();
};

#endif // ICONLABEL_H
