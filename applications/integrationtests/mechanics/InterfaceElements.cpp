//============================================================================
// Name        : InterfaceElements.cpp
// Author      : Philip Huschke
// Version     : 02 Sep 2015
// Copyright   :
// Description : Test for the interface element proposed by Goodman et al.
//               Fibre pullout test:
//
//
//============================================================================

#include "math/MathException.h"
#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "mechanics/groups/GroupEnum.h"
#include "mechanics/interpolationtypes/InterpolationTypeEnum.h"
#include "mechanics/nodes/NodeEnum.h"
#include "mechanics/sections/SectionEnum.h"
#include "mechanics/structures/unstructured/Structure.h"
#include "mechanics/timeIntegration/NewmarkDirect.h"
#include "visualize/VisualizeEnum.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>

constexpr unsigned int dimension = 2;

class Parameters
{
public:

    static const int mDimension = dimension;

    static const bool mPerformLineSearch = true;
    static const bool mAutomaticTimeStepping = true;

    static constexpr double mMatrixYoungsModulus = 4.0e4;
    static constexpr double mMatrixPoissonsRatio = 0.2;
    static constexpr double mMatrixThickness = 0.2;

    static constexpr double mFibreYoungsModulus = 2.1e5;
    static constexpr double mFibrePoissonsRatio = 0.2;
    static constexpr double mFibreCrossSection = 0.1;
    static constexpr double mFibreCircumference = 1.1;

    static constexpr double mInterfaceNormalStiffness = 1e6;
    static constexpr double mAlpha = 1;
    static constexpr double mMaxBondStress = 4e3;
    static constexpr double mResidualBondStress = 1e3;
    static constexpr double mSlipAtMaxBondStress = 0.5;
    static constexpr double mSlipAtResidualBondStress = 5;

    static constexpr double mTimeStep = 1.0e-2;
    static constexpr double mMinTimeStep = 1.0e-5;
    static constexpr double mMaxTimeStep = 1.0e-2;
    static constexpr double mToleranceForce = 1e-6;
    static constexpr double mSimulationTime = 1.0;
    static constexpr double mLoad = 10.0;

    static const Eigen::Matrix<double, dimension, 1> mDirectionX;
    static const Eigen::Matrix<double, dimension, 1> mDirectionY;
};

const Eigen::Matrix<double, dimension, 1> Parameters::mDirectionX = Eigen::Matrix<double, dimension, 1>::UnitX();
const Eigen::Matrix<double, dimension, 1> Parameters::mDirectionY = Eigen::Matrix<double, dimension, 1>::UnitY();

//////////////////////////////////////////////////////////
//  MAIN
//////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    try
    {

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Structure                **" << std::endl;
        std::cout << "***********************************" << std::endl;

        const boost::filesystem::path resultDir(boost::filesystem::initial_path().string() + "/results_interface_elements/");
        const boost::filesystem::path meshFile(boost::filesystem::initial_path().string() + "/InterfaceElements.msh");

        NuTo::Structure myStructure(Parameters::mDimension);
        myStructure.SetVerboseLevel(10);
        myStructure.SetShowTime(false);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Integration Scheme       **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::NewmarkDirect myIntegrationScheme(&myStructure);
        myIntegrationScheme.SetTimeStep(Parameters::mTimeStep);
        myIntegrationScheme.SetMinTimeStep(Parameters::mMinTimeStep);
        myIntegrationScheme.SetMaxTimeStep(Parameters::mMaxTimeStep);
        myIntegrationScheme.SetToleranceForce(Parameters::mToleranceForce);
        myIntegrationScheme.SetAutomaticTimeStepping(Parameters::mAutomaticTimeStepping);
        myIntegrationScheme.SetPerformLineSearch(Parameters::mPerformLineSearch);
        myIntegrationScheme.SetResultDirectory(resultDir.string(), true);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Section                  **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int matrixSection = myStructure.SectionCreate(NuTo::eSectionType::PLANE_STRESS);
        myStructure.SectionSetThickness(matrixSection, Parameters::mMatrixThickness);

        int fibreSection = myStructure.SectionCreate(NuTo::eSectionType::TRUSS);
        myStructure.SectionSetArea(fibreSection, Parameters::mFibreCrossSection);

        int fibreMatrixBond = myStructure.SectionCreate(NuTo::eSectionType::FIBRE_MATRIX_BOND);
        myStructure.SectionSetCircumference(fibreMatrixBond, Parameters::mFibreCircumference);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Material                 **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int matrixMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::LINEAR_ELASTIC_ENGINEERING_STRESS);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, Parameters::mMatrixYoungsModulus);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, Parameters::mMatrixPoissonsRatio);

        int fibreMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::LINEAR_ELASTIC_ENGINEERING_STRESS);
        myStructure.ConstitutiveLawSetParameterDouble(fibreMaterial, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, Parameters::mFibreYoungsModulus);
        myStructure.ConstitutiveLawSetParameterDouble(fibreMaterial, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, Parameters::mFibrePoissonsRatio);

        int interfaceMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::FIBRE_MATRIX_BOND_STRESS_SLIP);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::NORMAL_STIFFNESS, Parameters::mInterfaceNormalStiffness);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::ALPHA, Parameters::mAlpha);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::MAX_BOND_STRESS, Parameters::mMaxBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::RESIDUAL_BOND_STRESS, Parameters::mResidualBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::SLIP_AT_MAX_BOND_STRESS, Parameters::mSlipAtMaxBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::SLIP_AT_RESIDUAL_BOND_STRESS, Parameters::mSlipAtResidualBondStress);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Interpolation Type       **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int matrixInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::TRIANGLE2D);
        myStructure.InterpolationTypeAdd(matrixInterpolationType, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);
        myStructure.InterpolationTypeAdd(matrixInterpolationType, NuTo::Node::eDof::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);

        int fibreInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::TRUSSXD);
        myStructure.InterpolationTypeAdd(fibreInterpolationType, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);
        myStructure.InterpolationTypeAdd(fibreInterpolationType, NuTo::Node::eDof::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);

        int interfaceInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::INTERFACE);
        myStructure.InterpolationTypeAdd(interfaceInterpolationType, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);
        myStructure.InterpolationTypeAdd(interfaceInterpolationType, NuTo::Node::eDof::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Import Mesh File         **" << std::endl;
        std::cout << "***********************************" << std::endl;

        auto createdGroupIdMatrix = myStructure.ImportFromGmsh(meshFile.string());

        int groupIdFibre = createdGroupIdMatrix[0].first;
        int groupIdMatrix = createdGroupIdMatrix[1].first;

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Matrix                   **" << std::endl;
        std::cout << "***********************************" << std::endl;

        myStructure.ElementGroupSetInterpolationType(groupIdMatrix, matrixInterpolationType);
        myStructure.ElementGroupSetSection(groupIdMatrix, matrixSection);
        myStructure.ElementGroupSetConstitutiveLaw(groupIdMatrix, matrixMaterial);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Fibre                    **" << std::endl;
        std::cout << "***********************************" << std::endl;

        myStructure.ElementGroupSetInterpolationType(groupIdFibre, fibreInterpolationType);
        myStructure.ElementGroupSetSection(groupIdFibre, fibreSection);
        myStructure.ElementGroupSetConstitutiveLaw(groupIdFibre, fibreMaterial);

        myStructure.ElementTotalConvertToInterpolationType(1e-6, 10);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Interface                **" << std::endl;
        std::cout << "***********************************" << std::endl;

        auto pairGroupFiberGroupBond = myStructure.InterfaceElementsCreate(groupIdFibre, interfaceInterpolationType, fibreInterpolationType);

        int groupEleFiber   = pairGroupFiberGroupBond.first;
        int groupEleBond    = pairGroupFiberGroupBond.second;

        myStructure.ElementGroupSetConstitutiveLaw(groupEleFiber, fibreMaterial);
        myStructure.ElementGroupSetSection(groupEleFiber, fibreSection);

        myStructure.ElementGroupSetConstitutiveLaw(groupEleBond, interfaceMaterial);
        myStructure.ElementGroupSetSection(groupEleBond, fibreMatrixBond);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Boundary Conditions      **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int groupNodeBCLeft = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
        myStructure.GroupAddNodeCoordinateRange(groupNodeBCLeft, 0, -1e-6, +1e-6);
        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCLeft, Parameters::mDirectionX, 0);


        Eigen::Matrix<double, dimension, 1> nodeCoords;
        nodeCoords[0] = 0.0;
        nodeCoords[1] = 5.0;
        int nodeLeft = myStructure.NodeGetIdAtCoordinate(nodeCoords, 1e-6);
        myStructure.ConstraintLinearSetDisplacementNode(nodeLeft, Parameters::mDirectionY, 0);

        // 205 is the node id of the fibre
        myStructure.ConstraintLinearSetDisplacementNode(205, Parameters::mDirectionY, 0);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Loads                    **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int timeDependentConstraint = myStructure.ConstraintLinearSetDisplacementNode(205, Parameters::mDirectionX, 0);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Visualization            **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int visualizationGroup = myStructure.GroupCreate(NuTo::eGroupId::Elements);
        myStructure.GroupAddElementsTotal(visualizationGroup);

        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::DISPLACEMENTS);
        myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::CONSTITUTIVE);


        std::cout << "***********************************" << std::endl;
        std::cout << "**      Solver                   **" << std::endl;
        std::cout << "***********************************" << std::endl;

        myStructure.NodeBuildGlobalDofs();
        myStructure.CalculateMaximumIndependentSets();

        Eigen::Matrix<double, 5, 2> timeDependentLoad;
        timeDependentLoad(0, 0) = 0;
        timeDependentLoad(1, 0) = 0.25*Parameters::mSimulationTime;
        timeDependentLoad(2, 0) = 0.50*Parameters::mSimulationTime;
        timeDependentLoad(3, 0) = 0.75*Parameters::mSimulationTime;
        timeDependentLoad(4, 0) = 1.00*Parameters::mSimulationTime;

        timeDependentLoad(0, 1) = 0;
        timeDependentLoad(1, 1) = 0.25*Parameters::mLoad;
        timeDependentLoad(2, 1) = 0.50*Parameters::mLoad;
        timeDependentLoad(3, 1) = 0.25*Parameters::mLoad;
        timeDependentLoad(4, 1) = 0.00*Parameters::mLoad;

        myIntegrationScheme.AddTimeDependentConstraint(timeDependentConstraint, timeDependentLoad);


        myIntegrationScheme.Solve(Parameters::mSimulationTime);

        std::cout << "Results written to " + resultDir.string() << std::endl;

    } catch (NuTo::MechanicsException& e)
    {
        std::cout << e.ErrorMessage();
        return EXIT_FAILURE;

    } catch (NuTo::MathException& e)
    {
        std::cout << e.ErrorMessage();
        return EXIT_FAILURE;
    }

    std::cout << "***********************************" << std::endl;
    std::cout << "**      End                      **" << std::endl;
    std::cout << "***********************************" << std::endl;

}
