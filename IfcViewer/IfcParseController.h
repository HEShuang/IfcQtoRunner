#ifndef IFCPARSECONTROLLER_H
#define IFCPARSECONTROLLER_H

#include <QObject>
#include <QString>
#include <thread>
#include <memory>

#include "SceneData.h"

class IfcPreview;

class IfcParseController : public QObject {
    Q_OBJECT
public:
    explicit IfcParseController(QObject *parent = nullptr);
    ~IfcParseController();

    void startParsing(const QString& filePath);

signals:
    void objectReadyForOpenGL(std::shared_ptr<SceneData::Object> objectData); // To send to OpenGLWidget
    void parsingComplete(bool success, const QString& message);

private slots:
    // These slots will be invoked in the IfcParseController's thread (GUI thread)
    // via QMetaObject::invokeMethod
    void handleObjectReady(std::shared_ptr<SceneData::Object> objectData);
    void handleParsingFinished(bool success, const QString& message);

private:
    std::unique_ptr<IfcPreview> m_parserInstance;
    std::thread m_workerThread;
};

#endif // IFCPARSECONTROLLER_H
