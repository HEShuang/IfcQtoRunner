#ifndef QTREGISTRATION_H
#define QTREGISTRATION_H

#include <QMetaType>

#include "SceneData.h"

Q_DECLARE_METATYPE(std::shared_ptr<SceneData::Object>);

void registerQtMetaType()
{
    qRegisterMetaType<std::shared_ptr<SceneData::Object>>("std::shared_ptr<SceneData::Object>");
}

#endif // QTREGISTRATION_H
