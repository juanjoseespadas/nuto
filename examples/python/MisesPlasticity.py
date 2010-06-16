import nuto
import sys
import os
import numpy
import Gnuplot

#create structure
myStructure = nuto.Structure(3)

#create nodes
myNode1 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,(+1,+1,+1)))
myNode2 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,( 0,+1,+1)))
myNode3 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,( 0, 0,+1)))
myNode4 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,(+1, 0,+1)))
myNode5 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,(+1,+1, 0)))
myNode6 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,( 0,+1, 0)))
myNode7 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,( 0, 0, 0)))
myNode8 = myStructure.NodeCreate("displacements",nuto.DoubleFullMatrix(3,1,(+1, 0, 0)))

#create element
myElement1 = myStructure.ElementCreate("Brick8N",nuto.IntFullMatrix(8,1,(myNode1,myNode2,myNode3,myNode4,myNode5,myNode6,myNode7,myNode8)),"ConstitutiveLawElement_StaticData")

#create constitutive law
myStructure.ConstitutiveLawCreate("myMat","MisesPlasticity")
#myStructure.ConstitutiveLawCreate("myMat","LinearElastic")

myStructure.ConstitutiveLawSetYoungsModulus("myMat",100)
myStructure.ConstitutiveLawSetPoissonsRatio("myMat",0.0)
myStructure.ConstitutiveLawSetInitialYieldStrength("myMat",100)
myStructure.ConstitutiveLawAddYieldStrength("myMat",0.25,150)
myStructure.ConstitutiveLawAddYieldStrength("myMat",0.3,150)
#myStructure.ConstitutiveLawSetInitialHardeningModulus("myMat",0)
#myStructure.ConstitutiveLawAddHardeningModulus("myMat",0.5,0)
#myStructure.ConstitutiveLawAddHardeningModulus("myMat",0.2,1)
myStructure.ConstitutiveLawInfo(10)

#assign constitutive law 
myStructure.ElementSetConstitutiveLaw(myElement1,"myMat")

#apply constraints of left boundary
direction = nuto.DoubleFullMatrix(3,1,(1,0,0))
myStructure.ConstraintSetDisplacementNode(myNode2, direction, 0)
myStructure.ConstraintSetDisplacementNode(myNode3, direction, 0)
myStructure.ConstraintSetDisplacementNode(myNode6, direction, 0)
myStructure.ConstraintSetDisplacementNode(myNode7, direction, 0)
direction = nuto.DoubleFullMatrix(3,1,(0,1,0))
myStructure.ConstraintSetDisplacementNode(myNode3, direction, 0)
myStructure.ConstraintSetDisplacementNode(myNode7, direction, 0)
direction = nuto.DoubleFullMatrix(3,1,(0,0,1))
myStructure.ConstraintSetDisplacementNode(myNode6, direction, 0)
myStructure.ConstraintSetDisplacementNode(myNode7, direction, 0)

#create group of nodes at right boundary
myStructure.GroupCreate("NodeGroupRightBoundary","Nodes")
myStructure.GroupAddNode("NodeGroupRightBoundary",myNode1)
myStructure.GroupAddNode("NodeGroupRightBoundary",myNode4)
myStructure.GroupAddNode("NodeGroupRightBoundary",myNode5)
myStructure.GroupAddNode("NodeGroupRightBoundary",myNode8)

#apply displacement at right boundary
direction = nuto.DoubleFullMatrix(3,1,(1,0,0))
constraint_right_side = myStructure.ConstraintSetDisplacementNodeGroup("NodeGroupRightBoundary", direction, 0)

#initialize gnuplot
g = Gnuplot.Gnuplot(debug=1)
g.title('stress strain curve')   # (optional)
g("set style line 1 lt 1 lw 1.5 pt 5 ps 0.5 lc rgb 'red'")
g("set style line 2 lt 1 lw 1.5 pt 7 ps 0.5 lc rgb 'blue'")
g("set yrange [0:170]")


#loop over boundaryDisplacement
max_disp = 2.
num_steps = 10
stiffnessMatrix = nuto.DoubleSparseMatrixCSRGeneral()
dispForceVector = nuto.DoubleFullMatrix()
extForceVector = nuto.DoubleFullMatrix()
mySolver = nuto.SparseDirectSolverMUMPS()
displacementVector = nuto.DoubleFullMatrix()
deltaDisplacementVector = nuto.DoubleFullMatrix()
stiffnessMatrix.SetOneBasedIndexing()
intForceVector = nuto.DoubleFullMatrix()
boundaryForceVector = nuto.DoubleFullMatrix()
numActiveDofs = 0
plotMatrixLoadDisp = nuto.DoubleFullMatrix(2,1)
plotVectorLoadDisp = nuto.DoubleFullMatrix(2,1)
for i in range(0, num_steps):
    boundaryDisplacement = max_disp*(i+1)/num_steps
    print "boundary displacement :" + str(boundaryDisplacement)
    myStructure.ConstraintSetRHS(constraint_right_side,boundaryDisplacement)
   
    #number dofs and perform gauss elimination of the constraint matrix
    myStructure.NodeBuildGlobalDofs()
    
    if (numActiveDofs==0):
        numActiveDofs = myStructure.NodeGetNumberActiveDofs()
        intForceVector.Resize(numActiveDofs,1)
        displacementVector.Resize(numActiveDofs,1)

    #do while loop does not exist in python
    iteration = 0
    while True:
        #increment iteration
        iteration+=1
    
        # build global stiffness matrix and equivalent load vector which correspond to prescribed boundary values
        myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrix, dispForceVector)

        # build global external load vector
        myStructure.BuildGlobalExternalLoadVector(extForceVector)

        # calculate right hand side
        rhsVector = dispForceVector + extForceVector - intForceVector

        # solve
        mySolver.Solve(stiffnessMatrix, rhsVector, deltaDisplacementVector)

        #add delta
        displacementVector+=deltaDisplacementVector
        
        # write displacements to node
        myStructure.NodeMergeActiveDofValues(displacementVector)

        # calculate internal force vector
        myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector)

        # calculate residual
        residualVector = extForceVector - intForceVector
        print "residual: " + str(residualVector.Norm())

        #calculate engineering strain of myelement at all integration points
        EngineeringStrain = nuto.DoubleFullMatrix(6,3)
        myStructure.ElementGetEngineeringStrain(myElement1, EngineeringStrain)
        print "strain in element 1"
        EngineeringStrain.Info()

        #calculate engineering plastic strain of myelement at all integration points
        EngineeringPlasticStrain = nuto.DoubleFullMatrix(6,3)
        myStructure.ElementGetEngineeringPlasticStrain(myElement1, EngineeringPlasticStrain)
        print "plastic strain in element 1"
        EngineeringPlasticStrain.Info()

        #calculate engineering stress of myelement at all integration points
        EngineeringStress = nuto.DoubleFullMatrix(6,3)
        myStructure.ElementGetEngineeringStress(myElement1, EngineeringStress)
        print "stress in element 1"
        EngineeringStress.Info()
        
        # check for convergence
        if (residualVector.Norm()<1e-6):
            break
        else:
            print "iteration " + str(iteration)
            _ = raw_input('More than one iteration required, press enter to continue...') 

    #update static data
    myStructure.ElementTotalUpdateStaticData()
    
    #boundary force
    myStructure.NodeGroupGetInternalForce("NodeGroupRightBoundary", boundaryForceVector)
    print "boundary force vector"
    boundaryForceVector.Info()
    print ""
    
    #add load displacement curve and write to file
    plotVectorLoadDisp.SetValue(0,0,boundaryDisplacement)
    plotVectorLoadDisp.SetValue(1,0,boundaryForceVector.GetValue(0,0))
    plotMatrixLoadDisp.AppendColumns(plotVectorLoadDisp)
    plotMatrixLoadDisp.Trans().WriteToFile( "MisesLoadDisp.dat", " ", "#load-disp for mises plasticity", "" )
    
    g("plot 'MisesLoadDisp.dat' using 1:2 title'training set' with lines linestyle 1")

    # visualize results
    myStructure.ExportVtkDataFile("MisesPlasticity.vtk","displacements engineering_strain engineering_stress")
    
_ = raw_input('press enter to continue...') 
sys.exit(0)