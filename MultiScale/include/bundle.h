#ifndef BUNDLE_H
#define BUNDLE_H

#include "limits.h"
#include <forward_list>
#include <fstream>
#include <iostream>
#include "fiber.h"
#include "metrique.h"
#include <QFileDialog>
#include <arpa/inet.h>
#include <iomanip>

#define CHUNK 50
#define TEST_FIBERS_VALIDITY false

class WidgetOpenGL;

using namespace std;

// Definition de la structure
struct vertexPair
{
    unsigned int indexVertex;
    unsigned int indexFibreVertex;
    unsigned int indexNeighbour;
    unsigned int indexFibreNeighbour;
};


class Bundle
{
public:
    Bundle();

    void loadFibers(string filename);
    vector<vector<Vector3f> > getFibers();

    Fiber& operator[](unsigned int element);

    void addFiber(Fiber &new_fiber);//Fonction d'ajout d'une fibre au bundle
    int addFiberSize(Fiber &new_fiber);//Surcharge de la fonction renvoyant la position de la fibre dans le bundle

    void reOrienteFibers(unsigned int reference=0);

    unsigned int size() {return m_fibers.size();}

    // Resampling of the fibers so that the bigger ones and the smaller ones have approximatly the same sample
    void resample(float resampleValue);


    unsigned int getNbOfPoints() {return m_nbOfPoints;}

    void set_widget(WidgetOpenGL* widget_param);

    unsigned int getCurrentNberOfPoints();
    unsigned int getCurrentNberOfFibers();

    void computeTangents();


    // Function that calculates the distance between two neighbours
    float distance(Vector3f sommet1, Vector3f sommet2);

    // Function that tests the second condition, tests if the fibers are parallel enough
    vector<vertexPair> getNeighbours2Fibers(unsigned int nbfibre1, unsigned int nbfibre2,float dmax, float theta);

    //Function that tests the first condition: distance<dmax
    vector<unsigned int> closeFibers(unsigned int nbfibre1, unsigned int nbfibre2,float dmax) ;

    // Testing of the 3rd condition and generating the similarity list
    vector<vertexPair> createSimilarityList(float dmax, float theta);

    //Creates m_newData, new positions of vertexes after contraction
    void newData(vector<vertexPair> list);

    unsigned int getNbOfIterations(){return nbOfIterations;}
    void setNbOfIterations(unsigned int nb){nbOfIterations = nb;}


    // Takes of the outliers vertexes
    void noMoreOutliers(unsigned int minNbNeighbours);

private:
    //Access to the parent object
    WidgetOpenGL* m_pwidget;

    vector<Fiber> m_fibers;
    float m_maxLengthDifBetweenFibers=std::numeric_limits<float>::max();
    float m_maxDistBetweenEndPoints=std::numeric_limits<float>::max();
    unsigned int m_nbOfPoints=0;
    unsigned int nbOfIterations;
};

#endif // BUNDLE_H
