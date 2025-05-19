#ifndef FIMS_MODELS_GMACS_HPP
#define FIMS_MODELS_GMACS_HPP

#include "fishery_model_base.hpp"

namespace fims_popdy
{

    template <typename Type>
    class GMACS : public FisheryModelBase<Type>
    {
    public:
        size_t syr;     ///> initial year
        size_t nyr;     ///> terminal year
        size_t nseason; ///> time step (years)
        size_t nfleet;  ///> number of gears (including surveys and fisheries)
        size_t nsex;    ///> number of sexes

        size_t nshell;  ///> number of shell conditions
        size_t nmature; ///> number of maturity types
        size_t nclass;  ///> number of size-classes

        size_t season_recruitment; ///> Season that recruitment occurs (end of year before growth)
        size_t season_growth;      ///> Season that growth occurs (end of year after recruitment)
        size_t season_ssb;         ///> Season to calculate SSB (end of year)
        size_t season_N;           ///> Season to output N

        size_t n_grp; ///> number of sex/newshell/oldshell groups
        size_t nlikes;

        size_t nCatchDF;
        fims::Vector<int> nCatchRows;//(1, nCatchDF);
        fims::Vector<fims::Vector<fims::Vector<size_t> > > dCatchData;//(1, nCatchDF, 1, nCatchRows, 1, 11); // array of catch data
     
        fims::Vector<fims::Vector<Type> > obs_catch;//(1, nCatchDF, 1, nCatchRows);
        fims::Vector<fims::Vector<Type> > obs_effort;//(1, nCatchDF, 1, nCatchRows);
        fims::Vector<fims::Vector<fims::Vector<Type> > > dCatchData_out;//(1, nCatchDF, syr, nyr, 1, 11);
        fims::Vector<fims::Vector<Type> > obs_catch_out;//(1, nCatchDF, syr, nyr);

        fims::Vector<fims::Vector<Type> > catch_cv;//(1, nCatchDF, 1, nCatchRows);
        fims::Vector<fims::Vector<Type> > catch_dm;//(1, nCatchDF, 1, nCatchRows);
        fims::Vector<fims::Vector<Type> > catch_mult;//(1, nCatchDF, 1, nCatchRows);

        virtual void Initialize()
        {
        }

        virtual void Evaluate()
        {
        }
    };

}

#endif
