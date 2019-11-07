#include "widgetopengl.h"

WidgetOpenGL::WidgetOpenGL(const QGLFormat& format, QGLWidget *parent) :
    QGLWidget(format, parent)
{
    m_filename = QFileDialog::getOpenFileName(this, "Open a file containing fibers", "../data", "vtk binary files (*.vtk)").toStdString();
    QWidget::setFocusPolicy(Qt::WheelFocus);
    startTimer(25); //start timer every 25ms
}

WidgetOpenGL::~WidgetOpenGL()
{

}

void WidgetOpenGL::initializeGL()
{
    //Init OpenGL
    setup_opengl();

    //Init Camera
    cam.setupCamera();

    //Init Scene 3D
    m_scene.set_widget(this);
    if (!m_scene.load_scene(m_filename))
    {
        this->window()->close();
        exit(0);
    }
    //Activate depth buffer
    glEnable(GL_DEPTH_TEST); PRINT_OPENGL_ERROR();
    //glEnable(GL_POLYGON_SMOOTH);
}

void WidgetOpenGL::paintGL()
{
    //Compute current cameras
    cam.setupCamera();

    //clear screen
    glViewport (0, 0, cam.getScreenSizeX(), cam.getScreenSizeY()); PRINT_OPENGL_ERROR();
    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);                      PRINT_OPENGL_ERROR();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        PRINT_OPENGL_ERROR();

    m_scene.draw_scene();
}


void WidgetOpenGL::resizeGL(int const width, int const height)
{
    cam.setScreenSize(width, height);
    glViewport(0,0, width, height); PRINT_OPENGL_ERROR();
    //updateGL(); PRINT_OPENGL_ERROR();
}

void WidgetOpenGL::setup_opengl()
{
    print_current_opengl_context();
    setup_glew();
}

void WidgetOpenGL::setup_glew()
{
    //initialize Glew
    glewExperimental=true;
    GLenum GlewInitResult=glewInit();
    PRINT_OPENGL_ERROR();
    //error handling
    if(GLEW_OK != GlewInitResult)
    {
        std::cerr<<"Error: "<<glewGetErrorString(GlewInitResult)<<std::endl;
        exit(EXIT_FAILURE);
    }
    //print debug info
    std::cout<<"Glew initialized ("<<glewGetString(GLEW_VERSION)<<")"<<std::endl;
}

void WidgetOpenGL::print_current_opengl_context() const
{
    std::cout << "OpenGl informations: VENDOR:       " << glGetString(GL_VENDOR)<<std::endl;
    std::cout << "                     RENDERDER:    " << glGetString(GL_RENDERER)<<std::endl;
    std::cout << "                     VERSION:      " << glGetString(GL_VERSION)<<std::endl;
    std::cout << "                     GLSL VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
    std::cout<<"Current OpenGL context: "<< context()->format().majorVersion() << "." << context()->format().minorVersion()<<std::endl;
}

scene& WidgetOpenGL::get_scene()
{
    return m_scene;
}

void WidgetOpenGL::keyPressEvent(QKeyEvent *event)
{
    int current=event->key();
    Qt::KeyboardModifiers mod=event->modifiers();

    // We can quit the scene with 'Q'
    if( (mod&Qt::ShiftModifier)!=0 && (current==Qt::Key_Q) )
    {
        std::cout<<"\n[EXIT OK]\n\n"<<std::endl;
        this->window()->close();
    }
    if (current==Qt::Key_S)
    {
        fstream file;
        file.open("Camera.txt" , ios_base::out);
        Quaternion<float> q = cam.getQuat();
        file << q.x() << " " << q.y() << " " << q.z() << " " << q.w() << endl;
        Vector3f t=cam.getTranslation();
        file << t[0] << " " << t[1] << " " << t[2] << endl;
        file << cam.getDist() << endl;
        file.close();
        cout << "Position de la caméra enregistrée" << endl;
    }
    if (current==Qt::Key_C)
    {
        fstream file;
        file.open("Camera.txt", ios_base::in);
        float x, y, z, w;
        file >> x; file >> y; file >> z; file >> w;
        Quaternion<float> q(w, x, y, z);
        cam.setQuaternion(q);
        file >> x; file >> y; file >> z;
        cam.setTranslation(Vector3f(x, y, z));
        file >> x;
        cam.setDist(x);
        m_scene.draw_scene();
        updateGL(); PRINT_OPENGL_ERROR();
        cout << "Position de la caméra chargée" << endl;
    }
    if (current==Qt::Key_A)
    {
      m_scene.setConcat(false);
      m_scene.valueChanged();
      // Clear la window?
      m_scene.draw_scene();
      updateGL(); PRINT_OPENGL_ERROR();
    }

    if (current==Qt::Key_Z)
    {
      m_scene.setConcat(true);
      m_scene.valueChanged();
      m_scene.draw_scene();
      updateGL(); PRINT_OPENGL_ERROR();
    }

    if (current==Qt::Key_Up)// On incrémente le nombre d'itérations lorsque qu'on appuie sur la flèche bas du clavier
    {
      m_scene.setSceneNbOfIterations(m_scene.getSceneNbOfIterations() + 1);
      m_scene.valueChanged();
      m_scene.draw_scene();
      updateGL(); PRINT_OPENGL_ERROR();
    }

    if (current==Qt::Key_Down && m_scene.getSceneNbOfIterations() > 0)// On décrémente le nombre d'itérations lorsque qu'on appuie sur la flèche bas du clavier (si >0)
    {
      m_scene.setSceneNbOfIterations(m_scene.getSceneNbOfIterations() - 1);
      m_scene.valueChanged();
      m_scene.draw_scene();
      updateGL(); PRINT_OPENGL_ERROR();
    }

    QGLWidget::keyPressEvent(event);
    updateGL();
}

void WidgetOpenGL::timerEvent(QTimerEvent *event)
{
    event->accept();
    updateGL(); PRINT_OPENGL_ERROR();
}


void WidgetOpenGL::mousePressEvent(QMouseEvent *event)
{
    cam.xPrevious()=event->x();
    cam.yPrevious()=event->y();

    updateGL(); PRINT_OPENGL_ERROR();
}

void WidgetOpenGL::mouseMoveEvent(QMouseEvent *event)
{
    int const x=event->x();
    int const y=event->y();

    int const ctrl_pressed  = (event->modifiers() & Qt::ControlModifier);
    int const shift_pressed = (event->modifiers() & Qt::ShiftModifier);

    if(!ctrl_pressed && !shift_pressed && (event->buttons() & Qt::LeftButton))
        cam.rotation(x, y);
    if(!ctrl_pressed && !shift_pressed && (event->buttons() & Qt::RightButton))
        cam.zoom(y);

    // Shift+Left button controls the window translation (left/right, bottom/up)
    float const dL=0.0001f*(1+10*fabs(cam.getDist()));
    if( !ctrl_pressed && shift_pressed && (event->buttons() & Qt::LeftButton) )
    {
        cam.goUp(dL*(y-cam.yPrevious()));
        cam.goRight(-dL*(x-cam.xPrevious()));
    }

    // Shift+Right button enables to translate forward/backward
    if( !ctrl_pressed && shift_pressed && (event->buttons() & Qt::RightButton) )
        cam.goForward(5.0f*dL*(y-cam.yPrevious()));

    cam.xPrevious()=x;
    cam.yPrevious()=y;

    updateGL(); PRINT_OPENGL_ERROR();
}
