#pragma once

#include <cantera/Cantera.h>
#include <cantera/IdealGasMix.h>    // defines class IdealGasMix
#include <cantera/equilibrium.h>    // chemical equilibrium
#include <cantera/thermo.h>
#include <cantera/transport.h>      // transport properties
#include <cantera/kinetics.h>

#include <cantera/kernel/IdealGasPhase.h>
#include <cantera/kernel/GasKinetics.h>
#include <cantera/kernel/ctml.h>

#include "mathUtils.h"

#ifdef WIN32
#define Cantera_CXX Cantera
#endif

// NOTE: Use of this class requires a slightly modified installation of Cantera
// gasArray modifies protected members of Transport objects, requiring gasArray
// to be declared as a friend of Cantera's transport-related classes.
// In the files "transport/TransportBase.h", "transport/MixTransport.h" and
// "transport/MultiTransport.h", add the following declaration near the top of
// the file (before the start of namespace Cantera):
//
// class gasArray;
//
// And, as the first line of class Transport, class MixTransport and
// class MultiTransport, add the friend declaration (before "public:")
//
// friend class ::gasArray;
//

class gasArray
{
public:
	gasArray();
	~gasArray();
	std::string mechanismFile;
	std::string phaseID;
	double pressure; // thermodynamic pressure
	int nSpec;

	bool usingMultiTransport; // use multicomponent transport model? (vs. mixture-averaged)

	void initialize(bool multiTransportFlag);
	void resize(unsigned int n);

	void setStateMass(Cantera::Array2D& Y, dvector& T);
	void setStateMole(Cantera::Array2D& X, dvector& T);

	void getMoleFractions(Cantera::Array2D& X);
	void getMassFractions(Cantera::Array2D& Y);
	void getDensity(dvector& rho);
	void getMixtureMolecularWeight(dvector& Wmx);
	void getMolecularWeights(dvector& W);

	void getViscosity(dvector& mu);
	void getThermalConductivity(dvector& lambda);
	void getDiffusionCoefficients(Cantera::Array2D& Dkm);
	void getWeightedDiffusionCoefficients(Cantera::Array2D& rhoD);
	void getThermalDiffusionCoefficients(Cantera::Array2D& Dkt);

	void getSpecificHeatCapacity(dvector& cp);
	void getSpecificHeatCapacities(Cantera::Array2D& cpSpec);
	void getEnthalpies(Cantera::Array2D& hk);

	void getReactionRates(Cantera::Array2D& wDot);

	Cantera::IdealGasPhase& operator[](unsigned int i) const;
	Cantera::IdealGasPhase& thermo(unsigned int i) const;
	Cantera::GasKinetics& kinetics(unsigned int i) const;
	Cantera::MultiTransport& multiTrans(unsigned int i) const;
	Cantera::MixTransport& mixTrans(unsigned int i) const;

private:
	Cantera::XML_Node* rootXmlNode;
	Cantera::XML_Node* phaseXmlNode;

	int nPoints;

	vector<Cantera::IdealGasPhase*> m_thermo;
	vector<Cantera::GasKinetics*> m_kinetics;
	vector<Cantera::MultiTransport*> m_MultiTransport;
	vector<Cantera::MixTransport*> m_MixTransport;

	// Default objects
	Cantera::IdealGasPhase m_thermoBase;
	Cantera::GasKinetics* m_kineticsBase;
	Cantera::MultiTransport* m_MultiTransportBase;
	Cantera::MixTransport* m_MixTransportBase;
};

// Like class GasArray, but this one only uses a single set of Cantera objects,
// and relies on iteratively setting the state and calculating the desired property
// at each point.
class simpleGasArray
{
public:
	simpleGasArray();
	~simpleGasArray();

	std::string mechanismFile;
	std::string phaseID;
	double pressure; // thermodynamic pressure
	int nSpec; // number of species

	bool usingMultiTransport; // use multicomponent transport model? (vs. mixture-averaged)

	void initialize(bool multiTransportFlag);
	void resize(unsigned int n);

	void setStateMass(Array2D& Y, dvector& T);
	void setStateMole(Array2D& X, dvector& T);

	void getMoleFractions(Array2D& X);
	void getMassFractions(Array2D& Y);
	void getDensity(dvector& rho);
	void getMixtureMolecularWeight(dvector& Wmx);
	void getMolecularWeights(dvector& W);

	void getViscosity(dvector& mu);
	void getThermalConductivity(dvector& lambda);
	void getDiffusionCoefficients(Array2D& Dkm);
	void getWeightedDiffusionCoefficients(Array2D& rhoD);
	void getThermalDiffusionCoefficients(Array2D& Dkt);

	void getSpecificHeatCapacity(dvector& cp);
	void getSpecificHeatCapacities(Array2D& cpSpec);
	void getEnthalpies(Array2D& hk);

	void getReactionRates(Array2D& wDot);

	void getTransportProperties(dvector& mu, dvector& lambda, Array2D& rhoD, Array2D& Dkt);
	void getThermoProperties(dvector& rho, dvector& Wmx, dvector& cp, Array2D& cpSpec, Array2D& hk);

	Cantera::IdealGasPhase thermo;

private:
	Cantera::XML_Node* rootXmlNode;
	Cantera::XML_Node* phaseXmlNode;

	int nPoints;
	Array2D Y;
	dvector T;

	Cantera::GasKinetics* kinetics;
	Cantera::MultiTransport* multiTransport;
	Cantera::MixTransport* mixTransport;
};
