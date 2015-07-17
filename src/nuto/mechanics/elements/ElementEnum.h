// $Id$
#ifndef ELEMENTENUM_H_
#define ELEMENTENUM_H_

namespace NuTo
{
namespace Element
{
enum eElementType
{
    BOUNDARYGRADIENTDAMAGE1D=0,     //!< boundary element for gradient models
    BOUNDARYMOISTURETRANSPORT1D,    //!< boundary element for moisture transport
    BRICK8N,                        //!< three-dimensional brick element with 8 nodes
    BRICK20N,                       //!< three-dimensional brick element with 20 nodes - serendipitiy
    PLANE2D10N,                     //!< two-dimensional plane element with 10 nodes
    PLANE2D15N,                     //!< two-dimensional plane element with 10 nodes
    PLANE2D3N,                      //!< two-dimensional plane element with 3 nodes
    PLANE2D4N,                      //!< two-dimensional plane element with 4 nodes
    PLANE2D4NSPECTRALORDER2,        //!< two-dimensional plane element with 2x2 nodes for geometry interpolation and 3x3 for field interpolation
    PLANE2D4NSPECTRALORDER3,        //!< two-dimensional plane element with 2x2 nodes for geometry interpolation and 4x4 for field interpolation
    PLANE2D4NSPECTRALORDER4,        //!< two-dimensional plane element with 2x2 nodes for geometry interpolation and 5x5 for field interpolation
    PLANE2D6N,                      //!< two-dimensional plane element with 6 nodes
    TETRAHEDRON4N,                  //!< three-dimensional tetrahedron element with 4 nodes
    TETRAHEDRON10N,                 //!< three-dimensional tetrahedron element with 10 nodes
    TRUSS1D2N,                      //!< one-dimensional truss element with two nodes
    TRUSS1D3N,                      //!< one-dimensional truss element with three nodes
    TRUSS1D4NDISP3NX,               //!< one-dimensional truss element with four nodes for displacements, three for another dof
    TRUSS1D2NSPECTRALORDER2,        //!< one-dimensional truss element with two nodes for geometry and 3 for field interpolation
    TRUSS1D2NSPECTRALORDER3,        //!< one-dimensional truss element with two nodes for geometry and 4 for field interpolation
    TRUSS1D2NSPECTRALORDER4,        //!< one-dimensional truss element with two nodes for geometry and 5 for field interpolation
    ELEMENT1D,                      //!< one dimensional element
    ELEMENT1DIn2D,                  //!< one dimensional element in 2D
    ELEMENT2D,                      //!< two dimensional element
    ELEMENT3D,                      //!< three dimensional element
    BOUNDARYELEMENT1D,              //!< boundary of 1D element --> 0D surface
    BOUNDARYELEMENT2D,              //!< boundary of 2D element --> 1D surface
    BOUNDARYELEMENT3D,              //!< boundary of 3D element --> 2D surface
    BOUNDARYELEMENT2DADDITIONALNODE, //!< boundary of 2D element --> 1D surface with additional node as dependency
    BOUNDARYELEMENT3DADDITIONALNODE //!< boundary of 3D element --> 2D surface with additional node as dependency
};

enum eUpdateType
{
    STATICDATA=0,		          //!< @ToDo[eUpdateType]: Description
    TMPSTATICDATA,		          //!< @ToDo[eUpdateType]: Description
    CRACK,				          //!< update for crack informations
    SWITCHMULTISCALE2NONLINEAR    //!< move the fine scale model in a multiscale approach to the nonlinear part
};

enum eOutput
{
	INTERNAL_GRADIENT,               //!<
	INTERNAL_GRADIENT_ELASTIC,       //!< calculates internal gradient for the case that the state variables remain unchanged
	EXTERNAL_GRADIENT,               //!< TODO: calculate external forces in element
	HESSIAN_0_TIME_DERIVATIVE,       //!<
	HESSIAN_0_TIME_DERIVATIVE_ELASTIC,	//!<
	HESSIAN_1_TIME_DERIVATIVE,       //!<
	HESSIAN_2_TIME_DERIVATIVE,       //!<
	LUMPED_HESSIAN_2_TIME_DERIVATIVE,//!<
	UPDATE_STATIC_DATA,
	UPDATE_TMP_STATIC_DATA,
	IP_DATA,                         //!< this is primarily for plotting, give the 3D state  so for plane stress there is a z-component in the strain
	GLOBAL_ROW_DOF,                  //!< calculates the row dofs of the local element matrices
	GLOBAL_COLUMN_DOF,               //!< calculates the column dofs of the local element matrices
	FATIGUE_SAVE_STATIC_DATA,		 //!< for fatigue applications: saving static data in case if the cycle jump should be repeated
	FATIGUE_RESTORE_STATIC_DATA,     //!< cycle jump should be repeated, getting the previous state
	FATIGUE_EXTRAPOLATE_STATIC_DATA  //!< extrapolate static data to make a jump
};

enum eInput
{
    SOME_LOAD                        //! < some dummy for calculating the external load vector
};

}
}
#endif /* ELEMENTENUM_H_ */
