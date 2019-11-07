#include "../include/fiber.h"

Fiber::Fiber(MatrixXf points, unsigned int size)
{
    m_nb_points=size;
    m_data.resizeLike(points);
    for (unsigned int i=0; i<size; i++)
        m_data.row(i)=points.row(i);
    //Compute a random color for the fiber
    m_red=(float)rand()/RAND_MAX*255;
    m_green=(float)rand()/RAND_MAX*255;
    m_blue=(float)rand()/RAND_MAX*255;
}

Vector3f Fiber::operator[](unsigned int element) const
{
    if (element<m_nb_points)
        if (m_flipped)//Si la fibre est retournée, on la parcourt dans l'autre sens
            return (Vector3f)m_data.row(m_nb_points-element-1);
        else
            return (Vector3f)m_data.row(element);
    else
        return (Vector3f)(0);
}

//Returns the row i of m_new_data
Vector3f Fiber::getNewData(unsigned int i)
{
  if (i<m_nb_points)
      if (m_flipped)//If the fibber is flipped we go over the fiber from the other side
      {
        return (Vector3f)m_new_data.row(sizeND()-i-1);

      }
      else
      {
        return (Vector3f)m_new_data.row(i);
      }

  else
      return (Vector3f)(0);
}


// Sets the row i of m_new_data with newData
void Fiber::setNewData(unsigned int i, VectorXf newData)
{
  m_new_data.row(i) = (Vector3f)newData;
}

//Returns the row i of the deplacement vector of the fiber
Vector3f Fiber::getDVector(unsigned int i)
{
  Vector3f vector = (Vector3f)d_vector.row(i);
  return vector;
}

// Sets the row i of the deplacement vector to vector
void Fiber::setDVector(unsigned int i, VectorXf vector)
{
  d_vector.row(i) = (Vector3f)vector;
}

/**Resizes m_new data */
void Fiber::resizeND(unsigned int i, unsigned int j)
{
  m_new_data.resize(i,j);
}

/**Resizes the deplacement vector */
void Fiber::resizeD(unsigned int i, unsigned int j)
{
  d_vector.resize(i,j);
}

float Fiber::getPoint(int i, int j)
{
    if (m_flipped)
        return m_data(m_nb_points-1-i, j);
    else
        return m_data(i,j);
}

Vector3f Fiber::getCenter(int i)
{
    if (m_flipped)
        return m_centers.row(m_centers.rows()-1-i);
    else
        return m_centers.row(i);
}

Vector3f Fiber::getTangent(unsigned int i)
{
    if (m_flipped)
        i=size()-i-1;
    if (i<size()-1)
        return (m_data.row(i+1)-m_data.row(i)).normalized();
    else
        return (m_data.row(i)-m_data.row(i-1)).normalized();
}

void Fiber::setTangents(MatrixXf t)
{
    m_tangents.resizeLike(t);
    for (int i=0; i<t.rows(); i++)
        m_tangents.row(i)=t.row(i);
}

float Fiber::length()
{
    //Pas de recalcul s'il a déjà été fait
    if (m_length!=0) return m_length;
    m_length=0;
    for (unsigned int i=0; i<m_nb_points-1; i++)
        m_length+=(m_data.row(i+1)-m_data.row(i)).squaredNorm();
    return m_length;
}

vector<float> Fiber::cumulateLength()
{
    vector<float> cumulatedLength(m_nb_points);
    cumulatedLength[0]=0;
    for (unsigned int i=1; i<m_nb_points; i++)
        cumulatedLength[i]=(m_data.row(i)-m_data.row(i-1)).norm()+cumulatedLength[i-1];
    m_length=cumulatedLength[m_nb_points-1];
    return cumulatedLength;
}


/**Resamples the fibers so that the big and the small fibers have aproximately the same sample */
void Fiber::resample(float dsample)
{
    vector<float> cumulatedLength = this->cumulateLength();
    if (cumulatedLength[m_nb_points-1]<dsample) // If the fiber is to small to have at least one sample
    {
        cerr << "Error, impossible to resample with a length < dsample" << endl;
        setValid(false);
        return;
    }
    //No resample if fiber already has the number of points asked to prevent computational approximations leading to errors
    if (cumulatedLength[m_nb_points-1]==dsample)
        return;

    MatrixXf new_points;
    float next_point=0;
    float ratio;
    float step;
    unsigned int longueur;
    Vector3f delta;
    unsigned int i=0, j=0, k=0;

    //calculation of the step
    //rq = dinf ans dsup is a nb of points
    float dinf = cumulatedLength[m_nb_points-1]/(floor(cumulatedLength[m_nb_points-1]/dsample)+1);
    float dsup = cumulatedLength[m_nb_points-1]/(floor(cumulatedLength[m_nb_points-1]/dsample));
    if (min(dsample-dinf, dsup-dsample)==dsample-dinf)
    {
      step=dinf;
      longueur = floor(cumulatedLength[m_nb_points-1]/dsample)+2;
    }
    else
    {
      step=dsup;
      longueur = floor(cumulatedLength[m_nb_points-1]/dsample)+1;
    }
    // If dsample if too far from step we don't tale the fiber into account
    if (fabs(step-dsample)/dsample>0.2){
      setValid(false); //Fiber non valid
    }

    if(m_valid){
      new_points.resize(longueur,3);

      while (next_point<m_length) // We go through the whole fiber
      {
          if (next_point==cumulatedLength[k]) // The next point is on an existing point
          {
              new_points.row(i)=m_data.row(j);
              next_point+=step;
              i++; j++; k++;
          }
          else if (next_point<cumulatedLength[k])
          {
              ratio=1-(cumulatedLength[k]-next_point)/(cumulatedLength[k]-cumulatedLength[k-1]);
              delta=(m_data.row(j)-m_data.row(j-1));
              new_points.row(i)=(Vector3f)m_data.row(j)+(Vector3f)(ratio*delta);
              next_point+=step;
              i++;
          }
          else
          {
              j++; k++;
        }
      }
      //Adding of the last point
      new_points.row(longueur-1)=m_data.row(m_nb_points-1);

      //Current fibers gets the data
      m_nb_points=longueur;

      m_data.resizeLike(new_points);
      nb_neighbours.resize(m_data.size());
      for (unsigned int i=0; i<m_nb_points; i++)
      {
          m_data.row(i)=new_points.row(i);
          nb_neighbours[i] = 0;
      }
      m_length=0;
    }

}

void Fiber::PutInSameDirection(Fiber const &fiber)
{
    float distanceEndpoints1=0;
    float distanceEndpoints2=0;
    distanceEndpoints1=min(((*this)[0]-fiber[0]).squaredNorm(),((*this)[m_nb_points-1]-fiber[fiber.size()-1]).squaredNorm());
    distanceEndpoints2=min(((*this)[0]-fiber[fiber.size()-1]).squaredNorm(),((*this)[m_nb_points-1]-fiber[0]).squaredNorm());
    //The shortest distance determines if the fibers are oriented the same way or not
    if (distanceEndpoints2<distanceEndpoints1)
        this->flip();
}

void Fiber::computeCentersAndTangents()
{
    if (m_computedCenterAndTangents)
        return;
    m_centers.resize(m_nb_points-1, 3);
    m_tangents.resize(m_nb_points-1, 3);
    for (unsigned int i=0; i<m_nb_points-1; i++)
    {
        m_centers.row(i)=(m_data.row(i)+m_data.row(i+1))/2.0;
        m_tangents.row(i)=m_data.row(i+1)-m_data.row(i);
    }
    m_computedCenterAndTangents=true;
}

//PutInSameDirection(fiber) should be called before
//Compute the max euclidian distance between the endpoints of the two fibers
float Fiber::getMaxDistEndPoints(Fiber &fiber)
{
    float distanceEndpoints1=((*this)[0]-fiber[0]).squaredNorm();
    float distanceEndpoints2=((*this)[m_nb_points-1]-fiber[fiber.size()-1]).squaredNorm();
    return (max(distanceEndpoints1, distanceEndpoints2));
}

void Fiber::addNeighbours(vector<unsigned int> new_neighbours)
{
    for (unsigned int i=0; i<new_neighbours.size(); i++)
        m_neighbours.insert(new_neighbours[i]);
}

void Fiber::addToHierarchy(vector<unsigned int> new_elements)
{
    for (unsigned int i=0; i<new_elements.size(); i++)
        m_hierarchy.insert(new_elements[i]);
}

vector<unsigned int> Fiber::getHierarchy()
{
    vector<unsigned int> hierarchy;
    for (set<unsigned int>::iterator ite=m_hierarchy.begin(); ite!=m_hierarchy.end(); ++ite)
        hierarchy.push_back((unsigned int)*ite);
    return hierarchy;
}

float Fiber::getSelfScalarProduct(float lambdag)
{
    if (m_selfScalarProductComputed)
        return m_selfScalarProduct;
    float scalarProduct=0;
    float kg;
    for (unsigned int i=0; i<m_nb_points-1; i++)
        for (unsigned int j=0; j<m_nb_points-1; j++)
        {
            kg=exp(-(this->getCenter(i)-this->getCenter(j)).squaredNorm()/(lambdag*lambdag));
            scalarProduct+=this->getTangent(i).transpose()*kg*this->getTangent(j);
        }
    m_selfScalarProduct=scalarProduct;
    m_selfScalarProductComputed=true;
    return m_selfScalarProduct;
}

float Fiber::getSelfVarifoldScalarProduct(float sigma)
{
    if (m_selfScalarProductComputed)
        return m_selfScalarProduct;
    this->computeCentersAndTangents();
    float scalarProduct=0;
    float ksigma=0, kn=0;
    for (unsigned int i=0; i<m_nb_points-1; i++)
        for (unsigned int j=0; j<m_nb_points-1; j++)
        {
            ksigma=exp(-(this->getCenter(i)-this->getCenter(j)).squaredNorm()/(sigma*sigma));
            kn=(this->getTangent(i).transpose()*this->getTangent(j));
            scalarProduct+=ksigma*kn*kn/(this->getTangent(i).norm()*this->getTangent(j).norm());
        }
    m_selfScalarProduct=scalarProduct;
    m_selfScalarProductComputed=true;
    return m_selfScalarProduct;
}

void Fiber::moveExtremities(Vector3f deviation)
{
    m_data.row(0)+=deviation;
    m_data.row(m_nb_points-1)+=deviation;
}

/** Getter of the variable nb_neighboursNeighbours */
unsigned int Fiber::getNbNeighbours(unsigned int i)
{
  return nb_neighbours[i];
}

/** Setter of the variable nb_neighbours */
void Fiber::setNbNeighbours(unsigned int i, unsigned int j)
{
  nb_neighbours[i] = j;
}
