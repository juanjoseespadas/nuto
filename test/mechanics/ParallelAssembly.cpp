/**
 * Test file to
 *  1) check the parallel assembly
 *  2) ensure a significant (*) speedup
 *
 *  (*) ... not sure yet. First idea: speedup = 0.8 * numProc
 *
 */

#include "nuto/base/Timer.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"


void SetupStructure(NuTo::Structure& rStructure, int rNumElementsPerDimension)
{
    rStructure.SetShowTime(false);
    rStructure.SetVerboseLevel(0);
    //create nodes
    int numNodes = rNumElementsPerDimension+1;

    double deltaX = 2;
    double deltaY = 3;
    double deltaZ = 4;

    int nodeNum = 0;
    for (int iZ=0; iZ<numNodes; iZ++)
        for (int iY=0; iY<numNodes; iY++)
            for (int iX=0; iX<numNodes; iX++)
            {
                NuTo::FullVector<double,Eigen::Dynamic> coordinates(3);
                coordinates(0) = iX*deltaX;
                coordinates(1) = iY*deltaY;
                coordinates(2) = iZ*deltaZ;
                rStructure.NodeCreate(nodeNum, coordinates);
                nodeNum++;
            }

    int myInterpolationType = rStructure.InterpolationTypeCreate("Brick3D");
    rStructure.InterpolationTypeAdd(myInterpolationType, NuTo::Node::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
    rStructure.InterpolationTypeAdd(myInterpolationType, NuTo::Node::DISPLACEMENTS, NuTo::Interpolation::eTypeOrder::EQUIDISTANT2);

    for (int iZ=0; iZ<rNumElementsPerDimension; iZ++)
        for (int iY=0; iY<rNumElementsPerDimension; iY++)
            for (int iX=0; iX<rNumElementsPerDimension; iX++)
            {
                NuTo::FullVector<int,Eigen::Dynamic> nodes(8);
                nodes(0) = iX   +  iY    * numNodes +  iZ    * numNodes * numNodes;
                nodes(1) = iX+1 +  iY    * numNodes +  iZ    * numNodes * numNodes;
                nodes(2) = iX+1 + (iY+1) * numNodes +  iZ    * numNodes * numNodes;
                nodes(3) = iX   + (iY+1) * numNodes +  iZ    * numNodes * numNodes;
                nodes(4) = iX   +  iY    * numNodes + (iZ+1) * numNodes * numNodes;
                nodes(5) = iX+1 +  iY    * numNodes + (iZ+1) * numNodes * numNodes;
                nodes(6) = iX+1 + (iY+1) * numNodes + (iZ+1) * numNodes * numNodes;
                nodes(7) = iX   + (iY+1) * numNodes + (iZ+1) * numNodes * numNodes;

                rStructure.ElementCreate(myInterpolationType, nodes);

            }

    int allElements = rStructure.GroupCreate("Elements");
    rStructure.GroupAddElementFromType(allElements, myInterpolationType);

    rStructure.ElementTotalConvertToInterpolationType();

    int mySection = rStructure.SectionCreate("VOLUME");
    rStructure.ElementTotalSetSection(mySection);

    rStructure.ConstitutiveLawCreate(0, NuTo::Constitutive::LINEAR_ELASTIC_ENGINEERING_STRESS);
    rStructure.ConstitutiveLawSetParameterDouble(0,NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, 10024);
    rStructure.ConstitutiveLawSetParameterDouble(0,NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO,.12);
    rStructure.ConstitutiveLawSetParameterDouble(0,NuTo::Constitutive::eConstitutiveParameter::DENSITY, 125);
    rStructure.ElementTotalSetConstitutiveLaw(0);
}

void CompareHessiansAndInternalGradients(NuTo::Structure& rStructure1, NuTo::Structure& rStructure2)
{
    auto hessian1 = rStructure1.BuildGlobalHessian0();
    auto hessian2 = rStructure2.BuildGlobalHessian0();

    auto intGrad1 = rStructure1.BuildGlobalInternalGradient();
    auto intGrad2 = rStructure2.BuildGlobalInternalGradient();

    hessian1.AddScal(hessian2, -1);
    intGrad1 -= intGrad2;

    if (hessian1.JJ(NuTo::Node::DISPLACEMENTS, NuTo::Node::DISPLACEMENTS).AbsMax() > 1.e-10)
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "HESSIANS incorrect.");


    if (intGrad1.J[NuTo::Node::DISPLACEMENTS].Norm() > 1.e-10)
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, "INTERNAL GRADIENT incorrect.");
}

double MeasureHessianAndInternalGradient(NuTo::Structure& rStructure)
{
    NuTo::Timer timer("");

    auto hessian = rStructure.BuildGlobalHessian0();
    auto intGrad = rStructure.BuildGlobalInternalGradient();

    return timer.GetTimeDifference();
}


int main()
{
    constexpr int numProc = 4;
    constexpr double requiredSpeedup = 0.8 * numProc;

    NuTo::Timer timer("Setup structures");

    NuTo::Structure smallStructureSerial(3);
    NuTo::Structure smallStructureParallel(3);

    NuTo::Structure bigStructureSerial(3);
    NuTo::Structure bigStructureParallel(3);

    smallStructureSerial.SetNumProcessors(1);
    smallStructureParallel.SetNumProcessors(numProc);

    bigStructureSerial.SetNumProcessors(1);
    bigStructureParallel.SetNumProcessors(numProc);


    SetupStructure(smallStructureSerial,    2);
    SetupStructure(smallStructureParallel,  2);

    SetupStructure(bigStructureSerial,     15);
    SetupStructure(bigStructureParallel,   15);

    smallStructureSerial.CalculateMaximumIndependentSets();
    smallStructureParallel.CalculateMaximumIndependentSets();
    bigStructureSerial.CalculateMaximumIndependentSets();
    bigStructureParallel.CalculateMaximumIndependentSets();


    timer.Reset("Correctnes test");
    try
    {
        CompareHessiansAndInternalGradients(smallStructureSerial, smallStructureParallel);
    }
    catch (NuTo::Exception& e)
    {
        std::cout << e.ErrorMessage() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "Undefined exception..." << std::endl;
        return EXIT_FAILURE;
    }



    timer.Reset("Speedup test");
    double timeSerial = MeasureHessianAndInternalGradient(bigStructureSerial);
    std::cout << "Serial:   " << timeSerial << std::endl;

    double timeParallel = MeasureHessianAndInternalGradient(bigStructureParallel);
    std::cout << "Parallel: " << timeParallel << std::endl;

    double speedup = timeSerial / timeParallel;
    std::cout << "Speedup:  " << speedup << std::endl;
    std::cout << "required: " << requiredSpeedup << std::endl;

    if (speedup < requiredSpeedup)
    {
        std::cout << "Parallel assembly speedup smaller than " << requiredSpeedup << ". Something is wrong.\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}