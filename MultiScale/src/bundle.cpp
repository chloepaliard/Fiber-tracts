#include "../include/bundle.h"
#include "../../interface/widgetopengl.h"

void advanceBarCout(float percent)
{
    int nbBar = int(percent/5);
    if(percent < 100)
        std::cout << std::setfill('0') << std::setw(2) << int(percent) << " % |";
    else
        std::cout << "Done |";
    for(int i = 0; i < nbBar; ++i)
        std::cout << "–";
    for(int i = nbBar; i < 20; ++i)
        std::cout << " ";
    std::cout << "|\r";
    std::cout.flush();
}

Bundle::Bundle()
{
  nbOfIterations = 1;
}

float ReverseFloat( const float inFloat )
{
   float retVal;
   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}

void Bundle::loadFibers(string filename)
{
    srand(time(NULL));
    // Get all data from the file
    fstream file;
    file.open(filename, ios_base::in | ios_base::binary);
    if (!file.is_open()) {cerr << "Error while opening the file" << endl; return;}
    char s[256];
    file.getline(s, 256);//File version and identifier
    file.getline(s, 256);//Description
    file.getline(s, 256);//BINARY or ASCII
    if (strcmp(s,"BINARY")!=0) {cerr << "Error while opening the file, not a binary file ?" << endl; return;}
    file.getline(s, 256);
    if (strcmp(s,"DATASET POLYDATA")!=0) {cerr << "Error while opening the file, not a polydata ?" << endl; return;}
    file.getline(s, 256, ' ');
    if (strcmp(s,"POINTS")!=0) {cerr << "Error while opening the file, not points ?" << endl; return;}
    file.getline(s, 256, ' ');
    m_nbOfPoints=stoi(s);
    cout << m_nbOfPoints << " points in file " << filename << " found" << endl;
    file.getline(s, 256);
    if (strcmp(s,"float")!=0) {cerr << "Error while opening the file, values need to be floats" << endl; return;}

    vector<Vector3f> points(m_nbOfPoints);
    for (unsigned int i=0; i<m_nbOfPoints; i++)
        file.read(reinterpret_cast<char *>(&points[i]), sizeof(Vector3f));

    while (strstr(s, "LINES")==NULL)
    {
        file.getline(s, 256, ' ');
    }
    file.getline(s, 256, ' ');
    unsigned int numberOfFibers=stoi(s);
    cout << numberOfFibers << " lines found" << endl;
    file.getline(s, 256);

    unsigned int value;
    unsigned int fibSize;
    MatrixXf Points;
    for (unsigned int i=0; i<numberOfFibers; i++)
    {
        file.read(reinterpret_cast<char *>(&fibSize), sizeof(int));
        fibSize=htonl(fibSize);
        Points.resize(fibSize, 3);
        for (unsigned int j=0; j<fibSize; j++)
        {
            file.read(reinterpret_cast<char *>(&value), sizeof(int));
            value=htonl(value);
            Points.row(j)=Vector3f(ReverseFloat(points[value](0)), ReverseFloat(points[value](1)), ReverseFloat(points[value](2)));
        }
        Fiber newFib(Points, fibSize);
        this->addFiber(newFib);
    }
    for (unsigned int i=0; i<this->size(); i++)
        m_fibers[i].addToHierarchy(i);
    return;
}

vector<vector<Vector3f> > Bundle::getFibers()
{
    vector<vector<Vector3f> > fibres(m_fibers.size());
    for (unsigned int i=0; i<m_fibers.size(); i++)
    {fibres[i].resize(m_fibers[i].size());
        for (unsigned int j=0; j<m_fibers[i].size(); j++)
            fibres[i][j]=m_fibers[i][j];
    }
    return fibres;
}

Fiber& Bundle::operator[](unsigned int element)
{
    if (element>=m_fibers.size())
    {
        cerr << "Erreur : accès à un élément inexistant" << endl;
        exit(EXIT_FAILURE);
    }
    return m_fibers[element];
}

void Bundle::addFiber(Fiber &new_fiber)
{
#if (TEST_FIBERS_VALIDITY)
    {
        float ecart=0.00001f;
        bool same=false;
        unsigned int i=0;
        while (!same && i<m_fibers.size())
        {
            new_fiber.PutInSameDirection(m_fibers[i]);
            if (new_fiber[0]==m_fibers[i][0] && new_fiber.size()==m_fibers[i].size())
            {
                same=true;
                for (unsigned int j=0; j<new_fiber.size(); j++)
                    same = same && (new_fiber[j]==m_fibers[i][j]);
            }
            if (!same && (new_fiber[0]==m_fibers[i][0] || new_fiber[new_fiber.size()-1]==m_fibers[i][m_fibers[i].size()-1]))
            {
                cerr << "Extrémité identique" << endl;
                new_fiber.moveExtremities(Vector3f(ecart, ecart, ecart));
            }
            i++;
        }
        if (same)
            cerr << "Fibre en double trouvée" << endl;
        else
        {
            m_nbOfPoints+=new_fiber.size();
            m_fibers.push_back(new_fiber);
        }
    }
#else
    {
        m_nbOfPoints+=new_fiber.size();
        m_fibers.push_back(new_fiber);
    }
#endif
}

int Bundle::addFiberSize(Fiber &new_fiber)
{
    addFiber(new_fiber);
    return m_fibers.size();
}

void Bundle::reOrienteFibers(unsigned int reference)
{
#pragma omp parallel for
    for (unsigned int i=0; i<m_fibers.size(); i++)
        m_fibers[i].PutInSameDirection(m_fibers[reference]);
}

/** Resampling of the fibers so that the bigger ones and the smaller ones have approximatly the same sample */
void Bundle::resample(float resampleValue)
{
    for (unsigned int i=0; i<m_fibers.size(); i++)
        m_fibers[i].resample(resampleValue);
}

void Bundle::set_widget(WidgetOpenGL* widget_param)
{
    m_pwidget = widget_param;
}

unsigned int Bundle::getCurrentNberOfPoints()
{
    unsigned int value=0;
#pragma omp parallel for reduction (+:value)
    for (unsigned int i=0; i<m_fibers.size(); i++)
        if (m_fibers[i].is_valid())
            value=value+m_fibers[i].size();
    return value;
}

unsigned int Bundle::getCurrentNberOfFibers()
{
    unsigned int value=0;
#pragma omp parallel for reduction (+:value)
    for (unsigned int i=0; i<m_fibers.size(); i++)
        if (m_fibers[i].is_valid())
            value++;
    return value;
}

void Bundle::computeTangents()
{
    for (unsigned int i=0; i<size(); i++)
        m_fibers[i].computeCentersAndTangents();
}

/** Function that calculate the distance between two neighbours */
float Bundle::distance(Vector3f sommet1, Vector3f sommet2) {
  return (sommet1-sommet2).norm();
}

/** Function that tests the first condition: distance<dmax */
vector<unsigned int> Bundle::closeFibers(unsigned int nbfibre1, unsigned int nbfibre2,float dmax)
{

  Fiber f1=m_fibers[nbfibre1];
  Fiber f2=m_fibers[nbfibre2];
  float longueur;
  vector<unsigned int> indice; // Vertex with the indexes of the closest neighbour if it has some or if it doesn't, it contains the size of the fiber2
  bool test;

  for ( unsigned int i=0 ; i<f1.size() ; i++) {
    longueur=this->distance( f1[i] , f2[0] ); // To compare the distances with a value
    indice.push_back(0);
    test=(longueur<=dmax); // To test if index 0 verifies the condition or not
    for (unsigned int j=1 ; j<f2.size() ; j++)
    {
      float dist=this->distance( f1[i] , f2[j] );
      if (dist<longueur && dist <=dmax)
      {  // we only keep the closest neighbour
        indice[i]=j;
        longueur=dist;
        test=true;
      }
    }
    if (test == false )
    {
      indice[i]=f2.size();//if the index 0 doesn't verify the condition and its doesn't have any neighbour, we put the size of the fiber2
    }
  }
  return(indice);
}

/** Function that tests the second condition, tests if the fibers are parallel enough */
vector<vertexPair> Bundle::getNeighbours2Fibers(unsigned int nbfibre1, unsigned int nbfibre2,float dmax, float theta) {

  Fiber f1=m_fibers[nbfibre1];
  Fiber f2=m_fibers[nbfibre2];
  vector<unsigned int> indice;
  vector<vertexPair> neighbours;
  float prod, prod1, prod2;
  vector<float> prodscal;
  unsigned int i;

  indice = closeFibers(nbfibre1, nbfibre2, dmax);  // Test of the first condition

  for (unsigned int i=1 ; i<f1.size()-1 ; i++) { // We test the vertexes inside the fibers (not the ends)
    if (indice[i]>0 && indice[i]<f2.size()-1) {
      prodscal.push_back(f1.getTangent(i)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i])[2]); // Calculating the dot product between the two vertexes (f1&2)
      prodscal.push_back(f1.getTangent(i)[0]*f2.getTangent(indice[i]-1)[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i]-1)[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i]-1)[2]); // Calculating the dot product between the vertex (f1) and the previous vertex (2)
      prodscal.push_back(f1.getTangent(i-1)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i-1)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i-1)[2]*f2.getTangent(indice[i])[2]); // Calculating the dot product between the previous vertex (f1) and the vertex (f2)
      prodscal.push_back(f1.getTangent(i-1)[0]*f2.getTangent(indice[i]-1)[0]+f1.getTangent(i-1)[1]*f2.getTangent(indice[i]-1)[1]+f1.getTangent(i-1)[2]*f2.getTangent(indice[i]-1)[2]); // Calculating the dot product between the two previous vertexes

      if (!(acos(prodscal[0])<=theta || acos(prodscal[1])<=theta  || acos(prodscal[2])<=theta || acos(prodscal[3])<=theta)) // if the angle between the two fibers is too big
        {
          indice[i]= f2.size();  // No neighbour so we put the size of the second fiber
        }
    }
  }

////////////////////////////////////////////////////////// GESTIONS DES EXTREMITES ////////////////////////////////////////////////////////


  if (indice[0]==0) //Beginning of the 1st and 2nd fibre
  {
    prod = f1.getTangent(0)[0]*f2.getTangent(0)[0]+f1.getTangent(0)[1]*f2.getTangent(0)[1]+f1.getTangent(0)[2]*f2.getTangent(0)[2];
    if (acos(prod)>theta)
    {
      indice[0]= f2.size();
    }
  }
  if (indice[0]==f2.size()-1) //Beginning of fiber1 and end of fiber 2
  {
    prod = f1.getTangent(0)[0]*f2.getTangent(f2.size())[0]+f1.getTangent(0)[1]*f2.getTangent(f2.size())[1]+f1.getTangent(0)[2]*f2.getTangent(f2.size())[2];
    if (acos(prod)>theta)
    {
      indice[0]= f2.size();
    }
  }
  if (indice[f1.size()-1]==0)  //End of fiber 1 and beginning of fiber 2
  {
    prod = f1.getTangent(f1.size()-1)[0]*f2.getTangent(0)[0]+f1.getTangent(f1.size()-1)[1]*f2.getTangent(0)[1]+f1.getTangent(f1.size()-1)[2]*f2.getTangent(0)[2];
    if (acos(prod)>theta)
    {
      indice[f1.size()-1]=  f2.size();
    }
  }
  if (indice[f1.size()-1]==f2.size()-1)  //End of fiber 1 and 2
  {
    prod = f1.getTangent(f1.size()-1)[0]*f2.getTangent(f2.size()-1)[0]+f1.getTangent(f1.size()-1)[1]*f2.getTangent(f2.size()-1)[1]+f1.getTangent(f1.size()-1)[2]*f2.getTangent(f2.size()-1)[2];
    if (acos(prod)>theta)
    {
      indice[f1.size()-1]= f2.size();
    }

    if (indice[0]>0 && indice[0]<f2.size()-1 ){ //Beginning of fiber 1 and inside of fiber 2
      i=0;
      prod1 = f1.getTangent(0)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(0)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(0)[2]*f2.getTangent(indice[i])[2];
      prod2 = f1.getTangent(0)[0]*f2.getTangent(indice[i]-1)[0]+f1.getTangent(0)[1]*f2.getTangent(indice[i]-1)[1]+f1.getTangent(0)[2]*f2.getTangent(indice[i]-1)[2];
      if (!(acos(prod1)<=theta || acos(prod2)<=theta ) ){
        indice[i]= f2.size();
      }
    }
    if (indice[f1.size()-1]>0 && indice[f1.size()-1]<f2.size()-1 ){ //End of fiber 1 and inside of fiber 2
      i=f1.size()-1;
      prod1 = f1.getTangent(i)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i])[2];
      prod2 = f1.getTangent(i)[0]*f2.getTangent(indice[i]-1)[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i]-1)[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i]-1)[2];
      if (!(acos(prod1)<=theta || acos(prod2)<=theta )) {
        indice[i]= f2.size();
      }
    }
    for (unsigned int i =1; i< f1.size()-1; i++) { //Inside of fiber 1 and beginning of fiber 2
      if (indice[i]==0){
        prod1 = f1.getTangent(i)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i])[2];
        prod2 = f1.getTangent(i-1)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i-1)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i-1)[2]*f2.getTangent(indice[i])[2];
        if (!(acos(prod1)<=theta || acos(prod2)<=theta ) ){
          indice[i]= f2.size();
        }
      }
      if (indice[i]==f2.size()-1) { // Inside of fiber 1 and end of fiber 2
        prod1 = f1.getTangent(i)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i)[2]*f2.getTangent(indice[i])[2];
        prod2 = f1.getTangent(i-1)[0]*f2.getTangent(indice[i])[0]+f1.getTangent(i-1)[1]*f2.getTangent(indice[i])[1]+f1.getTangent(i-1)[2]*f2.getTangent(indice[i])[2];
        if (!(acos(prod1)<=theta || acos(prod2)<=theta ) ){
          indice[i]= f2.size();
        }
      }
    }
  }
  for (unsigned int i=0 ; i<f1.size() ; i++)
  {
    if (indice[i]<f2.size())
    {
      vertexPair vertex={i,nbfibre1,indice[i],nbfibre2}; // We only put the vertexes that have a closest neighbour
      neighbours.push_back(vertex);
    }
  }

  return neighbours;
}

/** Testing of the 3rd condition and generating the similarity list */
vector<vertexPair> Bundle::createSimilarityList(float dmax, float theta)
{
  vector<vertexPair> similarityList;
  vector<vertexPair> neighboursB;
  vector<vertexPair> neighboursA;
  vertexPair vertex;
  for (unsigned int i=0; i<m_fibers.size(); i++) //going through the bundle
  {
    for (unsigned int j=i+1; j<m_fibers.size(); j++) //testing the fibers in a given order, so that we don't test a couple of fibers twice
    {
      neighboursB = getNeighbours2Fibers(i, j, dmax, theta); //list of neighbours for vertices from fiber i
      neighboursA = getNeighbours2Fibers(j, i, dmax, theta); //list of neighbours for vertices from fiber j

      for (unsigned int k=0; k<neighboursB.size(); k++)
      {
        unsigned int indS1 = neighboursB[k].indexVertex; // index of the Kth vertex from fiber i
        unsigned int indS2 = neighboursB[k].indexNeighbour; // index of its closest neighbour on fiber j
        for (unsigned int l=0; l<neighboursA.size(); l++)
        {
          unsigned int indS3 = neighboursA[l].indexVertex;  // index of the Lth vertex from fiber j
          unsigned int indS4 = neighboursA[l].indexNeighbour; // index of its closest neighbour on fiber i
          if (!((indS1 == indS4) && (indS2 == indS3))) //we don't add (q,p) if there is already (p,q) in the list
          {
            if (indS2 == indS3) // when we find the vertex on fiber j which is the same as S1's closest neighbour, S2
            {
              if (fabs(indS4 - indS1) <= 1) // third condition, only taking consecutive vertices
              {
                vertex={indS2, j, indS1, i};
                similarityList.push_back(vertex); // adding the couple of neighbours (S1, S2) if the third condition is verified
                m_fibers[j].setNbNeighbours(indS2,m_fibers[j].getNbNeighbours(indS2)+1);
                m_fibers[i].setNbNeighbours(indS1,m_fibers[i].getNbNeighbours(indS1)+1);
              }
            }
            else if (indS1 == indS4) // we find the vertex on fiber i which is the same as S3's closest neighbour, S4
            {
              if (fabs(indS3 - indS2) <= 1) // third condition, only taking consecutive vertices
              {
                vertex={indS3, j, indS4, i};
                similarityList.push_back(vertex); // adding the couple of neighbours (S3, S4) if the third condition is verified
                m_fibers[j].setNbNeighbours(indS3,m_fibers[j].getNbNeighbours(indS3)+1);
                m_fibers[i].setNbNeighbours(indS4,m_fibers[i].getNbNeighbours(indS4)+1);
              }
            }
          }
          else {
              vertex={indS1, i, indS2, j};
              similarityList.push_back(vertex); // adding the couple of neighbours (S1, S2) if (S1, S2) = (S4, S3), as the third condition is always verified
              m_fibers[j].setNbNeighbours(indS2,m_fibers[j].getNbNeighbours(indS2)+1);
              m_fibers[i].setNbNeighbours(indS1,m_fibers[i].getNbNeighbours(indS1)+1);
          }
        }
      }
    }
  }
  return similarityList;
}


/** Creates m_newData, new positions of vertexes after contraction */
void Bundle::newData(vector<vertexPair> list)
{
  Vector3f v0(0,0,0); //Vertex for initilization
  for (unsigned int i=0; i<this->size(); i++)// Initilization
  {
    m_fibers[i].resizeND(m_fibers[i].size(),3);
    m_fibers[i].resizeD(m_fibers[i].size(),3);
    for (unsigned int j=0; j<m_fibers[i].size(); j++)  // We go through every fiber
    {
      m_fibers[i].setNewData(j,m_fibers[i][j]);
      m_fibers[i].setDVector(j,v0);
    }
  }
  for (unsigned int k = 0; k<nbOfIterations; k++)// Number of iterations for contraction
  {
    for (unsigned int l=0; l<list.size(); l++) // We go through all the couples of neighbours
    {
      vertexPair p = list[l];
      Vector3f s1 = m_fibers[p.indexFibreVertex].getNewData(p.indexVertex);
      Vector3f s2 = m_fibers[p.indexFibreNeighbour].getNewData(p.indexNeighbour);
      Vector3f center = (s1+s2)/2;
      m_fibers[p.indexFibreVertex].setDVector(p.indexVertex, m_fibers[p.indexFibreVertex].getDVector(p.indexVertex)+center-s1); // We modify the deplacement vectors to bring the vertexes closer to their neighbour
      m_fibers[p.indexFibreNeighbour].setDVector(p.indexNeighbour, m_fibers[p.indexFibreNeighbour].getDVector(p.indexNeighbour)+center-s2);

    }

    for (unsigned int i=0; i<this->size(); i++)// We go through every fiber
    {
      for (unsigned int j=0; j<m_fibers[i].size(); j++)// We go through every vertex
      {

        Vector3f N = m_fibers[i].getNewData(min(j+1,m_fibers[i].size()-1)) - m_fibers[i].getNewData(max((int)j-1,0));
        Vector3f N_norm = N.normalized();
        Vector3f deplacement = m_fibers[i].getDVector(j);
        Vector3f proj = deplacement - (deplacement.dot(N_norm))*N_norm; // We take the projection so that the vertex only moves on the orthogonal plane of the fiber


        if (m_fibers[i].getNbNeighbours(j) != 0){
          m_fibers[i].setNewData(j,m_fibers[i].getNewData(j)+proj/m_fibers[i].getNbNeighbours(j)); // We move the vertex
          m_fibers[i].setDVector(j,v0); // We re-initialize the deplacement vector
        }
      }
    }
  }
}


/** Takes of the outliers vertexes */
void Bundle::noMoreOutliers(unsigned int minNbNeighbours)
{
  unsigned int compt;
  for (unsigned int i=0; i<m_fibers.size(); i++)
  {
    vector<Vector3f> filter;
    compt = 0;
    for (unsigned int j=0; j<m_fibers[i].size(); j++)
    {
      if (m_fibers[i].getNbNeighbours(j) >= minNbNeighbours) // If the vertex have a small number of neighbour
      {
        filter.push_back(m_fibers[i].getNewData(j));
        compt = compt +1;// Another pointer to follow only the valid vertexes (with enough neighbours)
      }

    }
    m_fibers[i].resizeND(compt,3); // Resizing m_newData to match the new number of vertexes
    for(unsigned int k=0 ; k<filter.size(); k++)
    {
      m_fibers[i].setNewData(k,filter[k]); // we fill out m_new data with the valid vertexes
    }
  }
}
