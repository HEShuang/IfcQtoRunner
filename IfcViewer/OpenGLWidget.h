// OpenGLWidget.h
#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>
#include <QList>
#include <memory>

#include "SceneData.h"

struct RenderableMeshGL {
    QOpenGLVertexArrayObject vao; // VAO to encapsulate VBO bindings and attribute pointers
    QOpenGLBuffer vboVertices;
    QOpenGLBuffer vboNormals;
    int vertexCount = 0;
    QVector4D color;          // Store the actual color for this mesh part

    RenderableMeshGL() : vboVertices(QOpenGLBuffer::VertexBuffer), vboNormals(QOpenGLBuffer::VertexBuffer) {}

    // Call this when clearing geometry, in the OpenGLWidget's context
    void destroyGL() {
        if (vao.isCreated()) vao.destroy();
        if (vboVertices.isCreated()) vboVertices.destroy();
        if (vboNormals.isCreated()) vboNormals.destroy();
    }
};

struct RenderableObjectGL {
    QMatrix4x4 transform; // Model transform for this object
    QList<std::shared_ptr<RenderableMeshGL>> meshes; // Each object can have multiple meshes (e.g., per material)
    QString guid;
    QString type;
};

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    explicit OpenGLWidget(qreal dpiScale, QWidget *parent = nullptr);
    ~OpenGLWidget() override;

public slots:
    void addNewObject(std::shared_ptr<SceneData::Object> pObject); // New slot for progressive loading
    void clearScene();
    void setVisibility(const QString& guid, bool visible);
    void selectObjects(const QSet<QString>& guids);
    void deselect();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    qreal m_dpiScale;
    QOpenGLShaderProgram *m_program;

    QList<RenderableObjectGL> m_renderableObjects; // Stores all displayable objects
    QSet<QString> m_hiddenGuids;
    QSet<QString> m_selectedGuids;

    // Camera parameters
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;   // The point the camera orbits/looks at
    QVector3D m_cameraUp;       // Will be derived from orientation
    QQuaternion m_cameraOrientation; // Represents the camera's orientation
    float m_cameraDistance;

    QPoint m_lastMousePos;



};

#endif // OPENGLWIDGET_H
