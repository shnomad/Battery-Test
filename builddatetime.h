#ifndef BUILDDATETIME_H
#define BUILDDATETIME_H

#include <QString>
#include <QLocale>

QString build_date = QString("%1T%2").arg(__DATE__).arg(__TIME__);

#endif // BUILDDATETIME_H
