#include "scene.h"
#include "../interface/widgetopengl.h"

scene::scene()
{
  //Initializing the parameters of the contraction
  dmax = 1;
  theta = 0.05;
}

bool scene::load_scene(string filename)
{
    //*****************************************//
    // Preload default structure               //
    //*****************************************//
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "../sources/opengl/shaders/shader.vert", 0 },
        { GL_FRAGMENT_SHADER, "../sources/opengl/shaders/shader.frag", 0 },
        { GL_NONE, NULL, 0 }
    };
    m_basicProgram=loadShaders(shaders);
    if (m_basicProgram==0)
        cerr << "Error due to basic shaders" << endl;

    ShaderInfo singleColorShaders[] =
    {
        { GL_VERTEX_SHADER, "../sources/opengl/shaders/shaderSingleColor.vert", 0 },
        { GL_FRAGMENT_SHADER, "../sources/opengl/shaders/shaderSingleColor.frag", 0 },
        { GL_NONE, NULL, 0 }
    };
    m_singleColorProgram=loadShaders(singleColorShaders);
    if (m_singleColorProgram==0)
        cerr << "Error due to single color shaders" << endl;

    //*****************************************//
    // Load fibers                             //
    //*****************************************//

    auto start = chrono::steady_clock::now();

    // Load fiber data
    cout << "  Load data from file " << filename << " ... " << endl;
    m_brain.loadFibers(filename);
    cout << "   [OK] " << endl;

    cout << "  Resampling ..." << endl;
    m_brain.resample(1);
    cout << "   [OK] " << endl;

    cout << "Calculating neighbours..." << endl;
    nList = m_brain.createSimilarityList(dmax,theta);
    cout << "before new data" << endl;
    m_brain.newData(nList);
    cout << "Neighbours done" << endl;



    //*****************************************//
    // Create OPENGL VBO for the fibers        //
    //*****************************************//
    {
        cout << "  Fill all fibers as VBO ... " << endl;

        // Store all vertices consecutively for OpenGL drawing as GL_LINES
        vector<Vector3f> fibers;
        vector<Vector3f> colors;
        m_numberOfFiberLines=0;
        m_first=new GLint[m_brain.size()];
        m_count=new GLint[m_brain.size()];
        unsigned int counter=0;
        for (unsigned int i=0; i<m_brain.size(); i++)
        {
            if (m_brain[i].is_valid())
            {
                for (unsigned int n=0; n<m_brain[i].size(); n++)
                {
                    fibers.push_back(m_brain[i][n]);
                    colors.push_back(m_brain[i].getColor());
                }
                m_first[m_numberOfFiberLines]=counter;
                m_count[m_numberOfFiberLines]=m_brain[i].size();
                counter+=m_brain[i].size();
                m_numberOfFiberLines++;
            }
        }

        glUseProgram(m_basicProgram);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Create the VBO and fill the data
        glGenBuffers(1, &m_vboFibers); PRINT_OPENGL_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, m_vboFibers);                 PRINT_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*fibers.size()+3*sizeof(float)*colors.size(), NULL, GL_STATIC_DRAW); PRINT_OPENGL_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float)*fibers.size(), fibers.data());PRINT_OPENGL_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*fibers.size(), 3*sizeof(float)*colors.size(), colors.data());PRINT_OPENGL_ERROR();

        GLint loc1=glGetAttribLocation(m_basicProgram, "position");
        glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);PRINT_OPENGL_ERROR();
        GLint loc2=glGetAttribLocation(m_basicProgram, "color");
        glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(3*sizeof(float)*fibers.size()));PRINT_OPENGL_ERROR();

        glEnableVertexAttribArray(loc1);PRINT_OPENGL_ERROR();
        glEnableVertexAttribArray(loc2);PRINT_OPENGL_ERROR();
        m_numberOfPoints = fibers.size();

        cout << "   [OK] " << endl;
        cout << "  Number of valid fibers " << m_numberOfFiberLines << endl;
        m_valueChange = true;
    }

    cout << "Run drawing" << endl;

    auto end = chrono::steady_clock::now();
    auto diff = end-start;
    cout << "Total computational time : " << chrono::duration <double, milli> (diff).count() << " ms" << endl;
    return true;
}

void scene::draw_scene()
{
    glEnable(GL_MULTISAMPLE);
    glUseProgram(m_basicProgram);
    glUniformMatrix4fv(get_uni_loc(m_basicProgram,"camera_modelview"),1,false, m_pwidget->cam.getModelview().data());  PRINT_OPENGL_ERROR();
    glUniformMatrix4fv(get_uni_loc(m_basicProgram,"camera_projection"),1,false,m_pwidget->cam.getProjection().data());   PRINT_OPENGL_ERROR();
    //puts the data of the processor in the graphic card
    glUniform1f(get_uni_loc(m_basicProgram,"slider_position"),m_value_of_slider);   PRINT_OPENGL_ERROR();

    if(m_valueChange)
    {
        // Store all vertices consecutively for OpenGL drawing as GL_LINES
        vector<Vector3f> fibers;
        vector<Vector3f> colors;
        //creates a new list that will store m_new_data
        vector<Vector3f> newfibers;
        m_numberOfFiberLines=0;
        delete(m_first);
        delete(m_count);
        m_first=new GLint[m_brain.size()];
        m_count=new GLint[m_brain.size()];
        unsigned int counter=0;

        //// depending on the key pressed (A or Z), m_data or m_new_data is charged //
        // if(!this->getConcat())
        // {
        //   for (unsigned int i=0; i<m_brain.size(); i++)
        //   {
        //       if (m_brain[i].is_valid())
        //       {
        //           for (unsigned int n=0; n<m_brain[i].size(); n++)
        //           {
        //               fibers.push_back(m_brain[i][n]);
        //               if (m_randomColor)
        //                   colors.push_back(m_brain[i].getColor());
        //               else
        //                   colors.push_back((m_brain[i].getTangent(n)).cwiseAbs());
        //           }
        //           m_first[m_numberOfFiberLines]=counter;
        //           m_count[m_numberOfFiberLines]=m_brain[i].size();
        //           counter+=m_brain[i].size();
        //           m_numberOfFiberLines++;
        //       }
        //   }
        // }
        // else
        // {


          for (unsigned int i=0; i<m_brain.size(); i++)
          {
              if (m_brain[i].is_valid())
              {
                  for (unsigned int n=0; n<m_brain[i].sizeND(); n++)
                  {
                    //loads newfibers with m_new_data
                    newfibers.push_back(m_brain[i].getNewData(n));

                    fibers.push_back(m_brain[i][n]);
                    if (m_randomColor)
                        colors.push_back(m_brain[i].getColor());
                    else
                        colors.push_back((m_brain[i].getTangent(n)).cwiseAbs());
                  }
                  m_first[m_numberOfFiberLines]=counter;
                  m_count[m_numberOfFiberLines]=m_brain[i].sizeND();
                  counter+=m_brain[i].sizeND();
                  m_numberOfFiberLines++;
              }
          }

        // Create the VBO and fill the data
        glBindBuffer(GL_ARRAY_BUFFER, m_vboFibers);                 PRINT_OPENGL_ERROR();

        glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*fibers.size()+3*sizeof(float)*colors.size()+3*sizeof(float)*fibers.size(), NULL, GL_STATIC_DRAW); PRINT_OPENGL_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float)*fibers.size(), fibers.data());PRINT_OPENGL_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*fibers.size(), 3*sizeof(float)*colors.size(), colors.data());PRINT_OPENGL_ERROR();
        //new buffer that allocates space to store newfibers
        glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*newfibers.size()+3*sizeof(float)*colors.size(), 3*sizeof(float)*newfibers.size(), newfibers.data());PRINT_OPENGL_ERROR();

        GLint loc1=glGetAttribLocation(m_basicProgram, "position");
        GLint loc2=glGetAttribLocation(m_basicProgram, "color");
        //gets the attribute newposition
        GLint loc3=glGetAttribLocation(m_basicProgram, "newposition");

        glEnableVertexAttribArray(loc1);PRINT_OPENGL_ERROR();
        glEnableVertexAttribArray(loc2);PRINT_OPENGL_ERROR();
        //enables the generic vertex attribute array specified by loc3
        glEnableVertexAttribArray(loc3);PRINT_OPENGL_ERROR();

        m_numberOfPoints=fibers.size();
        m_valueChange=false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vboFibers);                 PRINT_OPENGL_ERROR();
    GLint loc1=glGetAttribLocation(m_basicProgram, "position");
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);PRINT_OPENGL_ERROR();
    GLint loc2=glGetAttribLocation(m_basicProgram, "color");
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(3*sizeof(float)*m_numberOfPoints));PRINT_OPENGL_ERROR();
    //gets the attribute newposition and gives the memory position to the pointer
    GLint loc3=glGetAttribLocation(m_basicProgram, "newposition");
    glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(2*3*sizeof(float)*m_numberOfPoints));PRINT_OPENGL_ERROR();

    glEnableVertexAttribArray(loc1);PRINT_OPENGL_ERROR();
    glEnableVertexAttribArray(loc2);PRINT_OPENGL_ERROR();
    //enables the generic vertex attribute array specified by loc3
    glEnableVertexAttribArray(loc3);PRINT_OPENGL_ERROR();

    glMultiDrawArrays(GL_LINE_STRIP, m_first, m_count, m_numberOfFiberLines);
}

void scene::set_widget(WidgetOpenGL *widget_param)
{
    m_pwidget=widget_param;
    m_brain.set_widget(widget_param);
}

void scene::start_stop()
{

}

void scene::slider_set_value(float value)
{
    m_value_of_slider = value;
}

/** getter and setter of m_brain.nbOfIterations */
void scene::setSceneNbOfIterations(unsigned int nb)
{
  m_brain.setNbOfIterations(nb);
  m_brain.newData(nList);
  cout<<"Nb d'itérations= "<<getSceneNbOfIterations()<<endl;
}

unsigned int scene::getSceneNbOfIterations()
{
    return m_brain.getNbOfIterations();
}

/** method that calls m_brain.noMoreOutliers(minNbNeighbours) depending on the state of a checkbox */
void scene::getRidOfOutliers(bool state, unsigned int minNbNeighbours)
{
  if (state == true)
  {
    m_brain.noMoreOutliers(minNbNeighbours);
  } else {
    m_brain.newData(nList);
  }
  m_valueChange = true;
}
