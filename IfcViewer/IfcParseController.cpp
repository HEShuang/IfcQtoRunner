#include "IfcParseController.h"
#include "IfcPreview.h"
#include <QMetaObject>

IfcParseController::IfcParseController(QObject *parent) : QObject(parent) {}

IfcParseController::~IfcParseController() {
    if (m_workerThread.joinable()) {
        m_workerThread.join(); // Ensure thread is joined on destruction
    }
}

void IfcParseController::startParsing(const QString& filePath) {
    if (m_workerThread.joinable()) {
        m_workerThread.join(); // Wait for any previous parsing to finish
    }

    m_parserInstance = std::make_unique<IfcPreview>(filePath.toStdString());


    auto callback_objectReady = [this](std::shared_ptr<SceneData::Object> objData){
        // This lambda is executed in m_workerThread.
        // Use QMetaObject::invokeMethod to call a slot in this controller's thread (GUI thread).
        QMetaObject::invokeMethod(this, "handleObjectReady", Qt::QueuedConnection,
                                  Q_ARG(std::shared_ptr<SceneData::Object>, objData));
    };

    auto callback_finished = [this](bool success, const std::string& msg) {
        // This lambda is executed in m_workerThread.
        QString qMsg = QString::fromStdString(msg);
        QMetaObject::invokeMethod(this, "handleParsingFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, success), Q_ARG(QString, qMsg));
    };

    // Start the parsing in a new std::thread
    m_workerThread = std::thread(&IfcPreview::parseGeometryStream,
                      m_parserInstance.get(),
                      callback_objectReady,
                      callback_finished
                      );
}

// These slots are guaranteed to be called in the thread of IfcParserController (GUI thread)
void IfcParseController::handleObjectReady(std::shared_ptr<SceneData::Object> objectData) {
    emit objectReadyForOpenGL(objectData); // Forward to OpenGLWidget
}

void IfcParseController::handleParsingFinished(bool success, const QString& message) {
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    emit parsingComplete(success, message);
}
