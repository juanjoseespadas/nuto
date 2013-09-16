// $Id: VelocityVerlet.cpp 575 2011-09-20 18:05:35Z unger3 $

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/ptr_container/serialize_ptr_map.hpp>
#endif // ENABLE_SERIALIZATION

# ifdef _OPENMP
#include <omp.h>
# endif

#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/timeIntegration/VelocityVerlet.h"
#include "nuto/mechanics/timeIntegration/TimeIntegrationEnum.h"
#include "nuto/math/FullMatrix.h"

//! @brief constructor
//! @param mDimension number of nodes
NuTo::VelocityVerlet::VelocityVerlet ()  : TimeIntegrationBase ()
{
    mTimeStep = 0.;
}


//! @brief ... Info routine that prints general information about the object (detail according to verbose level)
void NuTo::VelocityVerlet::Info()const
{
	TimeIntegrationBase::Info();
}

#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::VelocityVerlet::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::VelocityVerlet::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::VelocityVerlet::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::VelocityVerlet::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::VelocityVerlet::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::VelocityVerlet::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::VelocityVerlet::serialize(Archive & ar, const unsigned int version)
{
    #ifdef DEBUG_SERIALIZATION
        std::cout << "start serialization of VelocityVerlet" << "\n";
    #endif
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TimeIntegrationBase)
           & BOOST_SERIALIZATION_NVP(mTimeStep);
    #ifdef DEBUG_SERIALIZATION
        std::cout << "finish serialization of VelocityVerlet" << "\n";
    #endif
}

#endif // ENABLE_SERIALIZATION


//! @brief perform the time integration
//! @param rStructure ... structure
//! @param rTimeDelta ... length of the simulation
NuTo::Error::eError NuTo::VelocityVerlet::Solve(StructureBase& rStructure, double rTimeDelta)
{
#ifdef SHOW_TIME
    std::clock_t start,end;
#ifdef _OPENMP
    double wstart = omp_get_wtime ( );
#endif
    start=clock();
#endif
    try
    {
        if (mTimeStep==0.)
            mTimeStep = rStructure.ElementTotalCalculateCriticalTimeStep();

        std::cout << "time step " << mTimeStep << std::endl;
        std::cout << "number of time steps " << rTimeDelta/mTimeStep << std::endl;

        //calculate the node ptrs for the postprocessing routines
        CalculateOutputDispNodesPtr(rStructure);

        //renumber dofs and build constraint matrix
        rStructure.NodeBuildGlobalDofs();

        //calculate constraint matrix
        NuTo::SparseMatrixCSRGeneral<double> CmatTmp;
        rStructure.ConstraintGetConstraintMatrixAfterGaussElimination(CmatTmp);
        NuTo::SparseMatrixCSRVector2General<double> Cmat(CmatTmp);
        SparseMatrixCSRVector2General<double> CmatT (Cmat.Transpose());
        FullVector<double,Eigen::Dynamic> bRHS, bRHSdot, bRHSddot;
        if (CmatT.GetNumEntries() > 0)
        {
            throw MechanicsException("[NuTo::VelocityVerlet::Solve] not implemented for constrained systems including multiple dofs.");
        }

        //calculate individual inverse mass matrix, use only lumped mass matrices - stored as fullvectors and then use asDiagonal()
        NuTo::FullVector<double,Eigen::Dynamic> invMassMatrix_j(rStructure.GetNumActiveDofs());

        //extract displacements, velocities and accelerations
        NuTo::FullVector<double,Eigen::Dynamic> disp_j, vel_j, acc_j,acc_new_j,tmp_k;
        NuTo::FullVector<double,Eigen::Dynamic> extForce_j, extForce_k;
        NuTo::FullVector<double,Eigen::Dynamic> intForce_j(rStructure.GetNumActiveDofs()),
        		                                intForce_k(rStructure.GetNumDofs() - rStructure.GetNumActiveDofs());

        //store last converged displacements, velocities and accelerations
        rStructure.NodeExtractDofValues(0,disp_j,tmp_k);
        rStructure.NodeExtractDofValues(1,vel_j,tmp_k);
        rStructure.NodeExtractDofValues(2,acc_j,tmp_k);

        //calculate lumped mass matrix
        //intForce_j is just used as a tmp variable
        rStructure.BuildGlobalLumpedHession2(intForce_j,tmp_k);

        //check the sum of all entries
        std::cout << "the total mass is " << intForce_j.sum()/3. +  tmp_k.sum()/3. << std::endl;

        //invert the mass matrix
        invMassMatrix_j = intForce_j.cwiseInverse();

        double curTime  = 0;

        while (curTime < rTimeDelta)
        {
         	//increase time step
            curTime += mTimeStep;
            std::cout << "curTime " << curTime <<   " (" << curTime/rTimeDelta << ") max Disp = "  <<  disp_j.maxCoeff() << std::endl;

            //apply constraints for new time stepdouble RHSConstraint
            double timeDependentConstraintFactor(0);
            if (mTimeDependentConstraint!=-1)
            {
                timeDependentConstraintFactor = this->CalculateTimeDependentConstraintFactor(curTime);
                rStructure.ConstraintSetRHS(mTimeDependentConstraint,timeDependentConstraintFactor);
				rStructure.ConstraintGetRHSAfterGaussElimination(bRHS);
            }

            //calculate new displacement approximation (disp_k is calculated internally when NodeMerge is called
            disp_j += vel_j * mTimeStep +  acc_j * (mTimeStep*mTimeStep);

            rStructure.NodeMergeActiveDofValues(0,disp_j);
			rStructure.ElementTotalUpdateTmpStaticData();

			//calculate external force
			CalculateExternalLoad(rStructure, curTime, extForce_j, extForce_k);

            //calculate internal force (with update of history variables = true)
            rStructure.BuildGlobalGradientInternalPotentialSubVectors(intForce_j,intForce_k,true);

            //**********************************************
            //PostProcessing
            //**********************************************
            FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> plotVector(1,2);
            plotVector(0,0) = mTime+mTimeStep;
            plotVector(0,1) = timeDependentConstraintFactor;

            for (unsigned int countNode=0; countNode<mVecOutputDispNodesPtr.size(); countNode++)
            {
                switch(mVecOutputDispNodesPtr[countNode] -> GetNumDisplacements())
                {
                case 1:
                {
                	FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> displacements(1,1);
                	mVecOutputDispNodesPtr[countNode]->GetDisplacements1D(displacements.data());
                    plotVector.AppendColumns(displacements);
                }
                break;
                case 2:
                {
                	FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> displacements(1,2);
                	mVecOutputDispNodesPtr[countNode]->GetDisplacements2D(displacements.data());
                    plotVector.AppendColumns(displacements);
                }
                break;
                case 3:
                {
                	FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic> displacements(1,3);
                	mVecOutputDispNodesPtr[countNode]->GetDisplacements3D(displacements.data());
                    plotVector.AppendColumns(displacements);
                }
                break;
                default:
                    break;
                }
            }

            //postprocess data for plotting
            this->PostProcess(rStructure, plotVector);

            //calculate new accelerations and velocities of independent dofs
            acc_new_j = invMassMatrix_j.asDiagonal()*(extForce_j-intForce_j);
			vel_j += 0.5*mTimeStep*(acc_j+acc_new_j);

			//update new accelarations
            acc_j = acc_new_j;

			//update time, this is different from curTime if several load cycles are sequentially
            mTime+=mTimeStep;
        }
    }
    catch (MechanicsException& e)
    {
        e.AddMessage("[NuTo::VelocityVerlet::Solve] performing Newton-Raphson iteration.");
        throw e;
    }
#ifdef SHOW_TIME
    end=clock();
#ifdef _OPENMP
    double wend = omp_get_wtime ( );
    if (mShowTime)
        rStructure.GetLogger()<<"[NuTo::VelocityVerlet::Solve] " << difftime(end,start)/CLOCKS_PER_SEC << "sec(" << wend-wstart <<")\n";
#else
    if (mShowTime)
        rStructure.GetLogger()<< "[NuTo::VelocityVerlet::Solve] " << difftime(end,start)/CLOCKS_PER_SEC << "sec" << "\n";
#endif
#endif
    return NuTo::Error::SUCCESSFUL;

}

//! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
//!            in case of restoring from a file with the wrong object type, the file id is printed
//! @return    class name
std::string NuTo::VelocityVerlet::GetTypeId()const
{
    return std::string("VelocityVerlet");
}


#ifdef ENABLE_SERIALIZATION
//! @brief ... restore the object from a file
//! @param filename ... filename
//! @param aType ... type of file, either BINARY, XML or TEXT
//! @brief ... save the object to a file
void NuTo::VelocityVerlet::Restore (const std::string &filename, std::string rType )
{
    try
    {
        //transform to uppercase
        std::transform(rType.begin(), rType.end(), rType.begin(), toupper);
        std::ifstream ifs ( filename.c_str(), std::ios_base::binary );
        std::string tmpString;
        if (rType=="BINARY")
        {
            boost::archive::binary_iarchive oba ( ifs, std::ios::binary );
            oba & boost::serialization::make_nvp ( "Object_type", tmpString );
            if ( tmpString!=GetTypeId() )
                throw MechanicsException ( "[VelocityVerlet::Restore]Data type of object in file ("+tmpString+") is not identical to data type of object to read ("+GetTypeId() +")." );
            oba & boost::serialization::make_nvp(tmpString.c_str(), *this);
        }
        else if (rType=="XML")
        {
            boost::archive::xml_iarchive oxa ( ifs, std::ios::binary );
            oxa & boost::serialization::make_nvp ( "Object_type", tmpString );
            if ( tmpString!=GetTypeId() )
                throw MechanicsException ( "[VelocityVerlet::Restore]Data type of object in file ("+tmpString+") is not identical to data type of object to read ("+GetTypeId() +")." );
            oxa & boost::serialization::make_nvp(tmpString.c_str(), *this);
        }
        else if (rType=="TEXT")
        {
            boost::archive::text_iarchive ota ( ifs, std::ios::binary );
            ota & boost::serialization::make_nvp ( "Object_type", tmpString );
            if ( tmpString!=GetTypeId() )
                throw MechanicsException ( "[VelocityVerlet::Restore]Data type of object in file ("+tmpString+") is not identical to data type of object to read ("+GetTypeId() +")." );
            ota & boost::serialization::make_nvp(tmpString.c_str(), *this);
        }
        else
        {
            throw MathException ( "[Matrix::Restore]File type not implemented" );
        }
    }
    catch ( MechanicsException &e )
    {
        throw e;
    }
    catch ( std::exception &e )
    {
        throw MechanicsException ( e.what() );
    }
    catch ( ... )
    {
        throw MechanicsException ( "[VelocityVerlet::Restore]Unhandled exception." );
    }
}

//  @brief this routine has to be implemented in the final derived classes, which are no longer abstract
//! @param filename ... filename
//! @param aType ... type of file, either BINARY, XML or TEXT
void NuTo::VelocityVerlet::Save (const std::string &filename, std::string rType )const
{
    try
    {
        //transform to uppercase
        std::transform(rType.begin(), rType.end(), rType.begin(), toupper);
        std::ofstream ofs ( filename.c_str(), std::ios_base::binary );
        std::string tmpStr ( GetTypeId() );
        std::string baseClassStr = tmpStr.substr ( 4,100 );
        if (rType=="BINARY")
        {
            boost::archive::binary_oarchive oba ( ofs, std::ios::binary );
            oba & boost::serialization::make_nvp ( "Object_type", tmpStr );
            oba & boost::serialization::make_nvp(tmpStr.c_str(), *this);
        }
        else if (rType=="XML")
        {
            boost::archive::xml_oarchive oxa ( ofs, std::ios::binary );
            oxa & boost::serialization::make_nvp ( "Object_type", tmpStr );
            oxa & boost::serialization::make_nvp(tmpStr.c_str(), *this);
        }
        else if (rType=="TEXT")
        {
            boost::archive::text_oarchive ota ( ofs, std::ios::binary );
            ota & boost::serialization::make_nvp ( "Object_type", tmpStr );
            ota & boost::serialization::make_nvp(tmpStr.c_str(), *this);
        }
        else
        {
            throw MechanicsException ( "[VelocityVerlet::Save]File type not implemented." );
        }
    }
    catch ( boost::archive::archive_exception e )
    {
        std::string s ( std::string ( "[VelocityVerlet::Save]File save exception in boost - " ) +std::string ( e.what() ) );
        std::cout << s << "\n";
        throw MathException ( s );
    }
    catch ( MechanicsException &e )
    {
        throw e;
    }
    catch ( std::exception &e )
    {
        throw MechanicsException ( e.what() );
    }
    catch ( ... )
    {
        throw MechanicsException ( "[VelocityVerlet::Save]Unhandled exception." );
    }
}

#ifndef SWIG
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::VelocityVerlet)
#endif // SWIG
#endif // ENABLE_SERIALIZATION