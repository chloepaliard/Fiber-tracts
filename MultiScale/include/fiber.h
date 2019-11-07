#ifndef FIBER_H
#define FIBER_H

#include "iostream"
#include <set>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Geometry>
#include <Eigen/Dense>

#define PROFILE_PER_POINT true
#define MIN_RADIUS 0.1

using namespace std;
using namespace Eigen;

class Fiber
{
public:
    Fiber(MatrixXf points, unsigned int size);
    unsigned int size() const {return m_nb_points;}
    unsigned int sizeND() {return m_new_data.rows();}

    Vector3f operator[](unsigned int element) const;

    /** Returns the row i of m_new_data*/
    Vector3f getNewData(unsigned int i);
    /** Sets the row i of m_new_data with newData */
    void setNewData(unsigned int i, VectorXf newData);
    /**Returns the row i of the deplacement vector of the fiber*/
    Vector3f getDVector(unsigned int i);
    /** Sets the row i of the deplacement vector to vector */
    void setDVector(unsigned int i, VectorXf vector);
    /**Resizes m_new data */
    void resizeND(unsigned int i, unsigned int j);
    /**Resizes the deplacement vector*/
    void resizeD(unsigned int i, unsigned int j);

    void setTangents(MatrixXf t);
    float length();
    void PutInSameDirection(const Fiber &fiber);

    void setValue(float value){m_value=value;}
    float getPoint(int i, int j);

    Vector3f getCenter(int i);
    Vector3f getTangent(unsigned int i);
    float getValue(){return m_value;}
    float getMaxDistEndPoints(Fiber &fiber);

    /**Resamples the fibers so that the big and the small fibers have aproximately the same sample */
    void resample(float dsample);

    bool is_flipped(){return m_flipped;}
    void flip(){m_flipped=!m_flipped;}

    void computeCentersAndTangents();

    bool is_valid(){return m_valid;}
    void setValid(bool v){m_valid=v;}

    void addNeighbours(vector<unsigned int> new_neighbours);
    set<unsigned int> m_neighbours;

    void addToHierarchy(unsigned int new_element) {m_hierarchy.insert(new_element);}
    void addToHierarchy(vector<unsigned int> new_elements);
    vector<unsigned int> getHierarchy();

    unsigned int getNbFibHierarchy() {return m_NbFibHierarchy;}
    void addFibHierarchy(unsigned int nbFibHierarchy) {m_NbFibHierarchy+=nbFibHierarchy;}

    float getSelfScalarProduct(float lambdag);
    float getSelfVarifoldScalarProduct(float sigma);

    void setColor(unsigned char r, unsigned char g, unsigned char b){m_red=r; m_green=g; m_blue=b;}
    Vector3f getColor(){return Vector3f((float)m_red/255, (float)m_green/255, (float)m_blue/255);}

    void moveExtremities(Vector3f deviation);

    /** Getter of the variable nb_neighboursNeighbours */
    unsigned int getNbNeighbours(unsigned int i);
    /** Setter of the variable nb_neighbours */
    void setNbNeighbours(unsigned int i, unsigned int j);

private:
    //Data
    float m_length=0;
    float m_value=0;
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;
    unsigned int m_nb_points;
    MatrixXf m_data;
    MatrixXf m_new_data;
    MatrixXf m_centers;
    MatrixXf m_tangents;
    bool m_computedCenterAndTangents=false;

    MatrixXf d_vector;
    vector<unsigned int> nb_neighbours;

    bool m_flipped=false;
    bool m_valid=true;
    set<unsigned int> m_hierarchy;
    unsigned int m_NbFibHierarchy=1;
    float m_selfScalarProduct;
    bool m_selfScalarProductComputed=false;

    //Functions
    vector<float> cumulateLength();
};

#endif // FIBER_H
