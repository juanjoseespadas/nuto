// $Id$

//! @author Andrea Keßler, ISM
//! @date December 2011
//! @brief ... example for conjugate gradient grid routine with reduced vector size to existing dofs

#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/elements/ElementDataEnum.h"
#include "nuto/mechanics/elements/ElementEnum.h"
#include "nuto/mechanics/elements/IpDataEnum.h"

#ifdef ENABLE_SERIALIZATION
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#else
#include <boost/ptr_container/ptr_vector.hpp>
#endif //ENABLE_SERIALIZATION

#ifdef SHOW_TIME
    #include <ctime>
#endif

#include <iostream>
#include "nuto/base/NuToObject.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/mechanics/structures/grid/StructureGrid.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/optimize/CallbackHandlerGrid.h"
#include "nuto/optimize/ConjugateGradientGridRed.h"
#include "nuto/mechanics/elements/Brick8N.h"


int main()
{
	bool matrixFreeMethod=0; //0 -EBE, 1- NBN, false=0

//	bool matrixFreeMethod=1; //0 -EBE, 1- NBN
	std::fstream outputTime;
	std::string filename = "timeOutput";
    outputTime.open(filename,std::fstream::out|std::fstream::app);
	std::cout<<"[NuTo::Grid3D] matrixFreeMethod is";
	if (matrixFreeMethod)
	{
		std::cout<<" NBN \n";
		outputTime<<" NBN  ";
	}
	else
	{
		std::cout<<" EBE \n";
		outputTime<<" EBE  ";
	}
	//double microTomm =0.001;
	double microTomm =1.;

	//   int readFlag = false;
    double PoissonsRatio = 0.2;
    //for local base stiffness matrix
    double YoungsModulus = 1.;

    size_t rNumVoxel;
    double rVoxelSpacing[3];
    double rGridOrigin[3];
    size_t rGridDimension[3];

    // create structure
#ifdef SHOW_TIME
    std::clock_t start,end;
    start=clock();
#endif
	// read entries
	NuTo::StructureGrid myGrid(3);
	myGrid.SetVerboseLevel(0);
	// input files have to have a one voxel thick frame without material
	// dimensions are inclusive the frame
	myGrid.ImportFromVtkASCIIFileHeader("InputTest",rGridDimension,rVoxelSpacing,rGridOrigin,rNumVoxel);
//	std::cout<<__FILE__<<"  variab: spac: "<<rVoxelSpacing[0]<< " gridDim: "<<rGridDimension[0]<<std::endl;
	assert (rNumVoxel==rGridDimension[0]*rGridDimension[1]*rGridDimension[2]);
	std::cout<<"[NuTo::Grid3D] numVoxel "<<rNumVoxel<<std::endl;
	outputTime<<rNumVoxel<<"   ";

	std::vector<int> imageValues (rNumVoxel);
	myGrid.ImportFromVtkASCIIFile( "InputTest",imageValues);

//	std::cout<<"\n[Grid3D] : imageValues ";
//	for(int i=0;i<(int)rNumVoxel;++i)
//		std::cout<<imageValues[i]<<" ";
//	std::cout<<"\n";

	size_t numGridNodes=(rGridDimension[0]+1)*(rGridDimension[1]+1)*(rGridDimension[2]+1);//all nodes of the grid
    size_t numBytesEBE=0;

 	//RB
	double Force = -1.;
	bool EnableDisplacementControl = true;
//	bool EnableDisplacementControl = false;
	double BoundaryDisplacement = -1.0;
//	double BoundaryDisplacement = -(rGridDimension[2]*rVoxelSpacing[2]*microTomm)/20.0;
	std::cout<<"[NuTo::Grid3D]  Boundary Displacement: "<<BoundaryDisplacement<<"\n";
	//calculate one element stiffness matrix with E=1
	NuTo::Structure myHelpStruc(3);

	myHelpStruc.SetVerboseLevel(0);
	// create material law
	int myMat=myHelpStruc.ConstitutiveLawCreate("LinearElastic");
	myHelpStruc.ConstitutiveLawSetPoissonsRatio(myMat, PoissonsRatio);
	myHelpStruc.ConstitutiveLawSetYoungsModulus(myMat, YoungsModulus);

	// create nodes
	NuTo::FullMatrix<double> nodeCoordinates(3, 1);
	NuTo::FullMatrix<int> elementIncidence(8,1);

	nodeCoordinates(0, 0) = -rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = -rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = -rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(0,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = -rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = -rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(1,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = -rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(2,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = -rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = -rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(3,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = -rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = -rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(4,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = -rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(5,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(6,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	nodeCoordinates(0, 0) = -rVoxelSpacing[0] * microTomm * 0.5;
	nodeCoordinates(1, 0) = rVoxelSpacing[1] * microTomm * 0.5;
	nodeCoordinates(2, 0) = rVoxelSpacing[2] * microTomm * 0.5;
	elementIncidence(7,0)=myHelpStruc.NodeCreate("displacements", nodeCoordinates);

	// first element create

   // elementIncidence.Info();
	int myHelpElement=myHelpStruc.ElementCreate("Brick8N", elementIncidence);
	myHelpStruc.ElementSetConstitutiveLaw(myHelpElement,myMat);

	//myHelpStruc.NodeInfo(0);
//	myHelpStruc.NodeBuildGlobalDofs();

	// build global stiffness matrix and equivalent load vector which correspond to prescribed boundary values
	NuTo::FullMatrix<double> stiffnessMatrix;
	NuTo::FullMatrix<int> rows;
	NuTo::FullMatrix<int> coluums;
	myHelpStruc.ElementStiffness(0,stiffnessMatrix,rows,coluums );
//	std::cout<<"Element Stiffness created"<<std::endl;
//	stiffnessMatrix.Info(9,5);
	stiffnessMatrix.WriteToFile("stiffnessMatrix.txt"," ");
	std::vector<double> baseStiffness(24*24);
	for (int i=0;i<24;++i)
	{
		for (int j=0;j<24;++j)
		{
			baseStiffness[(24*i)+j]=stiffnessMatrix(i,j); //row based saved
		}
	}
	// new without Youngs Moduluds - general edge Stiffness
	// edgeStiffness 3x3 ************************
	std::vector<double> edgeStiffness(0);
	if (matrixFreeMethod)
	{
//		edgeStiffness.resize(9*64*27);
		edgeStiffness.resize(9*64);

		std::cout<<" set edge stiffness\n";
		// init edgeStiffness in order,
		// description: node nbr, elem nbr, row nbr (nodes), col nbr (nodes) of base
		int count=0;
		for(int row=0;row<3;++row)
		{
			for(int col=0;col<3;++col)
			{
				count=-1;
				// 0 - 0 - 6  0
				edgeStiffness[(++count)*9+(3*row+col)]=baseStiffness[(6*3+row)*24+0*3+col];
				// 1 - 0 - 6  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+1*3+col];
				// 1 - 1 - 7  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+0*3+col];
				// 2 - 1 - 7  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+1*3+col];
				// 3 - 0 - 6  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+3*3+col];
				// 3 - 2 - 5  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+0*3+col];
				// 4 - 0 - 6  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+2*3+col];
				// 4 - 1 - 7  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+3*3+col];
				// 4 - 2 - 5  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+1*3+col];
				// 4 - 3 - 4  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+0*3+col];
				// 5 - 1 - 7  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+2*3+col];
				// 5 - 3 - 4  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+1*3+col];
				// 6 - 2 - 5  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+3*3+col];
				// 7 - 2 - 5  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+2*3+col];
				// 7 - 3 - 4  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+3*3+col];
				// 8 - 3 - 4  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+2*3+col];
				// 9 - 0 - 6  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+4*3+col];
				// 9 - 4 - 2  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+0*3+col];
				//10- 0 - 6  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+5*3+col];
				//10 - 1 - 7  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+4*3+col];
				//10 - 4 - 2  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+1*3+col];
				//10 - 5 - 3  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+0*3+col];
				//11 - 1 - 7  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+5*3+col];
				//11 - 5 - 3  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+1*3+col];
				//12 - 0 - 6  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+7*3+col];
				//12 - 2 - 5  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+4*3+col];
				//12 - 4 - 2  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+3*3+col];
				//12 - 6 - 1  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+0*3+col];
				//13 - 0 - 6  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(6*3+row)*24+6*3+col];
				//13 - 1 - 7  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+7*3+col];
				//13 - 2 - 5  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+5*3+col];
				//13 - 3 - 4  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+4*3+col];
				//13 - 4 - 2  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+2*3+col];
				//13 - 5 - 3  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+3*3+col];
				//13 - 6 - 1  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+1*3+col];
				//13 - 7 - 0  0
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+0*3+col];
				//14 - 1 - 7  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(7*3+row)*24+6*3+col];
				//14 - 3 - 4  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+5*3+col];
				//14 - 5 - 3  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+2*3+col];
				//14 - 7 - 0  1
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+1*3+col];
				//15 - 2 - 5  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+7*3+col];
				//15 - 6 - 1  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+3*3+col];
				//16 - 2 - 5  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(5*3+row)*24+6*3+col];
				//16 - 3 - 4  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+7*3+col];
				//16 - 6 - 1  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+2*3+col];
				//16 - 7 - 0  3
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+3*3+col];
				//17 - 3 - 4  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(4*3+row)*24+6*3+col];
				//17 - 7 - 0  2
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+2*3+col];
				//18 - 4 - 2  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+4*3+col];
				//19 - 4 - 2  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+5*3+col];
				//19 - 5 - 3  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+4*3+col];
				//20 - 5 - 3  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+5*3+col];
				//21 - 4 - 2  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+7*3+col];
				//21 - 6 - 1  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+4*3+col];
				//22 - 4 - 2  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(2*3+row)*24+6*3+col];
				//22 - 5 - 3  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+7*3+col];
				//22 - 6 - 1  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+5*3+col];
				//22 - 7 - 0  4
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+4*3+col];
				//23 - 5 - 3  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(3*3+row)*24+6*3+col];
				//23 - 7 - 0  5
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+5*3+col];
				//24 - 6 - 1  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+7*3+col];
				//25 - 6 - 1  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(1*3+row)*24+6*3+col];
				//25 - 7 - 0  7
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+7*3+col];
				//26 - 7 - 0  6
				edgeStiffness[++count*9+(3*row+col)]=baseStiffness[(0*3+row)*24+6*3+col];

			}
		}
//			std::cout<<"\n edgeStiff ";
//			for (int i=0;i<9*64;++i)
//				std::cout<<i<<": "<<edgeStiffness[i]<<" ";
//			std::cout<<"\n";
	}

//	stiffnessMatrix=NuTo::FullMatrix<double>(24,24,baseStiffness);
//	stiffnessMatrix.WriteToFile("baseStiffness.txt"," ");

//	const double rLocalCoordinates[3]={0,0,0};
//	std::vector<double> rDerivativeShapeFunctions(24);
//	NuTo::ElementBase* myElem=myHelpStruc.ElementGetElementPtr(0);
//	NuTo::Brick8N* myBrick=static_cast<NuTo::Brick8N*> (myElem);
//	myBrick->CalculateDerivativeShapeFunctionsLocal(&rLocalCoordinates[0],rDerivativeShapeFunctions);
//	NuTo::FullMatrix<double> derShapeMat(rDerivativeShapeFunctions);
//	derShapeMat.WriteToFile("derivativeShape.txt"," ");


	//grid structure create
	/**************************************************************************/
	//material values are smaller than threshold value
	//color values form 0 255
	//thresholdMaterialValues: 180, 188 ->last value with material
	int thresholdMaterialValue=188; //last value for "air", voxel which does not get element
	//set Modul for each color
	std::vector<double> myMapColorModul(256);
	for(int count=0;count<thresholdMaterialValue;count++)
	{
//		myMapColorModul[count]=1.;
		myMapColorModul[count]=100000.;
	}
	for(int count=thresholdMaterialValue;count<255;count++)
		myMapColorModul[count]=0.;

	size_t numElems=0;
	size_t numNodes=0;
	boost::dynamic_bitset<> nodeExist(numGridNodes); //0 = false, all 0 here
	std::vector<size_t>  edgeId(numNodes);		//saves the id of the edge of this node
	std::vector<size_t>  nodeId(numGridNodes);		//saves the id of the edges of this node
	std::vector<size_t> voxelId(numElems);		//saves the id of the voxel of this element
	std::vector<double> youngsModulus(0);
	std::vector<int> materialOfElem(0);

	// put get routines for a while here
	// get voxel locations of voxels
	std::vector<int> allEdgesAtVoxel(rNumVoxel*8);
	for (size_t element=0;element<rNumVoxel;++element)
	{
			size_t numDimxy=element/((rGridDimension[0])*(rGridDimension[1]));
			size_t numDimx=0;
			size_t residual1=element%((rGridDimension[0])*(rGridDimension[1]));
			size_t residual2=0;
			numDimx=residual1/(rGridDimension[0]);
			residual2=residual1%(rGridDimension[0]);
			size_t rVoxelLocation[3];
			rVoxelLocation[0]=residual2;
			rVoxelLocation[1]=numDimx;
			rVoxelLocation[2]=numDimxy;
			allEdgesAtVoxel[8*element+0] = (int) rVoxelLocation[2]*(rGridDimension[0]+1)*(rGridDimension[1]+1) + rVoxelLocation[1]     * (rGridDimension[1]+1) + rVoxelLocation[0];
			allEdgesAtVoxel[8*element+1] =  (int)rVoxelLocation[2]*(rGridDimension[0]+1)*(rGridDimension[1]+1) + rVoxelLocation[1]     * (rGridDimension[1]+1) + rVoxelLocation[0]+1;
			allEdgesAtVoxel[8*element+2] = (int) rVoxelLocation[2]*(rGridDimension[0]+1)*(rGridDimension[1]+1) + (rVoxelLocation[1]+1) * (rGridDimension[1]+1) + rVoxelLocation[0] +1;
			allEdgesAtVoxel[8*element+3] = (int) rVoxelLocation[2]*(rGridDimension[0]+1)*(rGridDimension[1]+1) + (rVoxelLocation[1]+1) * (rGridDimension[1]+1) + rVoxelLocation[0];

			allEdgesAtVoxel[8*element+4] = (int) (rVoxelLocation[2]+1)*(rGridDimension[0]+1)*(rGridDimension[1]+1) + rVoxelLocation[1]     * (rGridDimension[1]+1) + rVoxelLocation[0];
			allEdgesAtVoxel[8*element+5] =  (int)(rVoxelLocation[2]+1)*(rGridDimension[0]+1)*(rGridDimension[1]+1) + rVoxelLocation[1]     * (rGridDimension[1]+1) + rVoxelLocation[0]+1;
			allEdgesAtVoxel[8*element+6] = (int) (rVoxelLocation[2]+1)*(rGridDimension[0]+1)*(rGridDimension[1]+1) + (rVoxelLocation[1]+1) * (rGridDimension[1]+1) + rVoxelLocation[0]+1;
			allEdgesAtVoxel[8*element+7] =  (int)(rVoxelLocation[2]+1)*(rGridDimension[0]+1)*(rGridDimension[1]+1) + (rVoxelLocation[1]+1) * (rGridDimension[1]+1) + rVoxelLocation[0];

		//	std::cout<<" nodesAtVoxel "<<element<<": "<<allEdgesAtVoxel[8*element+0]<<" "<<allEdgesAtVoxel[8*element+1]<<" "<<allEdgesAtVoxel[8*element+2]<<" "<<allEdgesAtVoxel[8*element+3]<<" "<<allEdgesAtVoxel[8*element+4]<<" "<<allEdgesAtVoxel[8*element+5]<<" "<<allEdgesAtVoxel[8*element+6]<<" "<<allEdgesAtVoxel[8*element+7]<<"\n";
			//also for global vector
			//allVoxelLocation[element]=new int[3];
			//allVoxelLocation[element]=rVoxelLocation;
	}

	int numCoeffMat=1;		// set material 0 with E=0
	youngsModulus.push_back(0.);

	bool matExistsAlready=false;
	for (size_t countVoxels=0;countVoxels<rNumVoxel;++countVoxels)
	{
		if (imageValues[countVoxels]<thresholdMaterialValue)
		{
			voxelId.push_back(countVoxels);
//			std::cout<<" voxel "<<countVoxels<<" elem "<<numElems;
			++numElems;
			for (int node=0;node<8;++node)
			{
				nodeExist.set(allEdgesAtVoxel[8*countVoxels+node],true);
//				std::cout<<" nodeExist "<<allEdgesAtVoxel[8*countVoxels+node];
			}

			// nodes are no more set here but not yet elsewhere
			for(int countMat=1;countMat<numCoeffMat;countMat++)
			{
				if (myMapColorModul[imageValues[countVoxels]]==youngsModulus.at(countMat)) //same modulus already used
				{
					materialOfElem.push_back(countMat);
					countMat=numCoeffMat+1;
					matExistsAlready=true;
				}
				else
					matExistsAlready=false;
			}
				if (!matExistsAlready)
				{
					materialOfElem.push_back(numCoeffMat);
					//set youngsModulus and add on material on counter
					youngsModulus.push_back(myMapColorModul[imageValues[countVoxels]]);
					numCoeffMat++;
					matExistsAlready=true; //matrix already added
				}
  		}
	}
    size_t numDofs=0;
 	for (size_t node=0;node<numGridNodes;++node)
	{
		if (nodeExist[node])
		{
			edgeId.push_back(node);
			nodeId[node]=numNodes++;
		}
		else
		{
			nodeId[node]=numGridNodes;
		}
	}
 	for (size_t node=0;node<numGridNodes;++node)
 	{
		if (nodeId[node]==numGridNodes)
		{
			nodeId[node]=numNodes;
		}

 	}
//	std::cout<<"\n[Grid3D] : voxelId ";
//	for(size_t i=0;i<numElems;++i)
//		std::cout<<voxelId[i]<<" ";
//	std::cout<<"\n";
//
//	std::cout<<"[Grid3D] : nodeId ";
//	for(size_t i=0;i<numGridNodes;++i)
//		std::cout<<nodeId[i]<<" ";
//	std::cout<<"\n";
//
//	std::cout<<"[Grid3D] : edgeId ";
//	for(size_t i=0;i<numNodes;++i)
//		std::cout<<edgeId[i]<<" ";
//	std::cout<<"\n";

 	numDofs=3*numNodes;

	std::vector<int> allNodesAtElem(8*numElems);
	int voxel=0;
	for (size_t element=0;element<numElems;++element)
	{
		voxel=voxelId[element];
		allNodesAtElem[8*element+0]=	nodeId[allEdgesAtVoxel[8*voxel+0]];
		allNodesAtElem[8*element+1]=	nodeId[allEdgesAtVoxel[8*voxel+1]];
		allNodesAtElem[8*element+2]=	nodeId[allEdgesAtVoxel[8*voxel+2]];
		allNodesAtElem[8*element+3]=	nodeId[allEdgesAtVoxel[8*voxel+3]];
		allNodesAtElem[8*element+4]=	nodeId[allEdgesAtVoxel[8*voxel+4]];
		allNodesAtElem[8*element+5]=	nodeId[allEdgesAtVoxel[8*voxel+5]];
		allNodesAtElem[8*element+6]=	nodeId[allEdgesAtVoxel[8*voxel+6]];
		allNodesAtElem[8*element+7]=	nodeId[allEdgesAtVoxel[8*voxel+7]];
}
	allEdgesAtVoxel.clear();


	std::cout<<"[NuTo::Grid3D] numElems = "<<numElems<<"\n";
	std::cout<<"[NuTo::Grid3D] numNodes = "<<numNodes<<"\n";
	std::cout<<"[NuTo::Grid3D] numDofs = "<<numDofs<<"\n";

	std::cout<<"[NuTo::Grid3D] Young's Modulus 0= "<<youngsModulus[0]<<"\n";
	std::cout<<"[NuTo::Grid3D] Young's Modulus 1= "<<youngsModulus[1]<<"\n";

//	std::cout<<"[NuTo::Grid3D] Young's Modulus = "<<youngsModulus[0]<<"\n";
//	std::cout<<" nodeExist reverse "<<nodeExist<<"\n";
//	std::cout<<" elemExist "<<elemExist<<"\n";

	//NBN**************************************************************************
//	int orderNodeForElem[8][8]={
//		{0,1,4,3,9,10,13,12},
//		{1,2,5,4,10,11,14,13},
//		{4,5,8,7,13,14,17,16},
//		{3,4,7,6,12,13,16,15},
//		{9,10,13,12,18,19,22,21},
//		{10,11,14,13,19,20,23,22},
//		{13,14,17,16,22,23,26,25},
//		{12,13,16,15,21,22,25,24},
//	};
	//field of 27 nodes which contain element and node of element
	std::vector<int> allNodesAtNode(27);
	if(matrixFreeMethod)
	{
//		allNodesAtNode.resize(27*numGridNodes);

		int neighborNodes[27];
		neighborNodes[0]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1)-1;
		neighborNodes[1]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1);
		neighborNodes[2]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1)+1;
		neighborNodes[3]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)-1;
		neighborNodes[4]=-(rGridDimension[0]+1)*(rGridDimension[1]+1);
		neighborNodes[5]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)+1;
		neighborNodes[6]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1)-1;
		neighborNodes[7]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1);
		neighborNodes[8]=-(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1)+1;
		neighborNodes[9]=-(rGridDimension[0]+1)-1;
		neighborNodes[10]=-(rGridDimension[0]+1);
		neighborNodes[11]=-(rGridDimension[0]+1)+1;
		neighborNodes[12]=-1;
		neighborNodes[13]=0;
		neighborNodes[14]=+1;
		neighborNodes[15]=+(rGridDimension[0]+1)-1;
		neighborNodes[16]=+(rGridDimension[0]+1);
		neighborNodes[17]=+(rGridDimension[0]+1)+1;
		neighborNodes[18]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1)-1;
		neighborNodes[19]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1);
		neighborNodes[20]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)-(rGridDimension[0]+1)+1;
		neighborNodes[21]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)-1;
		neighborNodes[22]=+(rGridDimension[0]+1)*(rGridDimension[1]+1);
		neighborNodes[23]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)+1;
		neighborNodes[24]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1)-1;
		neighborNodes[25]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1);
		neighborNodes[26]=+(rGridDimension[0]+1)*(rGridDimension[1]+1)+(rGridDimension[0]+1)+1;

//		std::cout<<"[Grid3D] : neighborNodes \n";
//		for(int i=0;i<27;++i)
//			std::cout<<neighborNodes[i]<<" ";
//		std::cout<<"\n";

//#ifdef NEIGHBORS
//		allNodesAtNode.resize(27*numGridNodes);

//		for(int node=0;node<numGridNodes;++node)
//		{
//			for(int i=0;i<27;++i)
//				allNodesAtNode[27*node+i]=node+neighborNodes[i];
//		}

//#else //NEIGHBORS
//	allNodesAtNode.resize(27);
	for(int i=0;i<27;++i)
		allNodesAtNode[i]=neighborNodes[i];
//#endif
	}

//	std::cout<<"[Grid3D] : neighborNodes \n";
//	for(int i=0;i<27;++i)
//		std::cout<<allNodesAtNode[i]<<"  ";
//	std::cout<<"\n";

	std::vector<int> matOfEdge(0);
	if(matrixFreeMethod)
	{
		matOfEdge.resize(64*numNodes,0);
		int orderEdgesForElem[8][8]={
			{35,39,47,45,57,59,63,62},// 7.
			{27,34,44,41,53,56,61,60},// 6.
			{17,20,32,26,48,49,54,52},// 4.
			{21,23,38,33,50,51,58,55},//5.
			{9,11,15,14,31,37,46,43},//3.
			{5,8,13,12,25,30,42,40}, //2.
			{0,1,6,4,16,18,28,24},//0. elem
			{2,3,10,7,19,22,36,29},//1. elem
		};
		std::vector<int> numElemPerNeigh={1,2,1,2,4,2,1,2,1,2,4,2,4,8,4,2,4,2,1,2,1,2,4,2,1,2,1};
		std::vector<int> edgesNumOfNeigh={0,1,3,4,6,10,12,13,15,17,21,23,27,35,39,41,45,47,48,50,51,53,57,59,60,62,63};

		for(size_t element=0;element<numElems;++element)
		{
//			std::cout<<" mat "<<materialOfElem[element];
			// over local 8 nodes to get correct neighbor element
			for(int locNode=0;locNode<8;++locNode)
			{
//				std::cout<<" node "<allNodesAtElem[8*element+locNode]<< " edge at ";
				// save edge matrix at all edges of element
				for(int node=0;node<8;++node)
				{
					matOfEdge.at(64*allNodesAtElem[8*element+locNode]+orderEdgesForElem[locNode][node])=materialOfElem[element];
//					std::cout<<orderEdgesForElem[locNode][node]<<" ";

				}
//				std::cout<<" "<<std::endl;
//				std::cout<<" matOfEdge: node"<<allNodesAtElem[8*element+locNode]<< " - ";
//				for (int i=0;i<64;++i)
//				{
//					if(matOfEdge[64*locNode+i]==1)
//					std::cout<< 64*locNode+i<<" ";
//				}
//				std::cout<<" "<<std::endl;
			}
		}
	}


	//********************************************************************************

	boost::dynamic_bitset<> rDofIsConstraint(3*numNodes); //0 = false, all 0 here

	std::vector<double> displVector(3*(numNodes+1),0.0);// initialized with zero

	//myMapColorModul.WriteToFile("$HOME/develop/nuto/MapColorModul.txt"," ");

//---------------------------------------------------------------------------------------//
// boundary conditions
//---------------------------------------------------------------------------------------//
	int numConstraintDofs=0;
// --------------------------------------------------------------------------------//
	//all nodes with x=0
	// set x,y,z direction constraint , zero
//	for (int count = 0;count<numGridNodes;count+= (rGridDimension[0] + 1))
//	{
//		//std::cout<<__FILE__<<" "<<__LINE__<<" node constraint x"<< count <<std::endl;
//		rDofIsConstraint.set(count*3+0,true);
//		rDofIsConstraint.set(count*3+1,true);
//		rDofIsConstraint.set(count*3+2,true);
//	}

	// all nodes with y=0 and x=0
	// set y direction zero
//	for (int count =0;count < numGridNodes;count+= (rGridDimension[0] + 1)*(rGridDimension[1] + 1))
//	{
//		//std::cout<<__FILE__<<" "<<__LINE__<<" node constraint y "<< count <<std::endl;
//		rDofIsConstraint.set(count*3+1,true);
//	}

	// all nodes with z=0 and x=0
	// set z direction zero
//	for (int count = 0;count<(rGridDimension[0] + 1)*(rGridDimension[1] + 1);count+=(rGridDimension[0] + 1))
//	{
//		//std::cout<<__FILE__<<" "<<__LINE__<<" node constraint z "<< count <<std::endl;
//		rDofIsConstraint.set(count*3+2,true);
//	}

	//----------------------------------------------------------------------------------------//
	// Boundary condition,all nodes with x=0 and z=0
	//----------------------------------------------------------------------------------------//
	// set x direction zero
//	for (int count = 0;count<(rGridDimension[0] + 1)*(rGridDimension[1] + 1);count+=(rGridDimension[0] + 1))
//	{
//		rDofIsConstraint.set(count*3,true);
//		++numConstraintDofs;
//	}
	//----------------------------------------------------------------------------------------//
	// Boundary condition,all nodes with x=0
	// Boundary condition,node 0 uy=uz=0 and node = 1*dimx*dimy uz=0
	//----------------------------------------------------------------------------------------//
//	for (int count = 0;count<numGridNodes;count+=(rGridDimension[0] + 1))
//	{
//		rDofIsConstraint.set(count*3,true);
//		++numConstraintDofs;
//	}

//	rDofIsConstraint.set(1,true);
//	++numConstraintDofs;
//	rDofIsConstraint.set(2,true);
//	++numConstraintDofs;
//	rDofIsConstraint.set(3*((rGridDimension[0]+1)*(rGridDimension[1]+1)-1)+2,true);
//	++numConstraintDofs;

	//----------------------------------------------------------------------------------------//

	//----------------------------------------------------------------------------------------//
	// Boundary condition: all nodes with y=0 and z=0
	//Boundary condition:  set y direction zero
////	for (int count = 0;count<(rGridDimension[0] + 1);++count)
//	// Boundary condition: all nodes with y=0
//	std::cout<<"[NuTo::Grid3D] Boundary conditions:  \n";
//	for(int row=0;row<(rGridDimension[2] + 1);++row)
//	{
//		for (int count = 0;count<(rGridDimension[0] + 1);++count)
//		{
//			rDofIsConstraint.set(3*row*(rGridDimension[0] + 1)*(rGridDimension[1] + 1)+count*3+1,true);
//			++numConstraintDofs;
//		}
//
//	}

	//----------------------------------------------------------------------------------------//
	// Boundary condition: all nodes with z=0
	// Boundary condition: set x,y,z direction zero
	//----------------------------------------------------------------------------------------//
	std::cout<<"[NuTo::Grid3D] Boundary conditions: all dofs constraint for z=0 \n";
	for (size_t count = (rGridDimension[0] + 1)*(rGridDimension[1] + 1);count<2*(rGridDimension[0] + 1)*(rGridDimension[1] + 1);++count)
	{
		if(nodeExist[count])
		{
			rDofIsConstraint.set(nodeId[count]*3+0,true);
			rDofIsConstraint.set(nodeId[count]*3+1,true);
			rDofIsConstraint.set(nodeId[count]*3+2,true);
	//		++numConstraintDofs;
			numConstraintDofs+=3;

		}
	}
	if(numConstraintDofs==0)
		std::cout<<"[NuTo::Grid3D] No boundary conditions set (z=0).  \n";

	//----------------------------------------------------------------------------------------//

	// apply nodes
	std::vector<double>extForces(0);
	if(EnableDisplacementControl)
	{
//		std::cout << "Displacement control" << std::endl;
		// boundary displacments

		//----------------------------------------------------------------------------------------//
		// Boundary condition: all nodes with z=max, uz=BoundaryDisplacement
		std::cout<<"[NuTo::Grid3D] Boundary conditions: Displacement at z=max in z-direction \n";
		size_t help=numConstraintDofs;
		for (size_t count = (rGridDimension[0] + 1)*(rGridDimension[1] + 1)*(rGridDimension[2]-1);count<(rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2];++count)
		{
			if(nodeExist[count])
			{
				rDofIsConstraint.set(nodeId[count]*3+2,true);
				displVector[nodeId[count]*3+2]=BoundaryDisplacement;
				++numConstraintDofs;
			}
		}
		if(numConstraintDofs-help==0)
			std::cout<<"[NuTo::Grid3D] No boundary conditions set (z=max).  \n";
		//----------------------------------------------------------------------------------------//

		//----------------------------------------------------------------------------------------//
		// Boundary condition: all nodes with x=max, ux=BoundaryDisplacement
		//----------------------------------------------------------------------------------------//
//		for (size_t count = rGridDimension[0];count<numGridNodes;count+=(rGridDimension[0] + 1))
//		{
//			rDofIsConstraint.set(count*3,true);
//			displVector[count*3]=BoundaryDisplacement;
//			++numConstraintDofs;
//		}
		//----------------------------------------------------------------------------------------//
	}
	else
	{
		std::cout << "[NuTo::Grid3D] Load control" <<std::endl;
		extForces.resize(3*numNodes,0.);
		// Boundary condition: fz=-1
//		for (size_t count = (rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2];count<numGridNodes;++count)
//		{
//
// 			if(nodeExist[count])
//			{
// 				extForces[count*3+2]=Force;
//			}
//		}
		// for 8 element example, unit force 1
		size_t count = (rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2];
//		std::cout <<__FILE__<<" "<<__LINE__<< "first node" <<count<<std::endl;
		extForces[count*3+2]=Force/4;
		++count;
		extForces[count*3+2]=Force/2;
		++count;
		extForces[count*3+2]=Force/4;
		++count;
		extForces[count*3+2]=Force/2;
		++count;
		extForces[count*3+2]=Force;
		++count;
		extForces[count*3+2]=Force/2;
		++count;
		extForces[count*3+2]=Force/4;
		++count;
		extForces[count*3+2]=Force/2;
		++count;
		extForces[count*3+2]=Force/4;

		std::cout<<"  extForces ";
		for (size_t i=0;i<3*numNodes;++i)
				std::cout<< extForces[i] <<" ";
			std::cout<<"\n";

	///////////////////////////////////////////////////////////

//		for (size_t xcount=0;xcount< rGridDimension[0];++xcount)
//		{
//			double nodeForce;
//				if(xCount == 0 || xCount == rGridDimension[0])
//				{
//					nodeForce = Force / (4 *rGridDimension[1] * rGridDimension[2]);
//				}
//				else
//				{
//					nodeForce = Force / (2 *rGridDimension[1] * rGridDimension[2]);
//				}
//
//		}
// 		for (size_t count = (rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2];count<numGridNodes;++count)
//		{
//
// 			if(nodeExist[count])
//			{
//				if (count==0 || count==(rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2]+rGridDimension[0]
//						||count==(rGridDimension[0] + 1)*(rGridDimension[1] + 1)*rGridDimension[2]+(rGridDimension[0] + 1)*(rGridDimension[1]) || count==(rGridDimension[0] + 1)*(rGridDimension[1] + 1)*(rGridDimension[2]+1)-1 )
//					nodeForce = Force / (4 *rGridDimension[1] * rGridDimension[0]);
//				else if ()
//
//
//				rDofIsConstraint.set(count*3+2,true);
//				displVector[count*3+2]=BoundaryDisplacement;
//				++numConstraintDofs;
//			}
//		}
//
	}
//	std::cout<<"  constraint z ";
//	for (size_t i=0;i<rDofIsConstraint.size();++i)
//			std::cout<< rDofIsConstraint[i] <<" ";
//		std::cout<<"\n";

	outputTime<<numDofs<<"   ";
#ifdef SHOW_TIME
end=clock();
std::cout<<"[NuTo::Grid3D] structure set " << difftime(end,start)/CLOCKS_PER_SEC << "sec \n";
	outputTime<<difftime(end,start)/CLOCKS_PER_SEC<<"   ";
#endif

	std::cout<<"[NuTo::Grid3D] number of dofs "<<numDofs<<" free: "<<numDofs-numConstraintDofs<<" constraint: "<<numConstraintDofs<<"\n";
	// start analysis
//	std::cout<<__FILE__<<" "<<__LINE__<<"  start analysis"<<std::endl;

	NuTo::ConjugateGradientGridRed myOptimizer(numNodes*3);

	myOptimizer.SetVerboseLevel(0);

	myOptimizer.SetParameters(displVector);
	std::cout<<"[NuTo::Grid3D] Parameters set, Anzahl = "<<numDofs<<std::endl;
//	std::cout<<"  voxelId ";
//	for (size_t i=0;i<numElems;++i)
//			std::cout<< voxelId[i] <<" ";
//		std::cout<<"\n";
//
//		if (matrixFreeMethod) //NBN
//	{
//		voxelId.resize(0);
//	}
//	std::cout<<"  nodeId ";
//	for (size_t i=0;i<numGridNodes;++i)
//			std::cout<< nodeId[i] <<" ";
//		std::cout<<"\n";
//
//	std::cout<<"  edgeId ";
//	for (size_t i=0;i<numNodes;++i)
//			std::cout<< edgeId[i] <<" ";
//		std::cout<<"\n";

	//
	// overhead std constructors
	// with saving neighbors,
	numBytesEBE=7*sizeof(std::vector<double>)+sizeof(rDofIsConstraint);
	if(matrixFreeMethod)
	{
		//NBN
		numBytesEBE+=sizeof(numNodes)+sizeof(rGridDimension)+sizeof(matrixFreeMethod)
					+sizeof(double)*(
						rDofIsConstraint.num_blocks()+youngsModulus.size()
						+edgeStiffness.size()+displVector.size()+extForces.size())
				+sizeof(size_t)*(
						edgeId.size()+nodeId.size()+matOfEdge.size()
						+allNodesAtNode.size());
	}
	else
	{
		numBytesEBE+=sizeof(numNodes)+sizeof(rGridDimension)+sizeof(matrixFreeMethod)
					+sizeof(double)*(
						rDofIsConstraint.num_blocks()+youngsModulus.size()+baseStiffness.size()
						+displVector.size()+extForces.size())
				+sizeof(size_t)*(voxelId.size()+nodeId.size()+materialOfElem.size());
#ifdef NODESATELEM
		numBytesEBE+=+sizeof(size_t)*allNodesAtElem.size();
#endif
	}
	// with overhead of CGMethode with P: p,r,pr,d,h
	numBytesEBE+=6*sizeof(double)+5*sizeof(size_t)+5*sizeof(std::vector<double>)+5*sizeof(double)*displVector.size();

	std::cout<<"[NuTo::Grid3D] Required memory: "<<numBytesEBE/(1024.*1024)<<" MiB ,"<<numBytesEBE/(1000000.)<<" MB ("<<numBytesEBE<<" byte) "<<std::endl;
	outputTime<<numBytesEBE/(1000000.)<<"   ";
	outputTime.close();
	if (matrixFreeMethod)
	{
		voxelId.clear();
		baseStiffness.clear();
		myOptimizer.Initialize(numNodes*3,rGridDimension,matrixFreeMethod,voxelId,edgeId,nodeId,
			rDofIsConstraint,youngsModulus,baseStiffness,edgeStiffness,matOfEdge,
			allEdgesAtVoxel,allNodesAtNode,displVector,extForces);
	}
	else
	{
		edgeId.clear();
#ifndef NODESATELEM
		allNodesAtElem.clear();
#endif
		allNodesAtNode.clear();
		myOptimizer.Initialize(numNodes*3,rGridDimension,matrixFreeMethod,voxelId,edgeId,nodeId,
			rDofIsConstraint,youngsModulus,baseStiffness,edgeStiffness,materialOfElem,
			allNodesAtElem,allNodesAtNode,displVector,extForces);
	}

	myOptimizer.AnsysInput(numNodes,nodeId,rDofIsConstraint,youngsModulus,rGridDimension,rVoxelSpacing,materialOfElem,allNodesAtElem,displVector);

//	std::cout<<"[NuTo::Grid3D] sizeof std::vector "<<sizeof(std::vector<double>)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof double "<<sizeof(double)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof numNodes "<<sizeof(numNodes)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof rGridDimension"<<" "<<sizeof(rGridDimension)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof matrixFreeMethod "<<sizeof(matrixFreeMethod)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof voxelId"<<" "<<sizeof(voxelId)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof nodeId"<<" "<<sizeof(nodeId)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof rDofIsConstraint "<<8*rDofIsConstraint.num_blocks()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof youndsModulus "<<youngsModulus.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof materialOfElem "<<materialOfElem.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof baseStiffness "<<baseStiffness.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof(edgeStiffness) "<<sizeof(edgeStiffness)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof edgeStiffnes "<<edgeStiffness.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof allNodes "<<allNodesAtNode.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof edgeStiffness.capacity "<<edgeStiffness.capacity()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof nodeExist "<<nodeExist.num_blocks()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof nodeExist "<<nodeExist.bits_per_block<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof nodeExist "<<sizeof(nodeExist)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof nodeExist "<<nodeExist.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof dof "<<rDofIsConstraint.num_blocks()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof dof "<<rDofIsConstraint.bits_per_block<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof dof "<<sizeof(rDofIsConstraint)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof bool "<<sizeof(bool)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof double "<<sizeof(double)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof int "<<sizeof(int)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof displ "<<displVector.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof extForces "<<sizeof(extForces)<<std::endl;
//	std::cout<<"[NuTo::Grid3D] sizeof extForces "<<extForces.size()<<std::endl;
//	std::cout<<"[NuTo::Grid3D] initialized with number of bytes "<<numBytesEBE<<std::endl;
	rDofIsConstraint.clear();
	materialOfElem.clear();
	youngsModulus.clear();
	edgeStiffness.clear();
	allEdgesAtVoxel.clear();
	allNodesAtNode.clear();

	myOptimizer.Optimize();
	myOptimizer.GetParameters(displVector);
	// open file
	std::ofstream file;
    file.open("displVTK.txt");
	for(size_t i=0;i<numGridNodes;++i)
	{
		if (nodeId[i]<0)
		{
			file<<0.0<<"\n";
			file<<0.0<<"\n";
			file<<0.0<<"\n";
		}
		else
		{
			file<<displVector[3*nodeId[i]]<<"\n";
			file<<displVector[3*nodeId[i]+1]<<"\n";
			file<<displVector[3*nodeId[i]+2]<<"\n";
		}
	}
	file.close();

    file.open("displacements.txt");
	for(size_t i=0;i<numNodes;++i)
	{
			file<<displVector[3*i]<<"\n";
			file<<displVector[3*i+1]<<"\n";
			file<<displVector[3*i+2]<<"\n";
	}
	file.close();
	//
	std::vector<double> dispRef;
	std::ifstream input;
	double help=0;
	// result file only with existing nodes
	input.open("result.txt");
	if(input)	// file is open
	{
		size_t count=0;
		while(!input.eof()) // keep reading untill end-of-file
		{
			input>>help;
			dispRef.push_back(help);
			++count;
		}
		input.close();
		--count; // for last empty line

		if (count==numDofs)
		{
			double squareDiffNorm=0;
			double squareRefNorm=0;
// output of diff and ref only for VTK
//			std::ofstream diffFile;
//			diffFile.open("displDiffVTK.txt");
//			file.open("displRefVTK.txt");
			for(size_t i=0;i<numNodes;++i)
			{
//					diffFile<<displVector[3*i]-dispRef[3*i]<<"\n";
//					diffFile<<displVector[3*i+1]-dispRef[3*i+1]<<"\n";
//					diffFile<<displVector[3*i+2]-dispRef[3*i+2]<<"\n";
//					file<<dispRef[3*i]<<"\n";
//					file<<dispRef[3*i+1]<<"\n";
//					file<<dispRef[3*i+2]<<"\n";
				squareDiffNorm+=(displVector[3*i]-dispRef[3*i])*(displVector[3*i]-dispRef[3*i]);
				squareDiffNorm+=(displVector[3*i+1]-dispRef[3*i+1])*(displVector[3*i+1]-dispRef[3*i+1]);
				squareDiffNorm+=(displVector[3*i+2]-dispRef[3*i+2])*(displVector[3*i+2]-dispRef[3*i+2]);
				squareRefNorm+=(dispRef[3*i])*(dispRef[3*i]);
				squareRefNorm+=(dispRef[3*i+1])*(dispRef[3*i+1]);
				squareRefNorm+=(dispRef[3*i+2])*(dispRef[3*i+2]);
			}
		std::cout<<"[NuTo::Grid3D] squared diff norm " <<squareDiffNorm<<std::endl;
		std::cout<<"[NuTo::Grid3D] error " <<sqrt(squareDiffNorm)/sqrt(squareRefNorm)*100<<" %"<<std::endl;
		}
		else
			std::cout<<"[NuTo::Grid3D] Comparison with reference results is not possible (wrong size).\n";
	}
	else
		std::cout<<"[NuTo::Grid3D] Comparison with reference results is not possible (no result file).\n";
	return 0;
}
