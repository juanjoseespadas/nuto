// $Id: LoadLoadSurfaceBase3D.cpp 178 2009-12-11 20:53:12Z eckardt4 $
#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/loads/LoadSurfacePressure2D.h"


//! @brief constructor
NuTo::LoadSurfacePressure2D::LoadSurfacePressure2D(int rLoadCase, StructureBase* rStructure,int rElementGroupId,int rNodeGroupId,
		double rPressure) :
		LoadSurfaceBase2D(rLoadCase,rStructure,rElementGroupId,rNodeGroupId)
{
	mPressure = rPressure;
}

//! @brief calculates the surface load as a function of the coordinates and the normal (for pressure)
//! @param rCoordinates ... global coordinates
//! @param rNormal ... normal to the surface (pointing outwards)
//! @param rLoadVector ... load vector
void NuTo::LoadSurfacePressure2D::CalculateSurfaceLoad(NuTo::FullVector<double,2>& rCoordinates,NuTo::FullVector<double,2>& rNormal,
		NuTo::FullVector<double,2>& rLoadVector)const
{
	assert(std::abs(rNormal.Norm()-1.)<1e-5);
	rLoadVector = rNormal*(-mPressure);
}


#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::LoadSurfacePressure2D)
#endif
