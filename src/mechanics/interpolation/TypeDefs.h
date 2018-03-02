#pragma once
#include <Eigen/Core>

namespace NuTo
{

typedef Eigen::VectorXd NodeValues;
typedef Eigen::MatrixXd DerivativeShapeFunctionsNatural;
typedef Eigen::MatrixXd DerivativeShapeFunctionsGlobal;
typedef Eigen::MatrixXd BMatrixStrain;
typedef Eigen::MatrixXd BMatrixGradient;
typedef Eigen::MatrixXd NMatrix;

constexpr int maxDim = 3;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor, NuTo::maxDim, 1> NaturalCoords;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1, Eigen::ColMajor, NuTo::maxDim, 1> GlobalCoords;

} /* NuTo */
