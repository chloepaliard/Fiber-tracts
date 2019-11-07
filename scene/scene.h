#pragma once

#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <iostream>
#include <chrono>
#include "../opengl/openglutils.h"
#include "../MultiScale/include/bundle.h"
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

class WidgetOpenGL;

class scene
{
public:
    scene();

    /**  Method called only once at the beginning (load off files ...) */
    bool load_scene(string filename);

    /**  Method called at every frame */
    void draw_scene();

    /** Set the pointer to the parent Widget */
    void set_widget(WidgetOpenGL* widget_param);

    /** Start/stop the animation */
    void start_stop();

    void slider_set_value(float value);

    void valueChanged(){m_valueChange=true;}

    void changeColorState(bool random) {m_randomColor=random; m_valueChange=true;}

    //fonctions utiles avant le slider
    /** getter and setter of concat */
    bool getConcat() {return concat;}
    void setConcat(bool nouvconcat) {concat=nouvconcat;}

    /** getter and setter of m_brain.nbOfIterations */
    void setSceneNbOfIterations(unsigned int nb);
    unsigned int getSceneNbOfIterations();

    /** method that calls m_brain.noMoreOutliers(minNbNeighbours) depending on the state of a checkbox */
    void getRidOfOutliers(bool state, unsigned int minNbNeighbours);

private:
    //Access to the parent object
    WidgetOpenGL* m_pwidget;

    //GL programs
    GLuint m_basicProgram;
    GLuint m_singleColorProgram;

    //Fibers
    Bundle m_brain;

    //True or false wether we want to contract the fibers or not
    bool concat;

    //Storage for the VBO associated to the fibers seen as lines
    GLuint m_vboFibers;
    GLuint m_vao;
    int m_numberOfFiberLines;
    int m_numberOfPoints;
    GLuint* m_vertexIndices;
    unsigned int m_nbOfIndices=0;
    unsigned int m_currentNbOfPoints=0;
    GLint* m_count;
    GLint* m_first;

    //Color Mode
    bool m_randomColor=true;

    //Slider
    float m_value_of_slider=1;
    bool m_valueChange=false;

    //New parameters linked to the contraction
    float dmax;
    float theta;
    vector<vertexPair> nList;
};

#endif // SCENE_H
