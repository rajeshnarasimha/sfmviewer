/*
 * GLCanvas.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: an OpenGL canvas
 */

#pragma once

#include <QGLWidget>

class QtLogo;

class GLCanvas : public QGLWidget
{
    Q_OBJECT

public:
    GLCanvas(QWidget *parent = 0);
    ~GLCanvas();

    // return the minimal size of the window
    QSize minimumSizeHint() const;

    // Used by windowing system, uses globals set by size
    QSize sizeHint() const;

    // Called from window.cpp to set default size
    void setSizeHint(int w, int h);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

     // record the last mouse position
    void mousePressEvent(QMouseEvent *event);

    // mouse click and drag
    void mouseMoveEvent(QMouseEvent *event);

    void DrawStructure();



private:
    QPoint lastPos_;
    QColor qtGreen;
    QColor qtPurple;

    int hintWidth_;
    int hintHeight_;

    // the data structure for the view port
    struct ViewPort {
        float m_shift[3];  // translation
        float m_quat[4];   // orientation in quaternion format
    };

    // the current opengl viewport
    ViewPort viewPort_;

    struct Vertex{
        GLfloat X,Y,Z;
    };
    std::vector<Vertex> structure_;
};
