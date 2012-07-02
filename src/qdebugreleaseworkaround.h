#ifndef QDEBUGRELEASEWORKAROUND_H
#define QDEBUGRELEASEWORKAROUND_H

#include <QUrl>

inline QDebug operator<<( QDebug d, const QUrl& t )
{
    return d << t.toEncoded();
}

#endif
