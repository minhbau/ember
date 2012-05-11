#pragma once

#include "mathUtils.h"
#include "sundialsUtils.h"
#include "strainFunction.h"
#include "grid.h"
#include "chemistry0d.h"
#include "perfTimer.h"
#include "quasi2d.h"

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class ConvectionSystemUTW : public sdODE, public GridBased
{
    // System representing the coupled convection equations for U, T, and Wmx
    // (tangential velocity, temperature, and mixture molecular weight)
public:
    ConvectionSystemUTW();

    // The ODE function: ydot = f(t,y)
    int f(const realtype t, const sdVector& y, sdVector& ydot);

    void unroll_y(const sdVector& y); // fill in current state variables from sdvector
    void roll_y(sdVector& y) const; // fill in sdvector with current state variables
    void roll_ydot(sdVector& ydot) const; // fill in sdvector with current time derivatives

    void resize(const size_t nPoints);
    void resetSplitConstants();

    void updateContinuityBoundaryCondition(const dvector& qdot,
                                           ContinuityBoundaryCondition::BC newBC);

    dvector U, dUdt;
    dvector T, dTdt;
    dvector Wmx, dWdt;

    double Tleft; // Temperature left boundary value
    double Wleft; // mixture molecular weight left boundary value
    double rVzero; // mass flux boundary value at j=0

    dvector drhodt;

    // Constant terms introduced by the splitting method
    dvector splitConstT;
    dvector splitConstW;
    dvector splitConstU;

    // Cantera data
    CanteraGas* gas;

    // Auxiliary variables
    dvector V; // mass flux [kg/m^2*s]
    dvector rV; // (radial) mass flux (r*V) [kg/m^2*s or kg/m*rad*s]
    dvector rho; // mixture density [kg/m^3]

    // variables used internally
    dvector dUdx;
    dvector dTdx;
    dvector dWdx;

    ContinuityBoundaryCondition::BC continuityBC;
    size_t jContBC; // The point at which the continuity equation BC is applied
    double xVzero; // Location of the stagnation point (if using fixedZero BC)

private:
    void V2rV();
    void rV2V();

    size_t nVars; // == 3
};

typedef std::map<double, dvector> vecInterpolator;

class ConvectionSystemY : public sdODE, public GridBased
{
    // System representing the convection equation for a single species
    // with a prescribed velocity field V
public:
    ConvectionSystemY() : quasi2d(false) {}

    // The ODE function: ydot = f(t,y)
    int f(const realtype t, const sdVector& y, sdVector& ydot);

    void resize(const size_t nPoints);
    void resetSplitConstants();

    double Yleft;
    int k; // species index (only needed for debugging)
    dvector splitConst; // constant term introduced by splitting

    size_t startIndex;
    size_t stopIndex;

    boost::shared_ptr<vecInterpolator> vInterp; // axial (normal) velocity [m/s] at various times

    //! Interpolators for the quasi-2d problem
    boost::shared_ptr<BilinearInterpolator> vzInterp;
    boost::shared_ptr<BilinearInterpolator> vrInterp;
    bool quasi2d;

private:
    void update_v(const double t);
    dvector v;
};


class ConvectionSystemSplit : public GridBased
{
    // System which combines a ConvectionSystemUTW objects and several
    // ConvectionSystemY objects that together represent the complete
    // convection term for all components.
public:
    ConvectionSystemSplit();

    void setGrid(const oneDimGrid& grid);
    void setTolerances(const configOptions& options);
    void setGas(CanteraGas& gas);
    void resize(const size_t nPointsUTW, const vector<size_t>& nPointsSpec, const size_t nSpec);
    void setSpeciesDomains(vector<size_t>& startIndices, vector<size_t>& stopIndices);
    void setState(const dvector& U, const dvector& T, dmatrix& Y, double tInitial);
    void setLeftBC(const double Tleft, const dvector& Yleft);
    void set_rVzero(const double rVzero);
    void evaluate(); // evaluate time derivatives and mass flux at the current state

    // Time derivatives of species and temperature from the other split terms are needed
    // to correctly compute the density derivative appearing in the continuity equation
    void setDensityDerivative(const dvector& drhodt);

    // Constants introduced by the splitting method
    void setSplitConstants(const dvector& splitConstU,
                           const dvector& splitConstT,
                           const dmatrix& splitConstY);

    void resetSplitConstants();

    void integrateToTime(const double tf);
    void unroll_y(); // convert the solver's solution vectors to the full U, Y, and T
    int getNumSteps();
    void setupQuasi2D(boost::shared_ptr<BilinearInterpolator>& vzInterp,
                      boost::shared_ptr<BilinearInterpolator>& vrInterp);

    dvector U;
    dvector T;
    dvector Wmx;
    dmatrix Y;

    // Time derivatives and mass flux are updated by evaluate()
    dvector V;
    dvector dUdt;
    dvector dTdt;
    dvector dWdt;
    dmatrix dYdt;

    ConvectionSystemUTW utwSystem;

    boost::shared_ptr<vecInterpolator> vInterp;

    PerfTimer utwTimer, speciesTimer;

private:
    // set parameters of a new species solver
    void configureSolver(sundialsCVODE& solver, const size_t k);

    // CVODE integration tolerances
    double reltol; // relative tolerance
    double abstolU; // velocity absolute tolerance
    double abstolT; // temperature absolute tolerance
    double abstolW; // molecular weight absolute tolerance
    double abstolY; // mass fraction absolute tolerance

    boost::shared_ptr<sundialsCVODE> utwSolver;
    boost::ptr_vector<ConvectionSystemY> speciesSystems;
    boost::ptr_vector<sundialsCVODE> speciesSolvers;

    dvector Yleft;
    dvector W;

    size_t nSpec;
    size_t nVars;
    size_t nPointsUTW;
    vector<size_t> nPointsSpec;

    vector<size_t>* startIndices; // index of leftmost grid point for each component (U,T,Yk)
    vector<size_t>* stopIndices; // index of rightmost grid point for each component (U,T,Yk)

    CanteraGas* gas;

    bool quasi2d;
};
