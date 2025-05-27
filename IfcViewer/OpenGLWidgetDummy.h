#ifndef OPENGLWIDGETDUMMY_H
#define OPENGLWIDGETDUMMY_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <vector>

#include "SceneData.h"

/*
 * Dummy opengl widget:
 * load and render all geometries once
 * with no support for adding/removing objects
 * */
class OpenGLWidgetDummy: public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLWidgetDummy(qreal dpiScale, QWidget *parent = nullptr);
    ~OpenGLWidgetDummy();

public slots:
    void setSceneObjects(std::unique_ptr<std::vector<SceneData::Object>>&& upObjects);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    qreal m_dpiScale;

    //OpenGL rendering resources
    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer m_vbo;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_ibo; //index buffer

    //Camera control
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    QPoint m_lastMousePos;

    std::unique_ptr<std::vector<SceneData::Object>> m_upObjects = nullptr;
};

#endif // OPENGLWIDGETDUMMY_H
