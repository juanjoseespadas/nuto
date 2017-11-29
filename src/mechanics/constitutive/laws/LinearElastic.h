#pragma once
#include <type_traits>
#include <tuple>
#include "mechanics/constitutive/laws/MechanicsInterface.h"
#include "mechanics/constitutive/inputoutput/ConstitutivePlaneStateEnum.h"

namespace NuTo
{
namespace Laws
{

template <int TDim>
class LinearElastic : public MechanicsInterface<TDim>
{
public:
    using typename MechanicsInterface<TDim>::MechanicsTangent;

    template <int U = TDim, typename std::enable_if<U != 2, int>::type = 0>
    LinearElastic(double E, double Nu)
        : mE(E)
        , mNu(Nu)
        , mC(CalculateC(E, Nu))
    {
    }

    template <int U = TDim, typename std::enable_if<U == 2, int>::type = 0>
    LinearElastic(double E, double Nu, ePlaneState planeState)
        : mE(E)
        , mNu(Nu)
        , mC(CalculateC(E, Nu, planeState))
    {
    }

    EngineeringStressPDE<TDim> Stress(EngineeringStrainPDE<TDim> strain, double = 0, int = 0, int = 0) const override
    {
        return mC * strain;
    }

    MechanicsTangent Tangent(EngineeringStrainPDE<TDim>, double = 0, int = 0, int = 0) const override
    {
        return mC;
    }

private:
    double mE;
    double mNu;
    MechanicsTangent mC;

    static MechanicsTangent CalculateC(double E, double Nu, ePlaneState planeState = ePlaneState::PLANE_STRESS);
};

//! @brief calculate coefficients of the PLANE_STRESS 2D material matrix
//! @param E ... Young's modulus
//! @param Nu ... Poisson's ratio
//! @return tuple <C11, C12, C33>
std::tuple<double, double, double> CalculateCoefficients2DPlaneStress(double E, double Nu)
{
    double factor = E / (1.0 - (Nu * Nu));
    return std::make_tuple(factor, // C11
                           factor * Nu, // C12
                           factor * 0.5 * (1.0 - Nu)); // C33
}

//! @brief calculate coefficients of the 3D material matrix
//! @param E ... Young's modulus
//! @param Nu ... Poisson's ratio
//! @return tuple <C11, C12, C33>
std::tuple<double, double, double> CalculateCoefficients3D(double E, double Nu)
{
    double factor = E / ((1.0 + Nu) * (1.0 - 2.0 * Nu));
    return std::make_tuple(factor * (1.0 - Nu), // C11
                           factor * Nu, // C12
                           E / (2. * (1.0 + Nu))); // C33
}

template <>
LinearElastic<1>::MechanicsTangent LinearElastic<1>::CalculateC(double E, double Nu, ePlaneState)
{
    return MechanicsTangent::Constant(E);
}

template <>
LinearElastic<2>::MechanicsTangent LinearElastic<2>::CalculateC(double E, double Nu, ePlaneState planeState)
{
    double C11 = 0, C12 = 0, C33 = 0;
    if (planeState == ePlaneState::PLANE_STRESS)
        std::tie(C11, C12, C33) = CalculateCoefficients2DPlaneStress(E, Nu);
    else
        std::tie(C11, C12, C33) = CalculateCoefficients3D(E, Nu);

    MechanicsTangent C = MechanicsTangent::Zero();
    C = Eigen::Matrix3d::Zero();
    C(0, 0) = C11;
    C(1, 0) = C12;

    C(0, 1) = C12;
    C(1, 1) = C11;

    C(2, 2) = C33;
    return C;
}

template <>
LinearElastic<3>::MechanicsTangent LinearElastic<3>::CalculateC(double E, double Nu, ePlaneState)
{
    double C11 = 0, C12 = 0, C44 = 0;
    std::tie(C11, C12, C44) = CalculateCoefficients3D(E, Nu);
    MechanicsTangent C = MechanicsTangent::Zero();
    // C11 diagonal:
    C(0, 0) = C11;
    C(1, 1) = C11;
    C(2, 2) = C11;

    // C12 off diagonals:
    C(0, 1) = C12;
    C(0, 2) = C12;
    C(1, 0) = C12;
    C(1, 2) = C12;
    C(2, 0) = C12;
    C(2, 1) = C12;

    // C44 diagonal:
    C(3, 3) = C44;
    C(4, 4) = C44;
    C(5, 5) = C44;
    return C;
}

} /* Laws */
} /* NuTo */
