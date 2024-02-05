#ifndef _GeoMagLib
#define _GeoMagLib

#define GeoMagLib_EXPORTS

#ifdef GeoMagLib_EXPORTS
#define GeoMagLib_API __declspec(dllexport)
#else
#define GeoMagLib_API __declspec(dllimport)
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI    ((2)*(acos(0.0)))
#endif

#define RAD2DEG(rad)    ((rad)*(180.0L/M_PI))
#define DEG2RAD(deg)    ((deg)*(M_PI/180.0L))

class CDate
{
public:

	int Year;
	int Month;
	int Day;

	double DecimalYear;

	CDate();

	CDate(double sdate);

	~CDate();
};

class CEllipsoid
{
public:

	double a; /*semi-major axis of the ellipsoid*/
	double b; /*semi-minor axis of the ellipsoid*/
	double fla; /* flattening */
	double epssq; /*first eccentricity squared */
	double eps; /* first eccentricity */
	double re; /* mean radius of  ellipsoid*/

	CEllipsoid();

	~CEllipsoid();
};

class CGeoid
{
public:

	int NumbGeoidCols; /* 360 degrees of longitude at 15 minute spacing */
	int NumbGeoidRows; /* 180 degrees of latitude  at 15 minute spacing */
	int NumbHeaderItems; /* min, max lat, min, max long, lat, long spacing*/

	int ScaleFactor; /* 4 grid cells per degree at 15 minute spacing  */

	// included in EGM9615.h
	//float* GeoidHeightBuffer;

	int NumbGeoidElevs;
	int Geoid_Initialized; /* indicates successful initialization */
	int UseGeoid; /* Is the Geoid being used? */

	CGeoid();

	CGeoid(const char p);

	~CGeoid();

	double GetGeoidHeight(double lon, double lat);

	void ConvertGeoidToEllipsoidHeight();

private:

	long Index;

	double DeltaX, DeltaY;
	double ElevationSE, ElevationSW, ElevationNE, ElevationNW;
	double OffsetX, OffsetY;
	double PostX, PostY;
	double UpperY, LowerY;
	double DeltaHeight;
};

class CCoordGeodetic
{
public:

	double lambda; /* longitude */
	double phi; /* geodetic latitude */

	double HeightAboveEllipsoid; /* height above the ellipsoid (HaE) */
	double HeightAboveGeoid; /* (height above the EGM96 geoid model ) */

	int UseGeoid;

	CCoordGeodetic();

	CCoordGeodetic(CGeoid* Geoid, double lon, double lat, double alt);

	~CCoordGeodetic();
};

class CCoordSpherical
{
public:

	double lambda;		/* longitude */
	double phig;		/* geocentric latitude */
	double r;			/* distance from the center of the ellipsoid */

	CCoordSpherical();

	~CCoordSpherical();

	void GeodeticToSperical(CEllipsoid* Ellip, CCoordGeodetic* CoordGeodetic);

private:

	double CosLat, SinLat, rc, xp, zp;
};

class CLegendreFunction
{
public:

	int NumTerms;

	double* Pcup;	/* Legendre Function */
	double* dPcup;	/* Derivative of Legendre fcn */

	CLegendreFunction();

	CLegendreFunction(int n);

	~CLegendreFunction();

	void AssociatedLegendreFunction(CCoordSpherical* CoordSpherical, int nMax);

	void PcupLow(double x, int nMax);

	void PcupHigh(double x, int nMax);
};

#define MAXLINELENGTH 1024

#define CALCULATE_NUMTERMS(N)    (N * ( N + 1 ) / 2 + N)

class CMagneticModel
{
public:

	double EditionDate;
	double epoch; /*Base time of Geomagnetic model epoch (yrs)*/
	char ModelName[32];

	double* Main_Field_Coeff_G; /* C - Gauss coefficients of main geomagnetic model (nT) Index is (n * (n + 1) / 2 + m) */
	double* Main_Field_Coeff_H; /* C - Gauss coefficients of main geomagnetic model (nT) */
	double* Secular_Var_Coeff_G; /* CD - Gauss coefficients of secular geomagnetic model (nT/yr) */
	double* Secular_Var_Coeff_H; /* CD - Gauss coefficients of secular geomagnetic model (nT/yr) */

	int nMax; /* Maximum degree of spherical harmonic model */
	int nMaxSecVar; /* Maximum degree of spherical harmonic secular model */
	int SecularVariationUsed; /* Whether or not the magnetic secular variation vector will be needed by program*/

	double CoefficientFileEndDate;

	int num_terms;

	CMagneticModel();

	~CMagneticModel();

	CMagneticModel(int n);

	CMagneticModel(const char* f);

	void TimelyModifyMagneticModel(CDate* UserDate, CMagneticModel* MagneticModel);

private:

	FILE* infile;

	errno_t err;

	char line[MAXLINELENGTH];

	char* eof;

	int a;
	int n;
	int m;
	int index;

	double gnm, hnm, dgnm, dhnm;
};

class CSphericalHarmonicVariables
{
public:

	int nMax;

	double* RelativeRadiusPower;	/* [earth_reference_radius_km / sph. radius ]^n */
	double* cos_mlambda;			/* cp(m)  - cosine of (m*spherical coord. longitude) */
	double* sin_mlambda;			/* sp(m)  - sine of (m*spherical coord. longitude) */

	CSphericalHarmonicVariables();

	CSphericalHarmonicVariables(int n);

	~CSphericalHarmonicVariables();

	void ComputeSphericalHarmonicVariables(CEllipsoid* Ellip, CCoordSpherical* CoordSpherical, int nMax);
};

class CMagneticResults
{
public:

	double Bx; /* North */
	double By; /* East */
	double Bz; /* Down */

	CMagneticResults();

	~CMagneticResults();

	void Summation(CLegendreFunction* LegendreFunction, CMagneticModel* MagneticModel, CSphericalHarmonicVariables* SphVariables, CCoordSpherical* CoordSpherical);

	void SummationSpecial(CMagneticModel* MagneticModel, CSphericalHarmonicVariables* SphVariables, CCoordSpherical* CoordSpherical);

	void SecVarSummation(CLegendreFunction* LegendreFunction, CMagneticModel* MagneticModel, CSphericalHarmonicVariables* SphVariables, CCoordSpherical* CoordSpherical);

	void SecVarSummationSpecial(CMagneticModel* MagneticModel, CSphericalHarmonicVariables* SphVariables, CCoordSpherical* CoordSpherical);

	void RotateMagneticVector(CCoordSpherical* CoordSpherical, CCoordGeodetic* CoordGeodetic, CMagneticResults* MagneticResults);

	void SphericalToCartesian(CCoordSpherical* CoordSpherical, double* x, double* y, double* z);
};

class CGeoMagneticElements
{
public:

	double Decl;    /*  1. Angle between the magnetic field vector and true north, positive east */
	double Incl;    /*  2. Angle between the magnetic field vector and the horizontal plane, positive down */
	double F;       /*  3. Magnetic Field Strength */
	double H;       /*  4. Horizontal Magnetic Field Strength */
	double X;       /*  5. Northern component of the magnetic field vector */
	double Y;       /*  6. Eastern component of the magnetic field vector */
	double Z;       /*  7. Downward component of the magnetic field vector */
	double GV;      /*  8. The Grid Variation */
	double Decldot; /*  9. Yearly Rate of change in declination */
	double Incldot; /* 10. Yearly Rate of change in inclination */
	double Fdot;    /* 11. Yearly rate of change in Magnetic field strength */
	double Hdot;    /* 12. Yearly rate of change in horizontal field strength */
	double Xdot;    /* 13. Yearly rate of change in the northern component */
	double Ydot;    /* 14. Yearly rate of change in the eastern component */
	double Zdot;    /* 15. Yearly rate of change in the downward component */
	double GVdot;   /* 16. Yearly rate of change in grid variation */

	CGeoMagneticElements();

	~CGeoMagneticElements();

	void CalculateFieldElements(CEllipsoid* Ellip, CCoordSpherical* CoordSpherical, CCoordGeodetic* CoordGeodetic, CMagneticModel* TimedMagneticModel);

	void CalculateGeoMagneticElements(CMagneticResults* MagneticResultsGeo);

	void CalculateSecularVariationElements(CMagneticResults* MagneticVariation);

private:

	int NumTerms;

	CLegendreFunction* LegendreFunction;
	CSphericalHarmonicVariables* SphVariables;

	CMagneticResults* MagneticResultsSph;
	CMagneticResults* MagneticResultsGeo;
	CMagneticResults* MagneticResultsSphVar;
	CMagneticResults* MagneticResultsGeoVar;
};

class GeoMagLib_API CGeoMagnetic
{
public:

	CMagneticModel* MagneticModel;
	CMagneticModel* TimedMagneticModel;
	CGeoid* Geoid;
	CEllipsoid* Ellip;
	CCoordGeodetic* CoordGeodetic;
	CDate* UserDate;
	CCoordSpherical* CoordSpherical;
	CGeoMagneticElements* GeoMagneticElements;

	CGeoMagnetic();

	CGeoMagnetic(double sdate, const char projection);

	~CGeoMagnetic();

	void CalculateFieldElements(double latitude, double longitude, double alt, const char unit);
};

#endif