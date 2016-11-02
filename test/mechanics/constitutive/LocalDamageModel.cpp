
#include "nuto/mechanics/constitutive/laws/LocalDamageModel.h"

#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/sections/SectionEnum.h"

#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveCalculateStaticData.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStress.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveScalar.h"
#include "ConstitutiveTangentTester.h"

#include <iostream>

#define BOOST_TEST_MODULE LocalDamageModelTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
// necessary to build with clang when boost has been compiled by gcc
std::string boost::unit_test::ut_detail::normalize_test_case_name(const_string name)
{
    return (name[0] == '&' ? std::string(name.begin()+1, name.size()-1) : std::string(name.begin(), name.size() ));
}


using std::cout;
using std::endl;
using NuTo::Constitutive::eInput;
using NuTo::Constitutive::eOutput;
using NuTo::Constitutive::eConstitutiveParameter;

template <int TDim>
void EvaluateLocalDamageModelModel(NuTo::EngineeringStrain<TDim> rStrain, NuTo::LocalDamageModel& rLocalDamageModel, double rKappa)
{
    auto iplaw = rLocalDamageModel.CreateIPLaw();
    iplaw->GetData<NuTo::LocalDamageModel>().SetData(rKappa);
    NuTo::Test::ConstitutiveTangentTester<TDim> tester(*iplaw.get(), 1.e-8, 1.e-5);

    NuTo::ConstitutiveInputMap input;
    input.Add<TDim>(eInput::ENGINEERING_STRAIN);
    input.Add<TDim>(eInput::CALCULATE_STATIC_DATA);
    input.Add<TDim>(eInput::PLANE_STATE);

    input[eInput::ENGINEERING_STRAIN]->AsEngineeringStrain<TDim>() = rStrain;
    dynamic_cast<NuTo::ConstitutiveCalculateStaticData&>(*input[eInput::CALCULATE_STATIC_DATA])
        .SetCalculateStaticData(NuTo::eCalculateStaticData::EULER_BACKWARD);

    auto& planeState = dynamic_cast<NuTo::ConstitutivePlaneState&>(*input[eInput::PLANE_STATE]);
    planeState.SetPlaneState(NuTo::ePlaneState::PLANE_STRESS);


    cout << "Input strain: " << rStrain.CopyToEigenMatrix().transpose() << endl;


    BOOST_CHECK(tester.CheckTangent(input,
                    eInput::ENGINEERING_STRAIN,
                    eOutput::ENGINEERING_STRESS,
                    eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN));

    if (TDim == 2)
    {
        planeState.SetPlaneState(NuTo::ePlaneState::PLANE_STRAIN);

        BOOST_CHECK(tester.CheckTangent(input,
                                        eInput::ENGINEERING_STRAIN,
                                        eOutput::ENGINEERING_STRESS,
                                        eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN));
    }
}

BOOST_AUTO_TEST_CASE(check_d_stress_d_strain)
{
    constexpr double youngsModulus          = 4e4;
    constexpr double poissonsRatio          = 0.2;
    constexpr double tensileStrength        = 3;
    constexpr double compressiveStrength    = 30;
    constexpr double fractureEnergy         = 0.01;


    NuTo::LocalDamageModel localDamageModel;
    localDamageModel.SetParameterDouble(eConstitutiveParameter::YOUNGS_MODULUS,       youngsModulus);
    localDamageModel.SetParameterDouble(eConstitutiveParameter::POISSONS_RATIO,       poissonsRatio);
    localDamageModel.SetParameterDouble(eConstitutiveParameter::TENSILE_STRENGTH,     tensileStrength);
    localDamageModel.SetParameterDouble(eConstitutiveParameter::COMPRESSIVE_STRENGTH, compressiveStrength);
    localDamageModel.SetParameterDouble(eConstitutiveParameter::FRACTURE_ENERGY,      fractureEnergy);

    double kappa_0 = tensileStrength / youngsModulus;
    double kappa = kappa_0/3.;
    EvaluateLocalDamageModelModel<2>({    0.,     0.,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 1.e-5,     0.,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({-1.e-5,     0.,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 1.e-5,  1.e-5,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 2.e-5,  1.e-5,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 2.e-5, -1.e-5,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({     0,      0,  2.e-5}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 1.e-5,  1.e-5,  2.e-5}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({ 1.e-5, -2.e-5,  2.e-5}, localDamageModel , kappa);

    // some test in damaged loading
    kappa = 2*kappa_0;
    double eps = 1.e-7; // small load increment = damaged loading
    EvaluateLocalDamageModelModel<2>({kappa+eps,   0.,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({kappa,      eps,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({kappa,       0.,    eps}, localDamageModel , kappa);

    // decrement = elastic unloading
    EvaluateLocalDamageModelModel<2>({kappa-eps,   0.,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({kappa,     -eps,     0.}, localDamageModel , kappa);
    EvaluateLocalDamageModelModel<2>({kappa,       0.,   -eps}, localDamageModel , kappa);
}


