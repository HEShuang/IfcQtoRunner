#include "OpenGLWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLContext>
#include <QDebug>

namespace {
    // --- Configurable Speeds ---
    constexpr float m_rotationSpeed = 0.25f;
    constexpr float m_panSpeed = 0.01f;   // This might need significant tuning based on your scene scale
    constexpr float m_zoomSensitivity = 0.1f; // Percentage change per wheel step

    constexpr float m_outlineScaleFactor = 1.03f;
    constexpr QVector3D m_highlightColor = QVector3D(1.0f,1.0f,0.0f);
}

OpenGLWidget::OpenGLWidget(qreal dpiScale, QWidget *parent) :
    QOpenGLWidget(parent),
    m_dpiScale(dpiScale),
    m_program(nullptr)
{
    // Initialize camera parameters (as in your existing code)
    m_cameraPosition = QVector3D(0.0f, -10.0f, 5.0f); // Example: Pull back Y, Z is up
    m_cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    m_cameraOrientation = QQuaternion(); // Initialize to identity
    // Derive distance and initial up vector correctly for Z-up
    // If camera is at (0, -D, H) looking at (0,0,0) with Z world up:
    // Initial view direction is (0, D, -H).normalize()
    // Initial right is cross((0,D,-H), (0,0,1)).normalize()
    // Initial up is cross(right, viewDir).normalize()
    // For simplicity, let's use lookAt to set initial orientation (can be done once)
    QMatrix4x4 initialViewMatrix;
    initialViewMatrix.lookAt(m_cameraPosition, m_cameraTarget, QVector3D(0.0f, 0.0f, 1.0f)); // World Z is UP
    m_cameraOrientation = QQuaternion::fromRotationMatrix(initialViewMatrix.normalMatrix()).conjugated(); // Conjugate if view matrix to world
    m_cameraDistance = (m_cameraPosition - m_cameraTarget).length();
    m_cameraUp = m_cameraOrientation.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f)); // Camera's local Y is its "up"

    // Ensure initial position matches distance and orientation
    QVector3D offsetDirectionWorld = m_cameraOrientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f)); // Camera's local +Z in world
    m_cameraPosition = m_cameraTarget - offsetDirectionWorld * m_cameraDistance; // Look from position towards target


    setMouseTracking(true);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    clearScene(); // Ensure all custom GL resources are destroyed
    if (m_program) delete m_program;
    doneCurrent();
}

//-------------------- public slots -------------------------//

void OpenGLWidget::clearScene() {
    makeCurrent();
    for (RenderableObjectGL& ro : m_renderableObjects) {
        for (auto& mesh : ro.meshes) {
            mesh->destroyGL();
        }
    }
    m_renderableObjects.clear();
    m_hiddenGuids.clear();
    doneCurrent();
    update(); // Request a repaint of the now empty scene
}

void OpenGLWidget::addNewObject(std::shared_ptr<SceneData::Object> pObject) {

    if (!pObject) {
        qWarning() << "OpenGLWidget::addNewObject received a null SceneData::Object pointer! Skipping.";
        return;
    }

    // This slot is called from the GUI thread (due to QueuedConnection or direct call from GUI thread)
    makeCurrent(); // CRITICAL: Need an active OpenGL context to create buffers

    RenderableObjectGL roGL;
    roGL.guid = QString::fromStdString(pObject->guid);
    roGL.type = QString::fromStdString(pObject->type);

    // Convert SceneData::Matrix4x4 to QMatrix4x4
    const float* m = pObject->transform.m;
    roGL.transform = QMatrix4x4(
                         (float)m[0], (float)m[1], (float)m[2], (float)m[3],
                         (float)m[4], (float)m[5], (float)m[6], (float)m[7],
                         (float)m[8], (float)m[9], (float)m[10], (float)m[11],
                         (float)m[12], (float)m[13], (float)m[14], (float)m[15]
        );

    if (pObject->meshes) {
        for (const SceneData::Mesh& meshData : *pObject->meshes) {
            if (meshData.vertices.empty()) {
                qDebug() << "Skipping empty mesh for object GUID:" << QString::fromStdString(pObject->guid);
                continue;
            }

            auto rmGL = std::make_shared<RenderableMeshGL>();
            rmGL->vertexCount = meshData.vertices.size(); // Number of Vec3f elements

            // Create and bind VAO for this mesh
            if (!rmGL->vao.create()) {
                qWarning() << "Failed to create VAO for mesh GUID:" << QString::fromStdString(pObject->guid);
                continue;
            }
            rmGL->vao.bind();

            // VBO for Vertices
            rmGL->vboVertices.create();
            rmGL->vboVertices.bind();
            rmGL->vboVertices.allocate(meshData.vertices.data(), meshData.vertices.size() * sizeof(SceneData::Vec3f));
            m_program->enableAttributeArray(0); // Position
            m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(SceneData::Vec3f));

            // VBO for Normals
            if (!meshData.normals.empty()) {
                rmGL->vboNormals.create();
                rmGL->vboNormals.bind();
                rmGL->vboNormals.allocate(meshData.normals.data(), meshData.normals.size() * sizeof(SceneData::Vec3f));
                m_program->enableAttributeArray(1); // Normal
                m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(SceneData::Vec3f));
            } else {
                // Handle missing normals by disabling attribute or using a default
                m_program->disableAttributeArray(1);
                qDebug() << "Mesh has no normals, GUID:" << QString::fromStdString(pObject->guid);
            }

            rmGL->color = QVector4D(meshData.color.r, meshData.color.g, meshData.color.b, meshData.color.a);
            rmGL->vao.release();
            roGL.meshes.append(std::move(rmGL));
        }
    }

    m_renderableObjects.append(std::move(roGL));

    doneCurrent();
    update(); // Schedule a repaint
    qDebug() << "Added object GUID:" << QString::fromStdString(pObject->guid) << "to render queue. Total objects:" << m_renderableObjects.size();
}

void OpenGLWidget::setVisibility(const QString& guid, bool visible)
{
    bool changed = false;
    if (visible) {
        if (m_hiddenGuids.remove(guid)) {
            changed = true;
        }
    } else { // Hide
        if (!m_hiddenGuids.contains(guid)) {
            m_hiddenGuids.insert(guid);
            changed = true;
        }
    }

    if (changed) update();
}

void OpenGLWidget::selectObjects(const QSet<QString>& guids)
{
    m_selectedGuids = guids;
    update();
}

void OpenGLWidget::deselect()
{
    if(!m_selectedGuids.isEmpty())
    {
        m_selectedGuids.clear();
        update();
    }
}


//-------------------- OpenGL -------------------------//

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 Normal;
        out vec3 FragPos;
        out vec4 VertColor;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            VertColor = vec4(0.8, 0.8, 0.8, 1.0);
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
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qWarning() << "Vertex shader compilation error:" << m_program->log();
    }
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qWarning() << "Fragment shader compilation error:" << m_program->log();
    }
    if (!m_program->link()) {
        qWarning() << "Shader program linking error:" << m_program->log();
    }

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void OpenGLWidget::resizeGL(int w, int h) {
    // Adjust viewport for DPI, ensure h is not zero
    if (h == 0) h = 1;
    glViewport(0, 0, static_cast<GLsizei>(w * m_dpiScale), static_cast<GLsizei>(h * m_dpiScale));
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, (float)w / h, 0.1f, 1000.0f);
}

void OpenGLWidget::paintGL() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_program || !m_program->isLinked()) {
        qWarning("Shader program not linked in paintGL");
        return;
    }
    m_program->bind();

    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);

    m_program->setUniformValue("view", m_viewMatrix);
    m_program->setUniformValue("projection", m_projectionMatrix);
    m_program->setUniformValue("viewPos", m_cameraPosition);
    m_program->setUniformValue("lightPos", QVector3D(m_cameraPosition.x()+10, m_cameraPosition.y()+10, m_cameraPosition.z()+10)); // Light relative to camera
    m_program->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));

    //Pass 1 : Draw normally
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    QList<RenderableObjectGL> selected;

    for (auto& roGL : m_renderableObjects) {

        if(m_hiddenGuids.contains(roGL.guid)) continue;

        bool isSelected = false;

        if(m_selectedGuids.contains(roGL.guid)){
            selected.push_back(roGL);
            isSelected = true;
        }

        m_program->setUniformValue("model", roGL.transform);

        for (auto& meshGL : roGL.meshes) {
            if (meshGL->vertexCount == 0 || !meshGL->vao.isCreated()) continue; // Skip empty or uninitialized meshes

            QOpenGLVertexArrayObject::Binder vaoBinder(&meshGL->vao); // RAII binder for VAO

            // VBOs and attribute pointers are already configured in the VAO.
            // We just need to set uniforms that might change per mesh (like color).
            if(isSelected)
                m_program->setUniformValue("objectColor", m_highlightColor);
            else
                m_program->setUniformValue("objectColor", meshGL->color.toVector3D());

            glDrawArrays(GL_TRIANGLES, 0, meshGL->vertexCount);
        }
    }

    // --- PASS 2: Draw outline for the selected object ---
    if(!selected.isEmpty())
    {
        glCullFace(GL_FRONT);
        glDepthMask(GL_FALSE);
        glPolygonOffset(1.0f, 1.0f);
        glEnable(GL_POLYGON_OFFSET_FILL);


        for (auto& roGL : selected) {

            QMatrix4x4 scaledModelMatrix = roGL.transform;
            // Scaling should be around the object's center, or it will shift.
            // For a simple start, local scale is used.
            // A more robust method is to push vertices along normals in a shader.
            scaledModelMatrix.scale(m_outlineScaleFactor);

            m_program->setUniformValue("model", scaledModelMatrix);
            m_program->setUniformValue("objectColor", m_highlightColor);

            for (auto& meshGL : roGL.meshes) {
                if (meshGL->vertexCount == 0 || !meshGL->vao.isCreated()) continue;
                QOpenGLVertexArrayObject::Binder vaoBinder(&meshGL->vao);
                glDrawArrays(GL_TRIANGLES, 0, meshGL->vertexCount);
            }
        }

        //Reset GL states
        glCullFace(GL_BACK);
        glDepthMask(GL_TRUE);
        glDisable(GL_POLYGON_OFFSET_FILL);

    }

    m_program->release();
}

//-------------------- mouse control -------------------------//

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
    event->accept();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    QPoint diff = event->pos() - m_lastMousePos;

    if (event->buttons() & Qt::LeftButton) { // Orbiting (Pitch and Yaw)
        float pitchAngle = diff.y() * m_rotationSpeed; // Vertical drag for pitch
        float yawAngle = - diff.x() * m_rotationSpeed;   // Horizontal drag for yaw

        // 1. Apply Yaw / Orbit around the UP axis
        QVector3D worldZAxis = QVector3D(0.0f, 0.0f, 1.0f); // World UP is Z
        QQuaternion yawDeltaRotation = QQuaternion::fromAxisAndAngle(worldZAxis, yawAngle);
        m_cameraOrientation = yawDeltaRotation * m_cameraOrientation;

        // 2. Apply Pitch / Orbit around the X axis
        QVector3D newLocalRightAxis = m_cameraOrientation.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
        QQuaternion pitchDeltaRotation = QQuaternion::fromAxisAndAngle(newLocalRightAxis, pitchAngle);
        m_cameraOrientation = pitchDeltaRotation * m_cameraOrientation;

        m_cameraOrientation.normalize();

    } else if (event->buttons() & Qt::MiddleButton) { // Panning (using Middle Mouse/Wheel Button)
        QVector3D worldRightFromCamera = m_cameraOrientation.rotatedVector(QVector3D(1.0f,0.0f,0.0f));
        QVector3D worldUpFromCamera = m_cameraOrientation.rotatedVector(QVector3D(0.0f,1.0f,0.0f)); // Camera's local Y

        float panScale = m_cameraDistance * m_panSpeed * 0.1f;

        QVector3D panOffset = worldRightFromCamera * diff.x() * panScale +
                              worldUpFromCamera * diff.y() * panScale;

        m_cameraPosition += panOffset;
        m_cameraTarget += panOffset;
    }

    // After any transformation, update the derived camera vectors and ensure position honors distance
    // The camera's "up" direction for its view (local Y) transformed to world space
    m_cameraUp = m_cameraOrientation.rotatedVector(QVector3D(0.0f, 1.0f, 0.0f));

    // Position the camera at m_cameraDistance from m_cameraTarget,
    // along the direction defined by m_cameraOrientation's Z-axis.
    QVector3D offsetDirectionWorld = m_cameraOrientation.rotatedVector(QVector3D(0.0f, 0.0f, 1.0f)); // Camera's local +Z in world
    m_cameraPosition = m_cameraTarget - offsetDirectionWorld * m_cameraDistance; // Look from position towards target

    m_lastMousePos = event->pos();
    update(); // Request redraw
    event->accept();
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    // Could add inertia logic here
    event->accept();
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y();
    float zoomFactor = 1.0f;

    if (delta > 0) { // Zoom in
        zoomFactor -= m_zoomSensitivity;
    } else if (delta < 0) { // Zoom out
        zoomFactor += m_zoomSensitivity;
    }

    m_cameraDistance *= zoomFactor;
    m_cameraDistance = qMax(0.1f, m_cameraDistance); // Prevent zooming too close or negative

    // Update camera position based on new distance
    QVector3D localCameraOffset = QVector3D(0.0f, 0.0f, m_cameraDistance);
    m_cameraPosition = m_cameraTarget - m_cameraOrientation.rotatedVector(localCameraOffset);

    update(); // Request redraw
    event->accept();
}
