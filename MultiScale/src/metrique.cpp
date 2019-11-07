#include "../include/metrique.h"
#include "../include/fiber.h"

float fiberLength(const MatrixXf &fiber, int size)
{
    float length=0;
    for (int i=0; i<size-1; i++)
        length+=(fiber.row(i+1)-fiber.row(i)).squaredNorm();
    return length;
}

float minimum(const VectorXf &tableau, int size)
{
    float mini=tableau(0);
    for (int i=1; i<size; i++)
        if (mini>tableau(i)) mini=tableau(i);
    if (mini<0) cerr << "Négatif : " << mini << endl;
    return mini;
}

float MC(Fiber &fiber1, Fiber &fiber2)
{
    MatrixXf distances;
    float dm12=0;
    float dm21=0;
    float length1;
    float length2;

    length1=fiber1.length();
    length2=fiber2.length();

    distances.resize(fiber1.size(), fiber2.size());
    for (unsigned int i=0; i<fiber1.size(); i++)
        for (unsigned int j=0; j<fiber2.size(); j++)
        {
            distances(i,j)= (fiber1[i]-fiber2[j]).squaredNorm();
            if (distances(i,j)<0) cerr << "Négatif : " << distances(i,j) << endl;
        }

    for (unsigned int i=0; i<fiber1.size(); i++)
        dm12+=minimum(distances.row(i), fiber2.size())/length1;
    for (unsigned int i=0; i<fiber2.size(); i++)
        dm21+=minimum(distances.col(i), fiber1.size())/length2;

    distances.resize(0,0);

    return (dm12+dm21)/2;
}

float averageOfPointwiseEuclideanMetric(Fiber &fiber1, Fiber &fiber2)
{
    if (fiber1.size()!=fiber2.size())
    {
        if (fiber1.size()>fiber2.size())
            fiber2.resample(fiber1.size());
        else
            fiber1.resample(fiber2.size());
    }
    float sum=0;
    for (unsigned int i=0; i<fiber1.size(); i++)
        sum+=(fiber1[i]-fiber2[i]).squaredNorm();
    sum/=fiber1.size();
    return sum;
}

//Shorter mean of closest distances
float SC(Fiber &fiber1, Fiber &fiber2)
{
    MatrixXf distances;
    float dm12=0;
    float dm21=0;
    float length1;
    float length2;

    length1=fiber1.length();
    length2=fiber2.length();

    distances.resize(fiber1.size(), fiber2.size());
    for (unsigned int i=0; i<fiber1.size(); i++)
        for (unsigned int j=0; j<fiber2.size(); j++)
        {
            distances(i,j)= (fiber1[i]-fiber2[j]).squaredNorm();
            if (distances(i,j)<0) cerr << "Négatif : " << distances(i,j) << endl;
        }

    for (unsigned int i=0; i<fiber1.size(); i++)
        dm12+=minimum(distances.row(i), fiber2.size())/length1;
    for (unsigned int i=0; i<fiber2.size(); i++)
        dm21+=minimum(distances.col(i), fiber1.size())/length2;

    distances.resize(0,0);

    return min(dm12,dm21);
}

//Longer mean of closest distances
float LC(Fiber &fiber1, Fiber &fiber2)
{
    MatrixXf distances;
    float dm12=0;
    float dm21=0;
    float length1;
    float length2;

    length1=fiber1.length();
    length2=fiber2.length();

    distances.resize(fiber1.size(), fiber2.size());
    for (unsigned int i=0; i<fiber1.size(); i++)
        for (unsigned int j=0; j<fiber2.size(); j++)
        {
            distances(i,j)= (fiber1[i]-fiber2[j]).squaredNorm();
            if (distances(i,j)<0) cerr << "Négatif : " << distances(i,j) << endl;
        }

    for (unsigned int i=0; i<fiber1.size(); i++)
        dm12+=minimum(distances.row(i), fiber2.size())/length1;
    for (unsigned int i=0; i<fiber2.size(); i++)
        dm21+=minimum(distances.col(i), fiber1.size())/length2;

    distances.resize(0,0);

    return max(dm12,dm21);
}

//MDF distance
float MDF(Fiber &fiber1, Fiber &fiber2)
{
    if (fiber1.size()!=fiber2.size())
    {
        cerr << "Error : fibers must be the same size, use the fiber::resample(unsigned int nb_points) function" << endl;
        return -1;
    }
    float ddirect=0;
    float dflipped=0;
    unsigned int size = fiber1.size();

    for (unsigned int i=0; i<size; i++)
    {
        ddirect+=(fiber1[i]-fiber2[i]).squaredNorm();
        dflipped+=(fiber1[i]-fiber2[size-i]).squaredNorm();
    }
    ddirect/=(float)size;
    dflipped/=(float)size;

    return min(ddirect, dflipped);
}

float scalarProductPDM(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    float scalarProduct=0;
    float denominator=fiber1.size()*fiber2.size();
    for (unsigned int i=0; i<fiber1.size(); i++)
        for (unsigned int j=0; j<fiber2.size(); j++)
            scalarProduct+=exp(-(fiber1[i]-fiber2[j]).squaredNorm()/(sigma*sigma))/denominator;
    return scalarProduct;
}

//Point Density Model
float PDM(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    float scalarProduct11=scalarProductPDM(fiber1, fiber1, sigma);
    float scalarProduct22=scalarProductPDM(fiber2, fiber2, sigma);
    float scalarProduct12=scalarProductPDM(fiber1, fiber2, sigma);
    return sqrt(scalarProduct11+scalarProduct22-2*scalarProduct12);
}

float scalarProductVD(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    float scalarProduct=0;
    float ksigma=0, kn=0;
    for (unsigned int i=0; i<fiber1.size()-1; i++)
    {
        for (unsigned int j=0; j<fiber2.size()-1; j++)
        {
            ksigma=exp(-(fiber1.getCenter(i)-fiber2.getCenter(j)).squaredNorm()/(sigma*sigma));
            kn=(fiber1.getTangent(i).transpose()*fiber2.getTangent(j));
            scalarProduct+=ksigma*kn*kn/(fiber1.getTangent(i).norm()*fiber2.getTangent(j).norm());
//            if (isnan(scalarProduct) && fiber1.getTangent(i).norm()*fiber2.getTangent(j).norm()==0)
//            {
//#pragma omp critical
//                {
//                cout << "Scalar product nan : " << fiber1.getTangent(i) << " " <<fiber2.getTangent(j) << endl;
//                cout << i << " " << j << " " << fiber1.size() << " " << fiber2.size() << endl;
//                }
//            }
        }
    }
    return scalarProduct;
}

//Varifold distance
float VarifoldDistance(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    fiber1.computeCentersAndTangents();
    fiber2.computeCentersAndTangents();
    float scalarProduct11=fiber1.getSelfVarifoldScalarProduct(sigma);//scalarProductVD(fiber1, fiber1, sigma);//
    float scalarProduct22=fiber2.getSelfVarifoldScalarProduct(sigma);//scalarProductVD(fiber2, fiber2, sigma);//
    float scalarProduct12=scalarProductVD(fiber1, fiber2, sigma);
    return sqrt(scalarProduct11+scalarProduct22-2*scalarProduct12);
}

float scalarProductCurrent(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    float scalarProduct=0;
    float ksigma=0, kn=0;
    for (unsigned int i=0; i<fiber1.size()-1; i++)
    {
        for (unsigned int j=0; j<fiber2.size()-1; j++)
        {
            ksigma=exp(-(fiber1.getCenter(i)-fiber2.getCenter(j)).squaredNorm()/(sigma*sigma));
            kn=fiber1.getTangent(i).dot(fiber2.getTangent(j));
            scalarProduct+=ksigma*kn;
        }
    }
    return scalarProduct;
}

//Current distance
float CurrentDistance(Fiber &fiber1, Fiber &fiber2, float sigma)
{
    fiber1.computeCentersAndTangents();
    fiber2.computeCentersAndTangents();
    float scalarProduct11=scalarProductCurrent(fiber1, fiber1, sigma);
    float scalarProduct22=scalarProductCurrent(fiber2, fiber2, sigma);
    float scalarProduct12=scalarProductCurrent(fiber1, fiber2, sigma);
    return sqrt(scalarProduct11+scalarProduct22-2*scalarProduct12);
}

float scalarProductWeightedCurrents(Fiber &fiber1, Fiber &fiber2, float lambdac, float lambdab, float lambdag)
{
    float scalarProduct=0;
    float kc, kb, kg;
    for (unsigned int i=0; i<fiber1.size()-1; i++)
    {
        for (unsigned int j=0; j<fiber2.size()-1; j++)
        {
            kg=exp(-(fiber1.getCenter(i)-fiber2.getCenter(j)).squaredNorm()/(lambdag*lambdag));
            scalarProduct+=fiber1.getTangent(i).transpose()*kg*fiber2.getTangent(j);
        }
    }
    kc=exp(-(fiber1[0]-fiber2[0]).squaredNorm()/(lambdac*lambdac));
    kb=exp(-(fiber1[fiber1.size()-1]-fiber2[fiber2.size()-1]).squaredNorm()/(lambdab*lambdab));
    scalarProduct*=kc*kb;
    return scalarProduct;
}

//Weighted currents distance
float WeightedCurrentsDistance(Fiber &fiber1, Fiber &fiber2, float lambdac, float lambdab, float lambdag)
{        
    fiber1.computeCentersAndTangents();
    fiber2.computeCentersAndTangents();
    float scalarProduct11=fiber1.getSelfScalarProduct(lambdag);//scalarProductWeightedCurrents(fiber1, fiber1, lambdac, lambdab, lambdag);
    float scalarProduct22=fiber2.getSelfScalarProduct(lambdag);//scalarProductWeightedCurrents(fiber2, fiber2, lambdac, lambdab, lambdag);
    float scalarProduct12=scalarProductWeightedCurrents(fiber1, fiber2, lambdac, lambdab, lambdag);
    return sqrt(scalarProduct11+scalarProduct22-2*scalarProduct12);
}

float metriqueTest(Fiber fiber1, Fiber fiber2, bool & is_valid/*, int metric*/)
{
    is_valid=true;
    fiber1.PutInSameDirection(fiber2);

    int metric=1;

    if (metric==0)
    {
        fiber1.computeCentersAndTangents();
        fiber2.computeCentersAndTangents();
        float scalarProduct = fabs(scalarProductWeightedCurrents(fiber1, fiber2, 6,6,8));
        if (scalarProduct<0)
            is_valid=false;
        return (-scalarProduct);
    }
    else if (metric==1)
    {
        return MDF(fiber1, fiber2);//averageOfPointwiseEuclideanMetric(fiber1, fiber2);
    }
    else if (metric==2)
    {
        return WeightedCurrentsDistance(fiber1, fiber2, 6, 6, 8);
    }
    else if (metric==3)
    {
        return MC(fiber1, fiber2);
    }
    else
    {
        return VarifoldDistance(fiber1, fiber2, 15);
    }
}

//Distance between the endpoints of fibers oriented in the same direction
float endpointsDistance(Fiber &fiber1, Fiber &fiber2)
{
    return (fiber1[0]-fiber2[0]).squaredNorm()+(fiber1[fiber1.size()-1]-fiber2[fiber2.size()-1]).squaredNorm();
}
