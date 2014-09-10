
#ifndef VLEROUTINES_H
#define VLEROUTINES_H

#include "HelmholtzEOSMixtureBackend.h"

namespace CoolProp{

namespace SaturationSolvers
{
    struct saturation_T_pure_Akasaka_options{
        bool use_guesses; ///< true to start off at the values specified by rhoL, rhoV
        long double omega, rhoL, rhoV, pL, pV;
        saturation_T_pure_Akasaka_options(){omega = _HUGE; rhoV = _HUGE; rhoL = _HUGE; pV = _HUGE, pL = _HUGE;}
    };
    struct saturation_T_pure_options{
        bool use_guesses; ///< true to start off at the values specified by rhoL, rhoV
        long double omega, rhoL, rhoV, pL, pV, p, T;
        saturation_T_pure_options(){omega = _HUGE; rhoV = _HUGE; rhoL = _HUGE; rhoL = _HUGE; pV = _HUGE, pL = _HUGE; T = _HUGE;}
    };
    
    struct saturation_D_pure_options{
        enum imposed_rho_options{IMPOSED_RHOL, IMPOSED_RHOV};
        bool use_guesses, ///< True to start off at the values specified by rhoL, rhoV, T
             use_logdelta; ///< True to use partials with respect to log(delta) rather than delta
        long double omega, rhoL, rhoV, pL, pV;
        int imposed_rho;
        saturation_D_pure_options(){ use_logdelta = true; omega = 1.0;} // Defaults
    };

    enum sstype_enum {imposed_T, imposed_p};
    struct mixture_VLE_IO
    {
        int sstype, Nstep_max;
        long double rhomolar_liq, rhomolar_vap, p, T, beta;
        std::vector<long double> x, y, K;
    };

    /*! Returns the natural logarithm of K for component i using the method from Wilson as in
	\f[
	\ln K_i = \ln\left(\frac{p_{c,i}}{p}\right)+5.373(1+\omega_i)\left(1-\frac{T_{c,i}}{T}\right)
	\f]
    @param HEOS The Helmholtz EOS mixture backend
	@param T Temperature [K]
	@param p Pressure [Pa]
	@param i Index of component [-]
	*/
	static long double Wilson_lnK_factor(HelmholtzEOSMixtureBackend &HEOS, long double T, long double p, int i){ 
        EquationOfState *EOS = (HEOS.get_components())[i]->pEOS; 
        return log(EOS->reduce.p/p)+5.373*(1 + EOS->accentric)*(1-EOS->reduce.T/T);
    };

    void saturation_D_pure(HelmholtzEOSMixtureBackend &HEOS, long double rhomolar, saturation_D_pure_options &options);
    void saturation_T_pure(HelmholtzEOSMixtureBackend &HEOS, long double T, saturation_T_pure_options &options);
    void saturation_T_pure_Akasaka(HelmholtzEOSMixtureBackend &HEOS, long double T, saturation_T_pure_Akasaka_options &options);
    
    /**
    */
    struct saturation_PHSU_pure_options{
        enum specified_variable_options{IMPOSED_HL, IMPOSED_HV, IMPOSED_PL, IMPOSED_PV, IMPOSED_SL, IMPOSED_SV, IMPOSED_UL, IMPOSED_UV, IMPOSED_INVALID_INPUT};
        bool use_guesses, ///< True to start off at the values specified by rhoL, rhoV, T
             use_logdelta; ///< True to use partials with respect to log(delta) rather than delta
        specified_variable_options specified_variable;
        long double omega, rhoL, rhoV, pL, pV, T, p;
        saturation_PHSU_pure_options(){ specified_variable = IMPOSED_INVALID_INPUT; use_guesses = true; omega = 1.0; }
    };
    /**

    */
    void saturation_PHSU_pure(HelmholtzEOSMixtureBackend &HEOS, long double specified_value, saturation_PHSU_pure_options &options);

    /* \brief This is a backup saturation_p solver for the case where the Newton solver cannot approach closely enough the solution
     *
     * This is especially a problem at low pressures where catastrophic truncation error occurs, especially in the saturated vapor side
     * 
     * @param HEOS The Helmholtz EOS backend instance to be used
     * @param p Imposed pressure in kPa
     * @param options Options to be passed to the function (at least T, rhoL and rhoV must be provided)
     */
    void saturation_P_pure_1D_T(HelmholtzEOSMixtureBackend &HEOS, long double p, saturation_PHSU_pure_options &options);
    
    /* \brief This is a backup saturation_T solver for the case where the Newton solver cannot approach closely enough the solution
     *
     * This is especially a problem at low pressures where catastrophic truncation error occurs, especially in the saturated vapor side
     * 
     * @param HEOS The Helmholtz EOS backend instance to be used
     * @param T Imposed temperature in K
     * @param options Options to be passed to the function (at least p, rhoL and rhoV must be provided)
     */
    void saturation_T_pure_1D_P(HelmholtzEOSMixtureBackend &HEOS, long double T, saturation_T_pure_options &options);

    /* \brief A robust but slow solver in the very-near-critical region
     * 
     * This solver operates in the following fashion:
     * 1. Using a bounded interval for rho'':[rhoc, rhoc-??], guess a value for rho''
     * 2. For guessed value of rho'' and given value of T, calculate p
     * 3. Using a Brent solver on the other co-existing phase (rho'), calculate the (bounded) value of rho' that yields the same pressure
     * 4. Use another outer Brent solver on rho'' to enforce the same Gibbs function between liquid and vapor
     * 5. Fin.
     * 
     * @param HEOS The Helmholtz EOS backend instance to be used
     * @param ykey The CoolProp::parameters key to be imposed - one of iT or iP
     * @param y The value for the imposed variable
     */
    void saturation_critical(HelmholtzEOSMixtureBackend &HEOS, CoolProp::parameters ykey, long double y);
        
    void successive_substitution(HelmholtzEOSMixtureBackend &HEOS,
                                        const long double beta,
                                        long double T,
                                        long double p,
                                        const std::vector<long double> &z,
                                        std::vector<long double> &K,
                                        mixture_VLE_IO &options);
    /** \brief Extract the mole fractions of liquid (x) and vapor (y) given the bulk composition (z), vapor mole fraction and K-factors
     * @param beta Vapor molar fraction [-]
     * @param K K-factors for the components [-]
     * @param z Bulk molar composition [-]
     * @param x Liquid molar composition [-]
     * @param y Vapor molar composition [-]
     */
    void x_and_y_from_K(long double beta, const std::vector<long double> &K, const std::vector<long double> &z, std::vector<long double> &x, std::vector<long double> &y);

    /*! A wrapper function around the residual to find the initial guess for the bubble point temperature
    \f[
    r = \sum_i \frac{z_i(K_i-1)}{1-beta+beta*K_i}
    \f]
    */
    class WilsonK_resid : public FuncWrapper1D
    {
    public:
        int input_type;
	    double T, p, beta;
	    const std::vector<long double> *z;
        std::vector<long double> *K;
	    HelmholtzEOSMixtureBackend *HEOS;

	    WilsonK_resid(HelmholtzEOSMixtureBackend &HEOS, double beta, double imposed_value, int input_type, const std::vector<long double> &z, std::vector<long double> &K){ 
            this->z = &z; this->K = &K; this->HEOS = &HEOS; this->beta = beta; this->input_type = input_type;
            if (input_type == imposed_T){
                this->T = imposed_value;
            }
            else{
                this->p = imposed_value;
            }
	    };
	    double call(double input_value){
		    double summer = 0;
            if (input_type == imposed_T){
                p = input_value; // Iterate on pressure
            }
            else{
                T = input_value; // Iterate on temperature, pressure imposed
            }
		    for (unsigned int i = 0; i< (*z).size(); i++) {
			    (*K)[i] = exp(Wilson_lnK_factor(*HEOS,T,p,i));
			    summer += (*z)[i]*((*K)[i]-1)/(1-beta+beta*(*K)[i]);
		    }
		    return summer;
	    };
    };
    inline double saturation_preconditioner(HelmholtzEOSMixtureBackend &HEOS, double input_value, int input_type, const std::vector<long double> &z)
    {
	    double ptriple = 0, pcrit = 0, Ttriple = 0, Tcrit = 0;
	    
	    for (unsigned int i = 0; i < z.size(); i++)
	    {
            EquationOfState *EOS = (HEOS.get_components())[i]->pEOS; 

		    ptriple += EOS->sat_min_liquid.p*z[i];
            pcrit += EOS->reduce.p*z[i];
		    Ttriple += EOS->sat_min_liquid.T*z[i];
		    Tcrit += EOS->reduce.T*z[i];
	    }

        if (input_type == imposed_T)
        {
            return exp(log(pcrit/ptriple)/(Tcrit-Ttriple)*(input_value-Ttriple)+log(ptriple));
        }
        else if (input_type == imposed_p)
        {
            return 1/(1/Tcrit-(1/Ttriple-1/Tcrit)/log(pcrit/ptriple)*log(input_value/pcrit));
        }
        else{ throw ValueError();}
    }
    inline double saturation_Wilson(HelmholtzEOSMixtureBackend &HEOS, double beta, double input_value, int input_type, const std::vector<long double> &z, double guess)
    {
	    double T;

	    std::string errstr;

	    // Find first guess for T using Wilson K-factors
        WilsonK_resid Resid(HEOS, beta, input_value, input_type, z, HEOS.get_K());
	    T = Secant(Resid, guess, 0.001, 1e-10, 100, errstr);
	
	    if (!ValidNumber(T)){throw ValueError("saturation_p_Wilson failed to get good T");}
	    return T;
    }
    struct SuccessiveSubstitutionStep
    {
        long double T,p;
    };

    struct newton_raphson_saturation_options{
        enum imposed_variable_options {IMPOSED_P, IMPOSED_T};
        int Nstep_max;
        bool bubble_point;
        std::size_t Nsteps;
        long double omega, rhomolar_liq, rhomolar_vap, pL, pV, p, T, hmolar_liq, hmolar_vap, smolar_liq, smolar_vap;
        imposed_variable_options imposed_variable;
        std::vector<long double> x, y;
        newton_raphson_saturation_options(){ Nsteps = 0;} // Defaults
    };

    /** \brief A class to do newton raphson solver for VLE given guess values for vapor-liquid equilibria.  This class will then be included in the Mixture class
     * 
     * A class is used rather than a function so that it is easier to store iteration histories, additional output values, etc.
     * 
     * This class only handles bubble and dew lines since the independent variables are N-1 of the mole fractions in the incipient phase along with one of T, p, or rho
     */
    class newton_raphson_saturation
    {
    public:
	    long double error_rms, rhomolar_liq, rhomolar_vap, T, p, max_rel_change, min_abs_change;
	    unsigned int N;
	    bool logging;
        bool bubble_point;
	    int Nsteps;
        long double dTsat_dPsat, dPsat_dTsat;
	    STLMatrix J;
        HelmholtzEOSMixtureBackend *HEOS;
	    std::vector<long double> K, x, y, phi_ij_liq, phi_ij_vap, dlnphi_drho_liq, dlnphi_drho_vap, r, negative_r, dXdS, neg_dFdS;
	    std::vector<SuccessiveSubstitutionStep> step_logger;

	    newton_raphson_saturation(){};

	    void resize(unsigned int N);
	
	    // Reset the state of all the internal variables
	    void pre_call()
	    {
		    K.clear(); x.clear(); y.clear();  phi_ij_liq.clear(); 
            phi_ij_vap.clear(); dlnphi_drho_liq.clear(), dlnphi_drho_vap.clear(),
            step_logger.clear(); error_rms = 1e99; Nsteps = 0;
		    rhomolar_liq = _HUGE; rhomolar_vap = _HUGE; T = _HUGE; p = _HUGE;
	    };

	    /** Call the Newton-Raphson VLE Solver
         *
	     * This solver must be passed reasonable guess values for the mole fractions, 
	     * densities, etc.  You may want to take a few steps of successive substitution
	     * before you start with Newton Raphson.
         * 
	     * @param HEOS HelmholtzEOSMixtureBackend instance
	     * @param z Bulk mole fractions [-]
	     * @param z_incipient Initial guesses for the mole fractions of the incipient phase [-]
         * @param IO The input/output data structure
	     */
	    void call(HelmholtzEOSMixtureBackend &HEOS, const std::vector<long double> &z, std::vector<long double> &z_incipient, newton_raphson_saturation_options &IO);

	    /*! Build the arrays for the Newton-Raphson solve

	    This method builds the Jacobian matrix, the sensitivity matrix, etc.
         * 
	    */
	    void build_arrays();

        /** Check the derivatives in the Jacobian using numerical derivatives.
        */
        void check_Jacobian();
    };
};
    
} /* namespace CoolProp*/

#endif
