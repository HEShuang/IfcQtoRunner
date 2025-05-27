#include "OpenGLWidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLContext>

OpenGLWidget::OpenGLWidget(qreal dpiScale, QWidget *parent) :
    QOpenGLWidget(parent),
    m_dpiScale(dpiScale),
    m_program(nullptr),
    m_vbo(QOpenGLBuffer::VertexBuffer),
    m_ibo(QOpenGLBuffer::IndexBuffer)
{
    // Initial camera settings
    m_cameraPosition = QVector3D(5.0f, 5.0f, 5.0f);
    m_cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    m_cameraUp = QVector3D(0.0f, 0.0f, 1.0f); // Z-up for IFC
    m_cameraDistance = m_cameraPosition.length(); // Initial distance

    // Enable mouse tracking for continuous movement
    setMouseTracking(true);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent(); // Essential to delete GL resources in the correct context
    if (m_program) delete m_program;
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_ibo.isCreated()) m_ibo.destroy();
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions(); // Initialize QOpenGLFunctions_3_2_Core

    // Basic Phong shading or simpler pass-through
    const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal; // Normal for lighting
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 lightPos; // Simple point light

        out vec3 Normal;
        out vec3 FragPos;
        out vec4 VertColor; // To pass material color

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            VertColor = vec4(0.8, 0.8, 0.8, 1.0); // Default color, replace with material
        }
    )";

    const char *fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec4 VertColor; // Passed material color

        uniform vec3 objectColor; // Material color passed from C++
        // uniform float alpha_from_cpp; // If you decide to pass sanitized alpha separately

        void main() {
            FragColor = vec4(objectColor, VertColor.a);
        }
    )";


    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_program->bind();

    // Setup global OpenGL state
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray background
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // For performance, cull back faces
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w * m_dpiScale, h * m_dpiScale); // Adjust viewport for DPI
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, (float)w / h, 0.1f, 1000.0f); // FOV, Aspect, Near, Far
}

void OpenGLWidget::paintGL()
{
    if(!m_upObjects)
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    // Update camera (view matrix)
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);


    m_program->setUniformValue("view", m_viewMatrix);
    m_program->setUniformValue("projection", m_projectionMatrix);
    m_program->setUniformValue("viewPos", m_cameraPosition);

    // Simple light setup
    m_program->setUniformValue("lightPos", QVector3D(10.0f, 10.0f, 10.0f)); // Example light position
    m_program->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));  // White light

    // Loop through your parsed IFC geometry and draw each group
    for (const auto& object : *m_upObjects.get())
    {
        QMatrix4x4 modelMatrix(object.transform.m);

        for(const auto& mesh : *object.meshes.get())
        {
            // Bind VAO specific to this geometry group
            // For now, we'll recreate a simple VAO/VBO for demonstration
            // In a real app, you'd manage these in a more persistent way
            // and maybe store them in the IfcGeometryGroup struct.

            QOpenGLVertexArrayObject vao;
            vao.create();
            vao.bind();

            QOpenGLBuffer vboPos(QOpenGLBuffer::VertexBuffer);
            vboPos.create();
            vboPos.bind();
            vboPos.allocate(mesh.vertices.data(), mesh.vertices.size() * sizeof(SceneData::Vec3f));
            m_program->enableAttributeArray(0);
            m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(SceneData::Vec3f));

            // VBO for Normals
            QOpenGLBuffer vboNorm(QOpenGLBuffer::VertexBuffer); // Inefficient.
            vboNorm.create();
            vboNorm.bind(); // Bind the new VBO for normals
            vboNorm.allocate(mesh.normals.data(), mesh.normals.size() * sizeof(SceneData::Vec3f));
            m_program->enableAttributeArray(1); // Enable normal attribute (aNormal)
            m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(SceneData::Vec3f)); // Location 1

            m_program->setUniformValue("model", modelMatrix); // Apply individual model transforms from IFC if available
            m_program->setUniformValue("objectColor", QVector3D(mesh.color.r, mesh.color.g, mesh.color.b)); // Pass material color

            glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.size()); // Assuming non-indexed drawing for simplicity

            vao.release();
            vboPos.release();
            vboNorm.release();
        }
    }

    m_program->release();
}

// --- Camera Control (Simplified Trackball-like) ---
void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
    event->accept();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - m_lastMousePos;
        float dx = diff.x() * 0.2f; // Sensitivity
        float dy = diff.y() * 0.2f;

        // Rotate camera around the target
        QMatrix4x4 rotation;
        rotation.rotate(-dx, m_cameraUp); // Yaw
        QVector3D right = QVector3D::crossProduct(m_cameraPosition - m_cameraTarget, m_cameraUp).normalized();
        rotation.rotate(-dy, right); // Pitch

        m_cameraPosition = rotation.map(m_cameraPosition - m_cameraTarget) + m_cameraTarget;
        m_cameraUp = rotation.map(m_cameraUp).normalized(); // Keep up vector consistent

        update(); // Request redraw
    }
    m_lastMousePos = event->pos();
    event->accept();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    event->accept();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    float delta = event->angleDelta().y();
    if (delta > 0) {
        m_cameraDistance *= 0.9f; // Zoom in
    } else {
        m_cameraDistance *= 1.1f; // Zoom out
    }
    m_cameraPosition = m_cameraTarget + (m_cameraPosition - m_cameraTarget).normalized() * m_cameraDistance;
    update(); // Request redraw
    event->accept();
}


void OpenGLWidget::setSceneObjects(std::unique_ptr<std::vector<SceneData::Object>>&& upObjects)
{
    m_upObjects = nullptr;
    if(!upObjects)
        return;

    m_upObjects = std::move(upObjects);

    // After parsing, set the initial camera view based on the loaded geometry
    // (You'll need bounding box calculation for your custom geometry)
    // For now, let's just use the initial camera settings
    m_cameraPosition = QVector3D(10.0f, 10.0f, 10.0f);
    m_cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    m_cameraUp = QVector3D(0.0f, 0.0f, 1.0f); // Z-up for IFC
    m_cameraDistance = m_cameraPosition.length();

    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);

    update(); // Request redraw
}
