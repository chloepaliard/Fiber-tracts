#ifndef METRIQUE_H
#define METRIQUE_H

#include "iostream"

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/Geometry>
#include <Eigen/Dense>

#include "fiber.h"

using namespace Eigen;
using namespace std;

float MC(Fiber &fiber1, Fiber &fiber2);
float SC(Fiber &fiber1, Fiber &fiber2);
float LC(Fiber &fiber1, Fiber &fiber2);
float MDF(Fiber &fiber1, Fiber &fiber2);
float PDM(Fiber &fiber1, Fiber &fiber2, float sigma);
float VarifoldDistance(Fiber &fiber1, Fiber &fiber2, float sigma);
float CurrentDistance(Fiber &fiber1, Fiber &fiber2, float sigma);
float WeightedCurrentsDistance(Fiber &fiber1, Fiber &fiber2, float lambdac, float lambdab, float lambdag);
float averageOfPointwiseEuclideanMetric(Fiber &fiber1, Fiber &fiber2); //Metric used by Quickbundles
float metriqueTest(Fiber fiber1, Fiber fiber2, bool &is_valid);

float endpointsDistance(Fiber &fiber1, Fiber &fiber2);


#endif // METRIQUE_H
