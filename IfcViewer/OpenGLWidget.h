#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <vector>

#include "SceneData.h"


class OpenGLWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    OpenGLWidget(qreal dpiScale, QWidget *parent = nullptr);
    ~OpenGLWidget();

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

#endif // OPENGLWIDGET_H
