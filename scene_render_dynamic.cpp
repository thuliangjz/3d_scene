#include "scene_render_dynamic.h"
#include <glm/vec3.hpp>
#include <QDebug>

SceneRenderDynamic::SceneRenderDynamic():
    m_window(nullptr), m_program(nullptr),
    m_vbo(nullptr), m_ebo(nullptr), m_vao(nullptr),
    m_gl_context_inited(false){
}

void SceneRenderDynamic::glInit(){
    m_gl_context_inited = true;
    initializeOpenGLFunctions();
    m_vao = new QOpenGLVertexArrayObject();
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_ebo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader_triangle.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader_triangle.frag");
    m_program->bindAttributeLocation("pos", 0);
    m_program->bindAttributeLocation("color_in", 1);
    m_program->link();
    m_program->bind();

    m_vao->create();    //glGenVertexArrays called
    m_vao->bind();      //glBindVertexArray called


    float b[] = {
        -0.5f, 0.5f, 0.f, 1.0f, 0.f, 0.f,
        0.5, 0.5f, 0.f, 0.f, 1.f, 0.f,
        .5f, -.5f, 0.f, 0.f, 0.f, 1.f,
        -.5f, -.5f, 0.f, 1.f, 0.f, 1.f
    };

    m_vbo->create();    //glGenbuffer called
    m_vbo->bind();      //glBindBuffer called
    //default usage pattern: STATIC_DRAW
    m_vbo->allocate(b, sizeof(b));   //glBufferData called

    unsigned int idx[] = {
        0, 1, 2,
        0, 2, 3,
    };

    m_ebo->create();
    m_ebo->bind();  //vao内部的状态被修改
    m_ebo->allocate(idx, sizeof(idx));

    m_program->enableAttributeArray(0);     //glEnableVertexAttributeArray called
    m_program->enableAttributeArray(1);     //同上
    //glVertexAttribPointer called, 注意到此时m_vao指向的vao的状态被修改
    int indent = 3 * sizeof(float);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 2 * indent);
    m_program->setAttributeBuffer(1, GL_FLOAT, indent, 3, 2 * indent);

    m_vao->release();
    m_vbo->release();
    m_ebo->release();
    m_program->release();
}

void SceneRenderDynamic::paint(){
    if(!m_gl_context_inited){
        glInit();
    }
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glDisable(GL_DEPTH_TEST);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    m_program->bind();
    m_vao->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    //glDrawArrays(GL_TRIANGLES, 0, 3);   //first为起始点索引，count是绘制顶点的个数
    m_vao->release();
    m_program->release();
    m_window->resetOpenGLState();
}
/*
    if(!m_gl_context_inited){
        //对Render进行必要的初始化，可以放在init函数中
        m_gl_context_inited = true;
        initializeOpenGLFunctions();
        //着色器代码编译与链接
        int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &shader_triangle_vertex, nullptr);
        glCompileShader(vertex_shader);
        int success;
        const int size_log_block = 512;
        char info_log[size_log_block];
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(vertex_shader, size_log_block, nullptr, info_log);
            qDebug() << "Error compiling vertex shader: " << info_log;
            return;
        }

        int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &shader_triangle_fragment, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(fragment_shader, size_log_block, nullptr, info_log);
            qDebug() << "Error compiling fragment shader: " << info_log;
            return;
        }
        m_shader_program_id = glCreateProgram();
        glAttachShader(m_shader_program_id, vertex_shader);
        glAttachShader(m_shader_program_id, fragment_shader);
        glLinkProgram(m_shader_program_id);
        glGetProgramiv(m_shader_program_id, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(m_shader_program_id, size_log_block, nullptr, info_log);
            qDebug() << "error while linking: " << info_log;
            return;
        }
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        //数据准备(特别是对于不变的数据，应该只准备一次)
        unsigned int id_vbo, id_vao;
        glGenBuffers(1, &id_vbo);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 0);
    }
    */

/*
    if(!m_program){
        initializeOpenGLFunctions();
        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, shader_triangle_vertex);
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, shader_triangle_fragment);
        m_program->bindAttributeLocation("pos", 0);
        m_program->bindAttributeLocation("color_in", 1);
        m_program->link();
        m_program->enableAttributeArray(0);
        m_program->enableAttributeArray(1);
        m_program->setAttributeArray(0, GL_FLOAT, m_vertices, 3);
        m_program->setAttributeArray(1, GL_FLOAT, m_colors, 3);
        m_program->setAttributeBuffer(1, GL_FLOAT, 0, 0);
    }
    m_program->bind();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glDisable(GL_DEPTH_TEST);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    m_program->release();
    m_window->resetOpenGLState();

*/
