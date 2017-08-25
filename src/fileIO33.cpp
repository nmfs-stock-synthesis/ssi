
#include "fileIO33.h"
#include "model.h"
#include "fleet.h"
#include "fileIOgeneral.h"
#include "message.h"
//#include "ss_math.h"
#include "block_pattern.h"

bool read33_dataFile(ss_file *d_file, ss_model *data)
{
    //  SS_Label_Info_2.0 #READ DATA FILE
    QString token;
    QString temp_str;
    QStringList str_lst;
    float temp_float;
    int temp_int = 0, num_input_lines = 0;
    int i, num_vals, total_fleets;
    int n_areas = 0, n_ages = 0, n_genders = 0;
    int units, err_type, year, season, fleet, obslength;
    float obs, err;
//    float month;

    if(d_file->open(QIODevice::ReadOnly))
    {
        //  SS_Label_Info_2.1.1 #Read comments
        d_file->seek(0);
        d_file->read_comments();

        //  SS_Label_Info_2.1.2 #Read model time dimensions
        token = d_file->get_next_value("start year");
        temp_int = token.toInt();
        data->set_start_year (temp_int);
        token = d_file->get_next_value("end year");
        temp_int = token.toInt();
        data->set_end_year(temp_int);
        token = d_file->get_next_value("seasons per year");
        temp_int = token.toInt();
        data->set_num_seasons(temp_int);
        for (i = 1; i <= data->get_num_seasons(); i++)
        {
            token = d_file->get_next_value("months per season");
            temp_float = token.toFloat();
            data->set_months_per_season(i, temp_float);
        }
        //  SS_Label_Info_2.1.3 #Set up seasons
        data->rescale_months_per_season();
        token = d_file->get_next_value("subseasons");
        temp_int = token.toInt();
        data->set_num_subseasons(temp_int);
        token = d_file->get_next_value("spawning month");
        temp_float = token.toFloat();
        data->set_spawn_month(temp_float);
        token = d_file->get_next_value("number of genders");
        temp_int = token.toInt();
        n_genders = temp_int;
        data->set_num_genders(n_genders);
        token = d_file->get_next_value("number of ages");
        temp_int = token.toInt();
        n_ages = temp_int;
        data->set_num_ages(n_ages);
        token = d_file->get_next_value("number of areas");
        temp_int = token.toInt();
        n_areas = temp_int;
        data->set_num_areas(n_areas);
        data->getPopulation()->Move()->setNumAreas(n_areas);
        //  SS_Label_Info_2.1.5  #Define fleets, surveys and areas
        token = d_file->get_next_value("number of fleets");
        temp_int = token.toInt();
        total_fleets = temp_int;
        data->set_num_fleets(total_fleets);
        for (i = 0; i < total_fleets; i++)
        {
            Fleet *flt = data->getFleet(i);
            flt->reset();
            flt->setActive(true);
            flt->getSizeSelectivity()->setNumAges(data->get_num_ages());
            flt->getAgeSelectivity()->setNumAges (data->get_num_ages());
            temp_int = d_file->get_next_value("fleet type").toInt();
            flt->setTypeInt(temp_int);
            temp_float = d_file->get_next_value("timing").toFloat();
            flt->setSeasTiming(temp_float);
            temp_int = d_file->get_next_value("area").toInt();
            flt->setArea(temp_int);
            temp_int = d_file->get_next_value("catch units").toInt();
            flt->setCatchUnits(temp_int);
/*            temp_float = d_file->next_value("equ_catch_se").toFloat();
            flt->set_equ_catch_se(temp_float);
            temp_float = d_file->next_value("catch_se").toFloat();
            flt->set_catch_se(temp_float);*/
            temp_int = d_file->get_next_value("use_catch_mult").toInt();
            flt->setCatchMultiplier(temp_int);
            temp_str = d_file->get_next_value("fleet name");
            flt->setName(temp_str);
            flt->setNumGenders(data->get_num_genders());
            flt->setNumSeasons(data->get_num_seasons());
            flt->setStartYear(data->get_start_year());
            flt->setTotalYears(data->getTotalYears());
        }
        data->assignFleetNumbers();

        //  ProgLabel_2.2  Read CATCH amount by fleet
        // Catch
        do {
            float ctch, ctch_se;
            str_lst.clear();
            str_lst.append(d_file->get_next_value("year"));
            str_lst.append(d_file->get_next_value("season"));
            str_lst.append(d_file->get_next_value("fleet"));
            str_lst.append(d_file->get_next_value("catch"));
            str_lst.append(d_file->get_next_value("catch_se"));
            year   = str_lst.at(0).toInt();
            fleet  = abs(str_lst.at(2).toInt()) - 1;
            ctch   = str_lst.at(3).toFloat();
/*            season = d_file->get_next_value("season").toInt();
            fleet = d_file->get_next_value("fleet").toInt();
            ctch = d_file->get_next_value("catch").toFloat();
            ctch_se = d_file->get_next_value("catch_se").toFloat();*/
            if (year == -999)
                data->getFleet(fleet)->set_catch_equil(ctch);
            if (year != -9999)
                data->getFleet(fleet)->addCatchObservation(str_lst);
//                data->getFleet(fleet-1)->add_catch_per_season(year, season, ctch, ctch_se);
        } while (year != -9999);

        //  SS_Label_Info_2.3 #Read fishery CPUE, effort, and Survey index or abundance
        // before we record abundance, get units and error type for all fleets
        for (i = 0; i < total_fleets; i++)
        {
            Fleet *flt = data->getFleet(i);
            fleet = abs(d_file->get_next_value("Abund fleet num").toInt()); // fleet number
            if (fleet != (i + 1))
                d_file->error(QString("Fleet number does not match."));
            units = d_file->get_next_value("Abund units").toInt(); // units
            flt->setAbundUnits(units);
            err_type = d_file->get_next_value("Abund err-type").toInt(); // err_flt->setAbundErrType(err_type);
            flt->setAbundErrType(err_type);
            temp_int = d_file->get_next_value("Abund sd ").toInt();
            flt->setSDReport(temp_int);
        }
        // here are the abundance numbers
        do
        {    // year, month, fleet_number, observation, error
            str_lst.clear();
            str_lst.append(d_file->get_next_value("Abund year"));
            str_lst.append(d_file->get_next_value("Abund month"));
            str_lst.append(d_file->get_next_value("Abund fleet"));
            str_lst.append(d_file->get_next_value("Abund obs"));
            str_lst.append(d_file->get_next_value("Abund err"));
            year = str_lst.at(0).toInt();
            fleet = abs(str_lst.at(2).toInt()) - 1;
/*            year = d_file->get_next_value().toInt();
            month = d_file->get_next_value().toFloat();
            fleet = abs(d_file->get_next_value().toInt()) - 1;
            obs = d_file->get_next_value().toFloat();
            err = d_file->get_next_value().toFloat();*/
            if (year != -9999)
                data->getFleet(fleet)->addAbundanceObs(str_lst);
//                data->getFleet(fleet)->addAbundByMonth(year, month, obs, err);
        } while (year != -9999);

        //  SS_Label_Info_2.4 #read Discard data
        token = d_file->get_next_value("num fleets with discard");
        num_vals = token.toInt();
        if (num_vals > 0)
        {
            for (i = 0; i < num_vals; i++)
            {
                fleet = abs(d_file->get_next_value("discard fleet").toInt()) - 1;
                units = d_file->get_next_value("discard units").toInt();
                err_type = d_file->get_next_value("discard err-type").toInt();
                data->getFleet(fleet)->setDiscardUnits(units);
                data->getFleet(fleet)->setDiscardErrType(err_type);
            }
            // observations
            do
            {    // year, month, fleet_number, observation, error
                str_lst.clear();
                str_lst.append(d_file->get_next_value("Discard year"));
                str_lst.append(d_file->get_next_value("Discard month"));
                str_lst.append(d_file->get_next_value("Discard fleet"));
                str_lst.append(d_file->get_next_value("Discard obs"));
                str_lst.append(d_file->get_next_value("Discard err"));
                year = str_lst.at(0).toInt();
                fleet = abs(str_lst.at(2).toInt()) - 1;
/*                year = d_file->get_next_value().toInt();
                month = d_file->get_next_value().toFloat();
                fleet = abs(d_file->get_next_value().toInt()) - 1;
                obs = d_file->get_next_value().toFloat();
                err = d_file->get_next_value().toFloat();*/
                if (year != -9999)
                    data->getFleet(fleet)->addDiscard(str_lst);
//                    data->getFleet(fleet)->setDiscardMonth(year, month, obs, err);
                if (str_lst.at(4).compare("EOF") == 0)
                    return false;
            } while (year != -9999);
        }

        //  SS_Label_Info_2.5 #Read Mean Body Weight data
        //  note that syntax for storing this info internal is done differently than for surveys and discard
        temp_int = d_file->get_next_value("use mean bwt").toInt();
        data->setUseMeanBwt(temp_int);
        if (temp_int > 0)
        {
            temp_int = d_file->get_next_value("mean bwt df").toInt();
            for (i = 0; i < data->get_num_fleets(); i++)
                data->getFleet(i)->setMbwtDF(temp_int);
            do
            {    // year, month, fleet_number, partition, obs, stderr
                str_lst.clear();
                for (int j = 0; j < 6; j++)
                    str_lst.append(d_file->get_next_value(QString("mean bwt")));
                year = str_lst.at(0).toInt();
                fleet = abs(str_lst.at(2).toInt()) - 1;
                if (year != -9999)
                    data->getFleet(fleet)->addMbwtObservation(str_lst);
                if (str_lst.at(4).compare("EOF") == 0)
                    return false;
            } while (year != -9999);
        }

        //  SS_Label_Info_2.6 #Setup population Length bins
        {
        compositionLength *l_data = data->get_length_composition();
        if (l_data == NULL)
            l_data = new compositionLength();
        data->set_length_composition(l_data);
        temp_int = d_file->get_next_value("Length comp alt bin method").toInt();
        l_data->setAltBinMethod(temp_int);
        switch (temp_int)
        {
        case 1:  // same as data bins - set after data bins
            break;
        case 2:  // generate from min, max, width
            float min, max, width;
            token = d_file->get_next_value("Length comp bin width");
            width = token.toFloat();
            l_data->setAltBinWidth(width);
            token = d_file->get_next_value("Length comp alt bin min");
            min = token.toFloat();
            l_data->setAltBinMin(min);
            token = d_file->get_next_value("Length comp alt bin max");
            max = token.toFloat();
            l_data->setAltBinMax(max);
            l_data->generateAltBins();
            break;
        case 3:  // read vector
            str_lst.clear();
            temp_int = d_file->get_next_value("Length comp num alt bins").toInt();
            l_data->setNumberAltBins(temp_int);
            for (int j = 0; j < temp_int; j++)
                str_lst.append(d_file->get_next_value("Length comp alt bin"));
            l_data->setAltBins(str_lst);
            break;
        }
        //  SS_Label_Info_2.7 #Start length data section
        temp_int = d_file->get_next_value("use length comp?").toInt();
        data->setUseLengthComp(temp_int);
        if (temp_int == 1)
        {
            for (i = 0; i < total_fleets; i++)
            {
                data->getFleet(i)->setLengthMinTailComp(d_file->get_next_value("min tail comp"));
                data->getFleet(i)->setLengthAddToData(d_file->get_next_value("add to data"));
                temp_int = d_file->get_next_value("combine genders").toInt();
                data->getFleet(i)->setLengthCombineGen(temp_int);
                temp_int = d_file->get_next_value("compress bins").toInt();
                data->getFleet(i)->setLengthCompressBins(temp_int);
                temp_int = d_file->get_next_value("error").toInt();
                data->getFleet(i)->setLengthCompError(temp_int);
                temp_int = d_file->get_next_value("error parameter").toInt();
                data->getFleet(i)->setLengthCompErrorParm(temp_int);
                temp_int = d_file->get_next_value("min sample size").toInt();
                data->getFleet(i)->setLengthMinSampleSize(temp_int);
            }
            temp_int = d_file->get_next_value("Length comp number bins").toInt();//token.toInt();
            l_data->setNumberBins(temp_int);
            for (int j = 0; j < data->get_num_fleets(); j++)
                data->getFleet(j)->setLengthNumBins(temp_int);
            str_lst.clear();
            for (i = 0; i < temp_int; i++)
            {
                str_lst.append(d_file->get_next_value("Length comp bin"));
            }
            l_data->setBins(str_lst);
            if (l_data->getAltBinMethod() == 1) // set alt bins if method = 1
            {
                l_data->setNumberAltBins(l_data->getNumberBins());
                l_data->setAltBins(l_data->getBins());
            }

            //  SS_Label_Info_2.7.4 #Read Length composition data
            obslength = data->getFleet(0)->getLengthObsLength();
            do
            {
                str_lst.clear();
                for (int j = 0; j < obslength; j++)
                {
                    token = d_file->get_next_value(QString("Length comp data"));
                    str_lst.append(token);
                }
                if (str_lst.at(0).toInt() == -9999)
                    break;
                if (str_lst.at(0).compare("EOF") == 0)
                    return false;
                temp_int = abs(str_lst.at(2).toInt());
                data->getFleet(temp_int - 1)->addLengthObservation(str_lst);// getLengthObs.addObservation(data);
            } while (str_lst.at(0).toInt() != -9999);
            data->set_length_composition(l_data);
        }
        }

        //  SS_Label_Info_2.8 #Start age composition data section
        {
        compositionAge *a_data = data->get_age_composition();
        if (a_data == NULL)
            a_data = new compositionAge ();
        token = d_file->get_next_value("age comp num bins");
        temp_int = token.toInt();
        a_data->setNumberBins(temp_int);
        for (i = 0; i < data->get_num_fleets(); i++)
        {
            data->getFleet(i)->setAgeNumBins(temp_int);
            data->getFleet(i)->setSaaNumBins(temp_int);
        }
        str_lst.clear();
        for (i = 0; i < temp_int; i++)
        {
            token = d_file->get_next_value("age comp bin");
            str_lst.append(token);
        }
        a_data->setBins(str_lst);
        token = d_file->get_next_value("age comp num error defs");
        temp_int = token.toInt();
        a_data->set_num_error_defs(temp_int);
        for (i = 0; i < temp_int; i++)
        {
            int numAges = data->get_num_ages();
            str_lst.clear();
            for (int j = 0; j <= numAges; j++)
                str_lst.append(d_file->get_next_value());
            a_data->set_error_def_ages(i, str_lst);
            str_lst.clear();
            for (int j = 0; j <= numAges; j++)
                str_lst.append(d_file->get_next_value());
            a_data->set_error_def(i, str_lst);
        }
        for (i = 0; i < total_fleets; i++)
        {
            data->getFleet(i)->setAgeMinTailComp(d_file->get_next_value("min tail comp"));
            data->getFleet(i)->setAgeAddToData(d_file->get_next_value("add to data"));
            temp_int = d_file->get_next_value("combine genders").toInt();
            data->getFleet(i)->setAgeCombineGen(temp_int);
            temp_int = d_file->get_next_value("compress bins").toInt();
            data->getFleet(i)->setAgeCompressBins(temp_int);
            temp_int = d_file->get_next_value("error").toInt();
            data->getFleet(i)->setAgeCompError(temp_int);
            temp_int = d_file->get_next_value("error parm").toInt();
            data->getFleet(i)->setAgeCompErrorParm(temp_int);
            temp_int = d_file->get_next_value("min sample size").toInt();
            data->getFleet(i)->setAgeCompMinSampleSize(temp_int);
        }

        token = d_file->get_next_value("Age comp alt bin method");
        a_data->setAltBinMethod(token.toInt());

        //  SS_Label_Info_2.8.2 #Read Age data
        obslength = data->getFleet(0)->getAgeObsLength();
        do
        {
            str_lst.clear();
            for (int j = 0; j < obslength; j++)
            {
                token = d_file->get_next_value(QString("Age data"));
                str_lst.append(token);
                if (token.contains("-9999"))
                {
                    d_file->skip_line();
                    break;
                }
            }
            if (str_lst.at(0).toInt() == -9999)
                break;
            if (str_lst.at(0).compare("EOF") == 0)
                return false;
            fleet = abs(str_lst.at(2).toInt());
            data->getFleet(fleet - 1)->addAgeObservation(str_lst);
        } while (str_lst.at(0).toInt() != -9999);

        //  SS_Label_Info_2.9 #Read mean Size_at_Age data
        temp_int = d_file->get_next_value().toInt();
        data->setUseMeanSAA(temp_int);
        if (temp_int > 0)
        {
            obslength = data->getFleet(0)->getSaaObservation(0).count();
            do
            {
                str_lst.clear();
                for (int j = 0; j < obslength; j++)
                {
                    token = d_file->get_next_value(QString("mean Size_at_Age"));
                    str_lst.append(token);
                }
                if (str_lst.at(0).toInt() == -9999)
                    break;
                if (str_lst.at(0).compare("EOF") == 0)
                    return false;
                fleet = abs(str_lst.at(2).toInt());
                data->getFleet(fleet - 1)->addSaaObservation(str_lst);
            } while (str_lst.at(0).toInt() != -9999);
        }
        }

        //  SS_Label_Info_2.10 #Read environmental data that will be used to modify processes and expected values
        temp_int = d_file->get_next_value("num env vars").toInt();
        data->setNumEnvironVars (temp_int);
        data->setNumEnvironVarObs(0);
        if (temp_int > 0)
        {
            obslength = data->getEnvVariables()->columnCount();
            do
            {
                str_lst.clear();
                for(int j = 0; j < obslength; j++)
                {
                    str_lst.append(d_file->get_next_value(QString("env var data")));
                }
                temp_int = str_lst.at(0).toInt();
                if (temp_int != -9999)
                    data->addEnvironVarObs (str_lst);
                if (str_lst.at(0).compare("EOF") == 0)
                    return false;
            } while (temp_int != -9999);
        }

        //  SS_Label_Info_2.11 #Start generalized size composition section
        num_vals = d_file->get_next_value().toInt();
        if (num_vals > 0)
        {
            for (i = 0; i < num_vals; i++)
            {
                compositionGeneral *cps = new compositionGeneral ();
                data->addGeneralCompMethod(cps);
                data->getGeneralCompMethod(i)->setNumber(i+1);
            }
            for (int j = 0; j < total_fleets; j++)
                data->getFleet(j)->setGenModelTotal(num_vals);
            for (i = 0; i < num_vals; i++)
            {
                temp_int = d_file->get_next_value().toInt();
                data->getGeneralCompMethod(i)->setNumberBins(temp_int);
                for (int j = 0; j < total_fleets; j++)
                    data->getFleet(j)->setGenNumBins(i, temp_int);
            }
            for (i = 0; i < num_vals; i++)
            {
                temp_int = d_file->get_next_value().toInt();
                data->getGeneralCompMethod(i)->setUnits(temp_int);
            }
            for (i = 0; i < num_vals; i++)
            {
                temp_int = d_file->get_next_value().toInt();
                data->getGeneralCompMethod(i)->setScale(temp_int);
            }
            for (i = 0; i < num_vals; i++)
            {
                temp_float = d_file->get_next_value().toFloat();
//                for (int j = 0; j < total_fleets; j++)
//                    data->getFleet(j)->setGenAddToData(i, temp_str);
                data->getGeneralCompMethod(i)->setMinComp(temp_float);
            }
            for (i = 0; i < num_vals; i++)
            {
                temp_int = d_file->get_next_value().toInt();
                data->getGeneralCompMethod(i)->setNumberObs(temp_int);
            }
            for (i = 0; i < num_vals; i++)
            {
                compositionGeneral *cps = data->getGeneralCompMethod(i);
                str_lst.clear();
                for (int j = 0; j < cps->getNumberBins(); j++)
                {
                    str_lst.append(d_file->get_next_value());
                }
                cps->getBinsModel()->setRowData(0, str_lst);
            }
            num_input_lines = 0;
            for (i = 0; i < num_vals; i++)
            {
                compositionGeneral *cps = data->getGeneralCompMethod(i);
                num_input_lines += cps->getNumberObs();
            }
            for (i = 0; i < num_input_lines; i++)
            {
                str_lst.clear();
                str_lst.append(d_file->get_next_value());
                temp_int = str_lst.at(0).toInt();
                obslength = data->getFleet(0)->getGenObsLength(temp_int-1) - 1;

                for (int k = 0; k < obslength; k++)
                {
                    str_lst.append(d_file->get_next_value());
                }

                fleet = abs(str_lst.at(3).toInt());
                data->getFleet(fleet-1)->addGenObservation(temp_int-1, str_lst);
            }
        }

        //  SS_Label_Info_2.12 #Read tag release and recapture data
        temp_int = d_file->get_next_value().toInt();
        data->set_do_tags(temp_int == 1);
        if (temp_int == 1)
        {
            temp_int = d_file->get_next_value().toInt();
            data->set_num_tag_groups(temp_int);
            num_input_lines = d_file->get_next_value().toInt();
            temp_int = d_file->get_next_value().toInt();
            data->set_tag_latency(temp_int);
            temp_int = d_file->get_next_value().toInt();
            data->set_tag_max_periods(temp_int);
            // release data
            for (i = 0; i < data->get_num_tag_groups(); i++)
            {
                str_lst.clear();
                for (int j = 0; j < 8; j++)
                    str_lst.append(d_file->get_next_value());
                data->set_tag_observation(i, str_lst);
            }
            // recapture data
            for (i = 0; i < num_input_lines; i++)
            {
                str_lst.clear();
                for (int j = 0; j < 5; j++)
                    str_lst.append(d_file->get_next_value());
                temp_int = abs(str_lst.at(3).toInt());
                data->getFleet(temp_int - 1)->addRecapObservation(str_lst);
            }
        }

        //  SS_Label_Info_2.13 #Morph composition data
        temp_int = d_file->get_next_value().toInt();
        data->setDoMorphComp(temp_int == 1);
        compositionMorph *mcps = new compositionMorph();
        data->set_morph_composition (mcps);
        if (data->getDoMorphComp())
        {
            num_input_lines = d_file->get_next_value().toInt(); // num observations
            mcps->setNumberObs(num_input_lines);
            temp_int = d_file->get_next_value().toInt(); // number of platoons
//            mcps->setNumberMorphs(temp_int);
            for (int j = 0; j < data->get_num_fleets(); j++)
                data->getFleet(j)->setMorphNumMorphs(temp_int);
            temp_str = d_file->get_next_value();         // min compression
            for (int j = 0; j < data->get_num_fleets(); j++)
                data->getFleet(j)->setMorphMinTailComp(temp_str);

            obslength = data->getFleet(0)->getMorphObsLength() + 1;
            for (i = 0; i < num_input_lines; i++)
            {
                str_lst.clear();
                for (int j = 0; j < obslength; j++)
                {
                    str_lst.append(d_file->get_next_value());
                }
                temp_int = abs(str_lst.at(2).toInt());
                data->getFleet(temp_int - 1)->addMorphObservation(str_lst);
            }
        }
        temp_int = d_file->get_next_value().toInt();
        data->setReadSelectivityPriors(temp_int == 1? true: false);

        //  SS_Label_Info_2.14 #End of datafile indicator
        temp_int = d_file->get_next_value().toInt();
        if (temp_int != END_OF_DATA)
        {
            d_file->error(QString("Found incorrect end of data marker."));
        }

        d_file->close();
        return true;
    }
    else
    {
        d_file->error(QString("Data file does not exist or is unreadable."));
    }
    return false;
}

int write33_dataFile(ss_file *d_file, ss_model *data)
{
    QString temp_str, line;
    QStringList str_lst, tmp_lst;
    int i, j, chars = 0;
    int temp_int = 0, num, num_lines;
//    float temp_float = 0.0;
    int total_fleets = data->getNumActiveFleets();
    Fleet *flt;

    if(d_file->open(QIODevice::WriteOnly))
    {
//        chars += d_file->writeline(QString("#V%1").arg(getDatafileVersionString()));
        chars += writeVersionComment(d_file);
        chars += d_file->write_comments();// (writeDatafileComment().toUtf8());

        line = QString (QString ("#_observed data: "));
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_styr").arg
                        (QString::number(data->get_start_year())));
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_endyr").arg
                        (QString::number(data->get_end_year())));
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_nseas").arg
                        (QString::number(data->get_num_seasons())));
        chars += d_file->writeline (line);

        line.clear();
        for (i = 0; i < data->get_num_seasons(); i++)
            line.append (QString(" %1").arg
                         (QString::number(data->get_months_per_season(i))));
        line.append (" #_months/season");
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_N_subseasons(even number, minimum is 2)").arg
                        (QString::number(data->get_num_subseasons())));
        chars += d_file->writeline (line);

        line = QString (QString ("%1 #_spawn_month").arg
                            (QString::number(data->get_spawn_month())));
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_Ngenders").arg
                        (QString::number(data->get_num_genders())));
        chars += d_file->writeline (line);

        line = QString (QString ("%1 #_Nages=accumulator age").arg
                        (QString::number(data->get_num_ages())));
        chars += d_file->writeline (line);

        line = QString (QString ("%1 #_N_areas").arg
                        (QString::number(data->get_num_areas())));
        chars += d_file->writeline (line);
        line = QString (QString ("%1 #_Nfleets (including surveys)").arg
                        (QString::number(data->get_num_fleets())));
        chars += d_file->writeline (line);
        line = QString ("#_fleet_type: 1=catch fleet; 2=bycatch only fleet; 3=survey; 4=ignore ");
        chars += d_file->writeline (line);
        line = QString ("#_survey_timing: -1=for use of catch-at-age to override the month value associated with a datum ");
        chars += d_file->writeline (line);
        line = QString ("#_fleet_area:  area the fleet/survey operates in ");
        chars += d_file->writeline (line);
        line = QString ("#_units of catch:  1=bio; 2=num (ignored for surveys; their units read later)");
        chars += d_file->writeline (line);
        line = QString ("#_catch_mult: 0=no; 1=yes");
        chars += d_file->writeline (line);
        line = QString ("#_rows are fleets");
        chars += d_file->writeline (line);
        line = QString ("#_fleet_type, timing, area, units, need_catch_mult fleetname");
        chars += d_file->writeline (line);

        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            str_lst.clear();
            str_lst.append(QString::number(flt->getTypeInt()));
            str_lst.append(QString::number(flt->getSeasTiming()));
            str_lst.append(QString::number(flt->getArea()));
            str_lst.append(QString::number(flt->getCatchUnits()));
            str_lst.append(QString::number(flt->getCatchMultiplier()));
            str_lst.append(flt->getName());
            chars += d_file->write_vector(str_lst, 5, QString::number(i));
/*            line.clear();
            line.append(QString(" %1").arg(QString::number(flt->getTypeInt())));
            line.append(QString(" %1").arg(QString::number(flt->getSeasTiming())));
            line.append(QString(" %1").arg(QString::number(flt->getArea())));
            line.append(QString(" %1").arg(QString::number(flt->getCatchUnits())));
//            line.append(QString(" %1").arg(QString::number(flt->equ_catch_se())));
//            line.append(QString(" %1").arg(QString::number(flt->catch_se())));
            line.append(QString(" %1").arg(QString::number(flt->getCatchMultiplier())));
            line.append(QString(" %1").arg(flt->getName()));

            line.append(QString("  # %1").arg(QString::number(i)));
            chars += d_file->writeline (line);*/
        }


        line = QString("#_Catch data: yr, seas, fleet, catch, catch_se");
        chars += d_file->writeline (line);
        line = QString("#_catch_se:  standard error of log(catch); can be overridden in control file with detailed F input");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            if (flt->getTypeInt() < 3)
            {
                num_lines = flt->getCatchModel()->rowCount();
                for (int j = 0; j <= num_lines; j++)
                {
                    str_lst = flt->getCatchObservation(j);
                    if (str_lst.at(0).isEmpty())
                        break;
//                    str_lst.insert(2, QString::number(i));
                    chars += d_file->write_vector(str_lst, 5);
/*                    line.clear();
                    for (int k = 0; k < str_lst.count(); k++)
                    {
                        line.append(QString("%1 ").arg (str_lst.at(k)));
                    }
                    line.chop(1);

                    chars += d_file->writeline (line);*/
                }
            }
        }
        line = QString("-9999 0 0 0 0");
        chars += d_file->writeline (line);
        chars += d_file->writeline ("#");

        // CPUE Abundance
        line = QString (" #_CPUE_and_surveyabundance_observations");
        chars += d_file->writeline(line);
        line = QString ("#_Units:  0=numbers; 1=biomass; 2=F; >=30 for special types");
        chars += d_file->writeline(line);
        line = QString ("#_Errtype:  -1=normal; 0=lognormal; >0=T");
        chars += d_file->writeline(line);
        line = QString ("#_SD_Report: 0=no sdreport; 1=enable sdreport");
        chars += d_file->writeline(line);
        line = QString ("#_Fleet Units Errtype SD_Report");
        chars += d_file->writeline(line);
        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            line = QString(QString("%1 %2 %3 %4 # %5").arg (
                   QString::number(i),
                   QString::number(flt->getAbundUnits()),
                   QString::number(flt->getAbundErrType()),
                   QString::number(flt->getSDReport()),
                   flt->getName()));
            chars += d_file->writeline (line);
        }

        line = QString("#_yr month fleet obs stderr");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++)//data->num_fisheries();i < data->num_fleets(); i++)
        {
            flt = data->getActiveFleet(i);
            int num_lines = flt->getAbundanceCount();
            for (int j = 0; j < num_lines; j++)
            {
                QStringList abund (flt->getAbundanceObs(j));
                if (!abund.at(0).isEmpty())
                {
                    chars += d_file->write_vector(abund, 4, flt->getName());
/*                    if (abund.at(1).isEmpty()) abund[1].append("1");

                    if (abund.at(2).isEmpty()) abund[2].append("0");
                    if (abund.at(3).isEmpty()) abund[3].append("0");
                    line = QString (QString("%1 %2 %3 %4 %5 #_ %6").arg (
                                        abund.at(0), abund.at(1),
                                        QString::number(i),
                                        abund.at(2), abund.at(3),
                                        flt->getName()));
                    chars += d_file->writeline (line);*/
                }
            }
        }
        line = QString("-9999 1 1 1 1 # terminator for survey observations ");
        chars += d_file->writeline (line);

        chars += d_file->writeline ("#");

        // discard
        temp_int = data->fleet_discard_count();
        line = QString (QString ("%1 #_N_fleets_with_discard").arg (temp_int));
        chars += d_file->writeline (line);
        line = QString("#_discard_units (1=same_as_catchunits(bio/num); 2=fraction; 3=numbers)");
        chars += d_file->writeline (line);
        line = QString("#_discard_errtype:  >0 for DF of T-dist(read CV below); 0 for normal with CV; -1 for normal with se; -2 for lognormal");
        chars += d_file->writeline (line);
        line = QString ("# note, only have units and errtype for fleets with discard ");
        chars += d_file->writeline (line);
        line = QString ("#_Fleet units errtype");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            if (flt->getDiscardCount() > 0)
            {
                line = QString(QString("%1 %2 %3 # %4").arg(
                            QString::number(i),
                            QString::number(flt->getDiscardUnits()),
                            QString::number(flt->getDiscardErrType()),
                            flt->getName()));
                chars += d_file->writeline (line);
            }
        }

        num = data->fleet_discard_obs_count();
//        line = QString(QString("%1 #N discard obs").arg(QString::number(temp_int)));
//        chars += d_file->writeline (line);
        line = QString("#_yr month fleet obs stderr");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            for (int j = 0; j < flt->getDiscardCount(); j++)
            {
                line.clear();
                str_lst = flt->getDiscard(j);
                chars += d_file->write_vector(str_lst, 5, flt->getName());
/*                str_lst.insert(2, QString::number(i));
                for (int m = 0; m < str_lst.count(); m++)
                    line.append(QString("%1 ").arg(str_lst.at(m)));
                line.append(QString(" # %1").arg(flt->getName()));
                chars += d_file->writeline (line);*/
            }
        }
        line.clear();
        if (num == 0)
            line = QString ("# ");
        line.append(QString ("-9999 0 0 0.0 0.0 # terminator for discard data "));
        chars += d_file->writeline (line);
        chars += d_file->writeline ("#");

        // mean body weight
        num = 0;
        for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
            num += data->getActiveFleet(i)->getMbwtNumObs();
        temp_int = num > 0? 1: 0;
        line = QString (QString("%1 #_use meanbodysize_data (0/1)").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        if (temp_int == 0)
        {
            line = QString("#_COND_0 ");
        }
        else
        {
            temp_int = data->getFleet(0)->getMbwtDF();
            line = QString (QString("%1 ").arg(QString::number(temp_int)));
        }
        line.append("#_DF_for_meanbodysize_T-distribution_like");
        chars += d_file->writeline (line);
        line = QString ("# note:  use positive partition value for mean body wt, negative partition for mean body length");
        chars += d_file->writeline (line);
        line = QString ("#_yr month fleet part obs stderr");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++)
        {
            flt = data->getActiveFleet(i);
            num_lines = flt->getMbwtNumObs();
            for (int j = 0; j < num_lines; j++)
            {
                line.clear();
                str_lst = flt->getMbwtObservation(j);
//                str_lst.insert(2, QString::number(i));
                chars += d_file->write_vector(str_lst, 4);
/*                for (int m = 0; m < str_lst.count(); m++)
                    line.append(QString(" %1").arg(str_lst.at(m)));

                chars += d_file->writeline (line);*/
            }
        }
        line.clear();
        if (num == 0)
            line.append("# ");
        line.append(QString("-9999 0 0 0 0 0 # terminator for mean body size data "));
        chars += d_file->writeline (line);
        chars += d_file->writeline ("#");


        // length composition
        {
        compositionLength *l_data = data->get_length_composition();
        line = QString ("# set up population length bin structure (note - irrelevant if not using size data and using empirical wtatage");
        chars += d_file->writeline (line);
        line = QString (QString("%1 # length bin method: 1=use databins; 2=generate from binwidth,min,max below; 3=read vector").arg(
                            QString::number(l_data->getAltBinMethod())));
        chars += d_file->writeline (line);
        switch (l_data->getAltBinMethod())
        {
        case 1:
            line = QString("# no additional input required.");
            chars += d_file->writeline (line);
            break;
        case 2:
            line = QString("%1 # binwidth for population size comp ").arg(QString::number(l_data->getAltBinWidth()));
            chars += d_file->writeline (line);
            line = QString("%1 # minimum size in the population (lower edge of first bin and size at age 0.00) ").arg(
                        QString::number(l_data->getAltBinMin()));
            chars += d_file->writeline (line);
            line = QString("%1 # maximum size in the population (lower edge of last bin) ").arg(
                        QString::number(l_data->getAltBinMax()));
            chars += d_file->writeline (line);
            break;
        case 3:
            line = QString("%1 # number of population size bins ").arg(
                        QString::number(l_data->getNumberAltBins()));
            chars += d_file->writeline (line);
            line.clear();
            str_lst = l_data->getAltBins();
            for (int j = 0; j < str_lst.count(); j++)
                line.append(QString(" %1").arg(str_lst.at(j)));

            chars += d_file->writeline (line);
            break;
        }
        line = QString(QString("%1 # use length composition data (0/1)").arg(QString::number(1)));
        chars += d_file->writeline(line);

        line = QString("#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.");
        chars += d_file->writeline (line);
        line = QString("#_addtocomp:  after accumulation of tails; this value added to all bins");
        chars += d_file->writeline (line);
        line = QString("#_males and females treated as combined gender below this bin number ");
        chars += d_file->writeline (line);
        line = QString("#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation");
        chars += d_file->writeline (line);
        line = QString("#_Comp_Error:  0=multinomial, 1=dirichlet");
        chars += d_file->writeline (line);
        line = QString("#_Comp_Error2:  parm number  for dirichlet");
        chars += d_file->writeline (line);
        line = QString("#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001");
        chars += d_file->writeline (line);
        line = QString("#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize");
        chars += d_file->writeline (line);
//        int fleetNum = 1;
        for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
        {
            flt = data->getActiveFleet(i);
            line.clear();
            line.append(QString("%1 ").arg((flt->getLengthMinTailComp())));
            line.append(QString("%1 ").arg((flt->getLengthAddToData())));
            line.append(QString("%1 ").arg(QString::number(flt->getLengthCombineGen())));
            line.append(QString("%1 ").arg(QString::number(flt->getLengthCompressBins())));
            line.append(QString("%1 ").arg(QString::number(flt->getLengthCompError())));
            line.append(QString("%1 ").arg(QString::number(flt->getLengthCompErrorParm())));
            line.append(QString("%1 ").arg(QString::number(flt->getLengthMinSampleSize())));
            line.append(QString("#_fleet:%1_%2").arg(QString::number(i), flt->getName()));
            chars += d_file->writeline (line);
        }


        line = QString("# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution");
        chars += d_file->writeline (line);
        line = QString("# partition codes:  (0=combined; 1=discard; 2=retained");
        chars += d_file->writeline (line);
        temp_int = l_data->getNumberBins();
        line = QString(QString("%1 #_N_LengthBins; then enter lower edge of each length bin").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        line.clear();
        str_lst = l_data->getBinsModel()->getRowData(0);
        for (i = 0; i < str_lst.count(); i++)
        {
            line.append(QString(" %1").arg(str_lst.at(i)));
        }

        chars += d_file->writeline (line);

        line = QString ("#_yr month fleet sex part Nsamp datavector(female-male)");
        chars += d_file->writeline (line);
//        for (int type = Fleet::Fishing; type < Fleet::None; type++)
        for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
        {
            flt = data->getActiveFleet(i);
            num = flt->getLengthNumObs();
            for( int j = 0; j < num; j++)
            {
                str_lst = flt->getLengthObservation(j);
                chars += d_file->write_vector(str_lst, 4);
/*                str_lst.insert(2, QString::number(i));
                line.clear();
                for (int j = 0; j < str_lst.count(); j++)
                    line.append(QString (" %1").arg(str_lst.at(j)));

                chars += d_file->writeline (line);*/
            }
        }
        line = QString("-9999");
        for (int j = 1; j < 6; j++)
            line.append(QString(" 0"));
        for (int j = 0; j < temp_int; j++)
            line.append(QString(" 0"));
        if (data->get_num_genders() > 1)
            for (int j = 0; j < temp_int; j++)
                line.append(QString(" 0"));
        line.append(' ');
        chars += d_file->writeline (line);
        chars += d_file->writeline ("#");
        }


        // age composition
        {
        compositionAge *a_data = data->get_age_composition();
        temp_int = a_data->getNumberBins();
        line = QString(QString("%1 #_N_age_bins").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        line.clear();
        str_lst = a_data->getBinsModel()->getRowData(0);
        for (i = 0; i < str_lst.count(); i++)
        {
            line.append(QString(" %1").arg(str_lst.at(i)));
        }

        chars += d_file->writeline (line);
        temp_int = a_data->number_error_defs();
        line = QString(QString("%1 #_N_ageerror_definitions").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        for (i = 0; i < temp_int; i++)
        {
            line.clear();
            str_lst = a_data->get_error_ages(i);
            for (int j = 0; j < str_lst.count(); j++)
                line.append(QString(" %1").arg(str_lst.at(j)));

            chars += d_file->writeline (line);
            line.clear();
            str_lst = a_data->get_error_def(i);
            for (int j = 0; j < str_lst.count(); j++)
                line.append(QString(" %1").arg(str_lst.at(j)));

            chars += d_file->writeline (line);
        }
        line = QString("#_mintailcomp: upper and lower distribution for females and males separately are accumulated until exceeding this level.");
        chars += d_file->writeline (line);
        line = QString("#_addtocomp:  after accumulation of tails; this value added to all bins");
        chars += d_file->writeline (line);
        line = QString("#_males and females treated as combined gender below this bin number ");
        chars += d_file->writeline (line);
        line = QString("#_compressbins: accumulate upper tail by this number of bins; acts simultaneous with mintailcomp; set=0 for no forced accumulation");
        chars += d_file->writeline (line);
        line = QString("#_Comp_Error:  0=multinomial, 1=dirichlet");
        chars += d_file->writeline (line);
        line = QString("#_Comp_Error2:  parm number  for dirichlet");
        chars += d_file->writeline (line);
        line = QString("#_minsamplesize: minimum sample size; set to 1 to match 3.24, minimum value is 0.001");
        chars += d_file->writeline (line);
        line = QString("#_mintailcomp addtocomp combM+F CompressBins CompError ParmSelect minsamplesize");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
        {
            flt = data->getActiveFleet(i);
            line = QString(QString("%1 %2 %3 %4 %5 %6 %7 #_fleet:%8_%9").arg (
                               flt->getAgeMinTailComp(),
                               flt->getAgeAddToData(),
                               QString::number(flt->getAgeCombineGen()),
                               QString::number(flt->getAgeCompressBins()),
                               QString::number(flt->getAgeCompError()),
                               QString::number(flt->getAgeCompErrorParm()),
                               QString::number(flt->getAgeCompMinSampleSize()),
                               QString::number(i),
                               flt->getName()));
            chars += d_file->writeline (line);
        }
        temp_int = a_data->getAltBinMethod();
        line = QString(QString("%1 #_Lbin_method_for_Age_Data: 1=poplenbins; 2=datalenbins; 3=lengths").arg(
                           QString::number(temp_int)));
        chars += d_file->writeline (line);

        temp_int = 0;
        for (i = 1; i <= total_fleets; i++)
        {
            temp_int += data->getActiveFleet(i)->getAgeNumObs();
        }

        line = QString ("# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution");
        chars += d_file->writeline (line);
        line = QString ("# partition codes:  (0=combined; 1=discard; 2=retained");
        chars += d_file->writeline (line);
        line = QString ("#_yr month fleet sex part ageerr Lbin_lo Lbin_hi Nsamp datavector(female-male)");
        chars += d_file->writeline (line);
        for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
        {
            flt = data->getActiveFleet(i);
            {
                temp_int = flt->getAgeNumObs();
                for( int j = 0; j < temp_int; j++)
                {
                    str_lst = flt->getAgeObservation(j);
                    chars += d_file->write_vector(str_lst, 4);
/*                    str_lst.insert(2, QString::number(i));
                    line.clear();
                    for (int j = 0; j < str_lst.count(); j++)
                        line.append(QString (" %1").arg(str_lst.at(j)));

                    chars += d_file->writeline (line);*/
                }
            }
        }
        line.clear();
        temp_int = a_data->getNumberBins();
        line.append("-9999 ");
        for (i = 1; i < 9; i++)
                line.append(" 0");
        for (int j = 0; j < temp_int; j++)
            line.append(QString(" 0"));
        if (data->get_num_genders() > 1)
            for (int j = 0; j < temp_int; j++)
                line.append(QString(" 0"));
        chars += d_file->writeline(line);
        chars += d_file->writeline ("#");
        }

        // mean size at age
        num = 0;
        for (i = 1; i <= total_fleets; i++)
        {
                num += data->getActiveFleet(i)->getSaaModel()->rowCount();
        }
        temp_int = num > 0? 1: 0;
        line = QString ("%1 #_Use_MeanSize-at-Age_obs (0/1)").arg (
                    QString::number(temp_int));
        chars += d_file->writeline (line);

        if (num > 0)
        {
            line = QString ("# sex codes:  0=combined; 1=use female only; 2=use male only; 3=use both as joint sexxlength distribution");
            chars += d_file->writeline (line);
            line = QString ("# partition codes:  (0=combined; 1=discard; 2=retained");
            chars += d_file->writeline (line);
            line = QString ("# ageerr codes:  positive means mean length-at-age; negative means mean bodywt_at_age");
            chars += d_file->writeline (line);
            line = QString ("#_yr month fleet sex part ageerr ignore datavector(female-male)");
            chars += d_file->writeline (line);
            line = QString ("#                                          samplesize(female-male)");
            chars += d_file->writeline (line);
            for (i = 1; i <= total_fleets; i++)
            {
                flt = data->getActiveFleet(i);
                {
                    for (int j = 0; j < flt->getSaaNumObs(); j++)
                    {
                        line.clear();
                        str_lst = flt->getSaaModel()->getRowData(j);
                        chars += d_file->write_vector(str_lst, 4);
/*                        str_lst.insert(2, QString::number(i));
                        for (int m = 0; m < str_lst.count(); m++)
                            line.append(QString(" %1").arg(str_lst.at(m)));

                        chars += d_file->writeline (line);*/
                    }
                }
            }
            line.clear();
            line = QString("-9999 ");
            for (i = 1; i < str_lst.count(); i++)
                line.append(" 0 ");
            chars += d_file->writeline(line);
        }
        chars += d_file->writeline ("#");


        // environment observations
        temp_int = data->getNumEnvironVars();
        line = QString (QString("%1 #_N_environ_variables").arg(
                            QString::number(temp_int)));
        chars += d_file->writeline (line);
        if (temp_int > 0)
        {
            num = data->getNumEnvironVarObs();
            line = QString ("#Year Variable Value");
            chars += d_file->writeline (line);
            for (i = 0; i < num; i++)
            {
                line.clear();
                str_lst = data->get_environ_var_obs(i);
                for (int j = 0; j < str_lst.count(); j++)
                    line.append(QString(" %1").arg(str_lst.at(j)));
                chars += d_file->writeline (line);
            }
            line = QString("-9999  0  0  # terminator");
            chars += d_file->writeline(line);
        }
        chars += d_file->writeline ("#");

        // general composition methods
        num = data->getNumGeneralCompMethods();
        line = QString (QString("%1 # N sizefreq methods to read ").arg(
                            QString::number(num)));
        chars += d_file->writeline (line);
        if (num > 0)
        {
            line.clear();
            for (i = 0; i < num; i++)
            {
                temp_str = QString::number(data->getGeneralCompMethod(i)->getNumberBins());
                line.append(QString("%1 ").arg(temp_str));
            }
            line.append("#_Sizefreq N bins per method");
            chars += d_file->writeline (line);
            line.clear();
            for (i = 0; i < num; i++)
            {
                temp_str = QString::number(data->getGeneralCompMethod(i)->getUnits());
                line.append(QString("%1 ").arg(temp_str));
            }
            line.append("#_Sizetfreq units (bio/num) per method");
            chars += d_file->writeline (line);
            line.clear();
            for (i = 0; i < num; i++)
            {
                temp_str = QString::number(data->getGeneralCompMethod(i)->getScale());
                line.append(QString("%1 ").arg(temp_str));
            }
            line.append("#_Sizefreq scale (kg/lbs/cm/inches) per method");
            chars += d_file->writeline (line);
            line.clear();
            for (i = 0; i < num; i++)
            {
                temp_str = QString::number(data->getGeneralCompMethod(i)->getMinComp());
                line.append(QString("%1 ").arg(temp_str));
            }
            line.append("#_Sizefreq mincomp per method ");
            chars += d_file->writeline (line);
            line.clear();
            for (i = 0; i < num; i++)
            {
                temp_int = 0;
                for (j = 1; j <= total_fleets; j++)
                {
                    flt = data->getActiveFleet(j);
//                    if (data->getFleet(j)->isActive())
                    {
                        temp_int += flt->getGenNumObs(i);
                    }
                }
                line.append(QString("%1 ").arg(QString::number(temp_int)));
            }
            line.append("#_Sizefreq N obs per method");
            chars += d_file->writeline (line);
            line = QString("#_Sizefreq bins");
            chars += d_file->writeline (line);
            for (i = 0; i < num; i++)
            {
                line.clear();
                str_lst = data->getGeneralCompMethod(i)->getBinsModel()->getRowData(0);
                for (int j = 0; j < str_lst.count(); j++)
                    line.append(QString (" %1").arg (str_lst.at(j)));

                chars += d_file->writeline (line);
            }
            line = QString ("#Method Yr Month Flt Gender Part Nsamp datavector(female-male)");
            chars += d_file->writeline (line);
            for (i = 0; i < num; i++)
            {
                for (j = 1; j <= total_fleets; j++) // for (int j = 0; j < data->num_fleets(); j++)
                {
                    flt = data->getActiveFleet(j);
//                    if (data->getFleet(j)->isActive())
                    {
                        num_lines = flt->getGenNumObs(i);
                        for (int k = 0; k < num_lines; k++)
                        {
                            line.clear();
                            str_lst = flt->getGenObservation(i, k);
                            chars += d_file->write_vector(str_lst, 4);
/*                            str_lst.insert(3, QString::number(j));
                            for (int m = 0; m < str_lst.count(); m++)
                                line.append(QString (" %1").arg(str_lst.at(m)));

                            chars += d_file->writeline (line);*/
                        }
                    }
                }
            }
        }
        chars += d_file->writeline ("#");

        // tag recapture
        temp_int = data->get_do_tags()? 1: 0;
        line = QString (QString("%1 # do tags (0/1)").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        if (temp_int == 1)
        {
            line = QString (QString("%1 # Do_tags").arg(QString("1")));
            chars += d_file->writeline (line);
            num = data->get_num_tag_groups();
            line = QString (QString("%1 # N_tag_groups").arg(QString::number(num)));
            chars += d_file->writeline (line);
            temp_int = 0;
            for (i = 1; i <= total_fleets; i++) // for (i = 0; i < data->num_fleets(); i++)
            {
//                if (data->getFleet(i)->isActive())
                    temp_int += data->getActiveFleet(i)->getRecapNumEvents();
            }
            line = QString (QString("%1 # N_recapture_events").arg(QString::number(temp_int)));
            chars += d_file->writeline (line);
            temp_int = data->get_tag_latency();
            line = QString (QString("%1 # Mixing_latency_period").arg(QString::number(temp_int)));
            chars += d_file->writeline (line);
            temp_int = data->get_tag_max_periods();
            line = QString (QString("%1 # Max_periods").arg(QString::number(temp_int)));
            chars += d_file->writeline (line);
            line = QString (QString("#Release_Data"));
            chars += d_file->writeline (line);
            line = QString (QString("#TG area yr month <tfill> gender age Nrelease"));
            chars += d_file->writeline (line);
            for (i = 0; i < num; i++)
            {
                line.clear();
                str_lst = data->get_tag_observation(i);
                chars += d_file->write_vector(str_lst, 4);
/*                for (int j = 0; j < str_lst.count(); j++)
                    line.append(QString(" %1").arg(str_lst.at(j)));
                chars += d_file->writeline (line);*/
            }
            line = QString("#Recapture_Data");
            chars += d_file->writeline (line);
            line = QString("#TG year month fleet Number");
            chars += d_file->writeline (line);
            for (i = 0; i < num; i++)
            {
                for (j = 1; j <= total_fleets; j++) // for (int j = 0; j < data->num_fleets(); j++)
                {
                    flt = data->getActiveFleet(j);
//                    if (data->getFleet(j)->isActive())
                    {
                        num_lines = flt->getRecapNumEvents();
                        for (int k = 0; k < num_lines; k++)
                        {
                            line.clear();
                            str_lst = flt->getRecapObservation(k);
                            if (str_lst.at(0).toInt() == i)
                            {
                                chars += d_file->write_vector(str_lst, 4);
                            }
/*                            str_lst.insert(3, QString::number(j));
                            if (str_lst.at(0).toInt() == i)
                            {
                            for (int m = 0; m < str_lst.count(); m++)
                                line.append(QString(" %1").arg(str_lst.at(m)));
                            chars += d_file->writeline (line);
                            }*/
                        }
                    }
                }
            }
        }
        chars += d_file->writeline(QString("#"));

        // morph composition
        temp_int = data->getDoMorphComp()? 1: 0;
        line = QString (QString("%1 # morphcomp data(0/1)").arg(QString::number(temp_int)));
        chars += d_file->writeline (line);
        line = QString ("#  Nobs, Nmorphs, mincomp");
        chars += d_file->writeline (line);
        line = QString ("#  yr, seas, type, partition, Nsamp, datavector_by_Nmorphs");
        chars += d_file->writeline (line);
        if (temp_int == 1)
        {
            num_lines = 0;
            for (i = 0; i < data->get_num_fleets(); i++)
                num_lines += data->getFleet(i)->getMorphNumObs();
            line = QString (QString("%1 # N_observations").arg(QString::number(num_lines)));
            chars += d_file->writeline (line);
            temp_int = data->get_morph_composition()->getNumberMorphs();
            line = QString (QString("%1 # N_morphs").arg(QString::number(temp_int)));
            chars += d_file->writeline (line);
            temp_str = data->getFleet(0)->getMorphMinTailComp();//get_morph_composition()->mincomp();
            line = QString (QString("%1 # Mincomp").arg(temp_str));//QString::number(temp_float)));
            chars += d_file->writeline (line);
            temp_int = data->get_tag_latency();
            line = QString("#Year month fleet partition Nsamp data_vector");
            chars += d_file->writeline (line);
            for (i = 1; i <= total_fleets; i++) // for (num = 0; num < data->num_fleets(); num++)
            {
                flt = data->getActiveFleet(i);
//                if (data->fleets.at(num)->isActive())
                {
                    num_lines = flt->getMorphNumObs();
                    for (int j = 0; j < num_lines; j++)
                    {
                        line.clear();
                        str_lst = flt->getMorphObservation(j);
                        chars += d_file->write_vector(str_lst, 4);
/*                        str_lst.insert(2, QString::number(i));
                        for (int k = 0; k < str_lst.count(); k++)
                            line.append(QString(" %1").arg(str_lst.at(k)));
                        chars += d_file->writeline (line);*/
                    }
                }
            }
        }
        chars += d_file->writeline ("#");
//        d_file->newline();

        line = QString(QString("%1 # Do dataread for selectivity priors(0/1)").arg(
                           QString::number(data->getReadSelectivityPriors()? 1: 0)));
        chars += d_file->writeline (line);
        line = QString("# Yr, Seas, Fleet,  Age/Size,  Bin,  selex_prior,  prior_sd");
        chars += d_file->writeline (line);
        line = QString("# feature not yet implemented");
        chars += d_file->writeline (line);
        chars += d_file->writeline ("#");

        //end of data
        line = QString (QString("%1").arg (QString::number(END_OF_DATA)));
        chars += d_file->writeline (line);
        d_file->newline();
        chars += d_file->writeline ("ENDDATA");


        d_file->close();
    }
    return chars;
}

bool read33_forecastFile(ss_file *f_file, ss_model *data)
{
    QString token;
    QString temp_str;
    QStringList str_lst(" ");
    float temp_float;
    int temp_int = 0;
    int i, num, fleet, area;

    if(f_file->open(QIODevice::ReadOnly))
    {
        //  SS_Label_Info_3.0 #Read forecast.ss
        ss_forecast *fcast = data->forecast;
        fcast->reset();
        f_file->seek(0);
        f_file->read_comments();

        fcast->set_num_seasons(data->get_num_seasons());
        fcast->set_num_fleets(data->get_num_fleets());
        fcast->set_num_genders(data->get_num_genders());

        token = f_file->get_next_value("benchmarks type");
        temp_int = token.toInt();
        fcast->set_benchmarks(temp_int);

        token = f_file->get_next_value("Forecast method");
        temp_int = token.toInt();
        fcast->set_MSY(temp_int);

        token = f_file->get_next_value("SPR target");
        temp_float = token.toFloat();
        fcast->set_spr_target(temp_float);

        token = f_file->get_next_value("biomass target");
        temp_float = token.toFloat();
        fcast->set_biomass_target(temp_float);

        for (i = 0; i < 10; i++)
        {
            token = f_file->get_next_value("benchmark year");
            temp_int = token.toInt();
            fcast->set_benchmark_years(i, temp_int);
        }
        token = f_file->get_next_value("bmark rel f basis");
        temp_int = token.toInt();
        fcast->set_benchmark_rel_f(temp_int);

        token = f_file->get_next_value("forecast type");
        temp_int = token.toInt();
        fcast->set_forecast(temp_int);
        token = f_file->get_next_value("number of forecast years");
        temp_int = token.toInt();
        fcast->set_num_forecast_years(temp_int);
        token = f_file->get_next_value("F scalar");
        temp_float = token.toFloat();
        fcast->set_f_scalar(temp_float);
        for (i = 0; i < 6; i++)
        {
            token = f_file->get_next_value("forecast year");
            temp_int = token.toInt();
            fcast->set_forecast_year(i, temp_int);
        }

        token = f_file->get_next_value("Forecast selectivity (not yet implemented)");
        temp_int = token.toInt();
        fcast->setSelectivity (temp_int);
        token = f_file->get_next_value("control rule method");
        temp_int = token.toInt();
        fcast->set_cr_method(temp_int);
        token = f_file->get_next_value("control rule upper limit");
        temp_float = token.toFloat();
        fcast->set_cr_biomass_const_f(temp_float);
        token = f_file->get_next_value("control rule lower limit");
        temp_float = token.toFloat();
        fcast->set_cr_biomass_no_f(temp_float);
        token = f_file->get_next_value("control rule buffer");
        temp_float = token.toFloat();
        fcast->set_cr_target(temp_float);

        token = f_file->get_next_value("number of loops");
        temp_int = token.toInt();
        fcast->set_num_forecast_loops(temp_int);
        token = f_file->get_next_value("loop with recruitment");
        temp_int = token.toInt();
        fcast->set_forecast_loop_recruitment(temp_int);
        token = f_file->get_next_value("loop control 3");
        temp_int = token.toInt();
        fcast->set_forecast_loop_ctl3(temp_int);
        token = f_file->get_next_value("loop control 4");
        temp_int = token.toInt();
        fcast->set_forecast_loop_ctl4(temp_int);
        token = f_file->get_next_value("loop control 5");
        temp_int = token.toInt();
        fcast->set_forecast_loop_ctl5(temp_int);

        token = f_file->get_next_value("caps and allocs first yr");
        temp_int = token.toInt();
        fcast->set_caps_alloc_st_year(temp_int);

        token = f_file->get_next_value("std dev log(catch/tgt)");
        temp_float = token.toFloat();
        fcast->set_log_catch_std_dev(temp_float);
        token = f_file->get_next_value("West Coast rebuilder");
        temp_int = token.toInt();
        fcast->set_do_rebuilder(temp_int == 1? true: false);
        token = f_file->get_next_value("rebuilder: first year");
        temp_int = token.toInt();
        fcast->set_rebuilder_first_year(temp_int);
        token = f_file->get_next_value("rebuilder: curr year");
        temp_int = token.toInt();
        fcast->set_rebuilder_curr_year(temp_int);

        token = f_file->get_next_value("fleet relative F");
        temp_int = token.toInt();
        fcast->set_fleet_rel_f(temp_int);

        token = f_file->get_next_value("max forecast catch basis");
        temp_int = token.toInt();
        fcast->set_catch_tuning_basis(temp_int);

        if (fcast->get_fleet_rel_f() == 2)
        {
            for (int s = 0; s < fcast->get_num_seasons(); s++)
            {
                int total = fcast->get_num_fleets();
                if (fcast->get_num_areas() > 1)
                    total += fcast->get_num_areas();
                for (i = 0; i < total; i++)
                {
                    temp_float = f_file->get_next_value("seas flt rel f").toFloat();
                    fcast->set_seas_fleet_rel_f(s, i, temp_float);
                }
            }
        }

        // max catch fleet
        do {
            fleet = f_file->get_next_value().toInt();
            temp_float = f_file->get_next_value("max catch fleet").toFloat();
            if (fleet != -9999)
                fcast->set_max_catch_fleet((fleet - 1), temp_float);
        } while (fleet != -9999);

        // max catch area
        for (i = 0; i < fcast->get_num_areas(); i++)
            fcast->set_max_catch_area(i, 0);
        do {
            area = f_file->get_next_value().toInt();
            temp_float = f_file->get_next_value("max catch area").toFloat();
            if (area != -9999)
                fcast->set_max_catch_area((area - 1), temp_float);
        } while (area != -9999);

        // Allocation groups
        fcast->set_num_alloc_groups(0);
        for (i = 0; i < data->get_num_fleets(); i++)
            data->getFleet(i)->setAllocGroup(0);
        do {
            fleet = f_file->get_next_value("fleet alloc grp").toInt();
            temp_int = f_file->get_next_value("alloc grp").toInt();
            if (fleet != -9999)
            {
                data->getFleet(fleet - 1)->setAllocGroup(temp_int);
                fcast->setAllocGrp(fleet - 1, temp_int);
            }
        } while (fleet != -9999);

        if (fcast->get_num_alloc_groups() > 0)
        {
            int j = 0;
            do
            {
                str_lst.clear();
                token = f_file->get_next_value("alloc grp frac year");
                temp_int = token.toInt();
                str_lst.append(token);
                for (i = 0; i < fcast->get_num_alloc_groups(); i++)
                {
                    token = f_file->get_next_value("alloc group fraction");
                    str_lst.append(token);
                }
                if (temp_int != -9999)
                    fcast->setAllocFractions(j++, str_lst);
            } while (temp_int != -9999);
        }

        // Forecast Catch
        fcast->setNumFixedFcastCatch(0);
        token = f_file->get_next_value("input catch basis");
        num = token.toInt();
        fcast->set_input_catch_basis(num);
        do
        {
            str_lst.clear();
            token = f_file->get_next_value("Year");
            temp_int = token.toInt();
            str_lst.append(token);                // Year
            str_lst.append(f_file->get_next_value("Season")); // Season
            str_lst.append(f_file->get_next_value("Fleet")); // Fleet
            str_lst.append(f_file->get_next_value("Catch")); // Catch
            if (num == -1)
                str_lst.append(f_file->get_next_value("Basis")); // Basis
            if (temp_int != -9999)
                fcast->addFixedFcastCatch(str_lst);
        } while (temp_int != -9999);

        //  SS_Label_Info_3.5 #End of datafile indicator
        token = f_file->get_next_value("End of data indicator");
        temp_int = token.toInt();
        if (temp_int != END_OF_DATA)
        {
            if (f_file->atEnd())
            {
                f_file->error(QString("Must have 999 to verify end of forecast inputs!"));
            }
            else
            {
                if (fcast->get_forecast() > 0)
                    f_file->error(QString("Stopped reading before end of data marker!"));
            }
        }

        f_file->close();
    }
    else
    {
        f_file->error(QString("Forecast file does not exist or is not readable."));
    }
    return 1;
}

int write33_forecastFile(ss_file *f_file, ss_model *data)
{
    int temp_int, num, i, chars = 0;
    int yr;
    QString value, line, temp_string;
    QStringList str_lst, tmp_lst;
    ss_forecast *fcast = data->forecast;


    if(f_file->open(QIODevice::WriteOnly))
    {
//        chars += f_file->writeline(QString("#V%1").arg(getDatafileVersionString()));
        chars += writeVersionComment(f_file);
        chars += f_file->write_comments();

        line = QString("# for all year entries except rebuilder; enter either: actual year, -999 for styr, 0 for endyr, neg number for rel. endyr");
        chars += f_file->writeline(line);

        chars += f_file->write_val(fcast->get_benchmarks(), 5,
                                   QString("Benchmarks: 0=skip; 1=calc F_spr,F_btgt,F_msy"));

        chars += f_file->write_val(fcast->get_MSY(), 5,
                                   QString("MSY: 1=set to F(SPR); 2=calc F(MSY); 3=set to F(Btgt); 4=set to F(endyr)"));

        chars += f_file->write_val(fcast->get_spr_target(), 5, QString("SPR target (e.g. 0.40)"));

        chars += f_file->write_val(fcast->get_biomass_target(), 5, QString("Biomass target (e.g. 0.40)"));

        line = QString("#_Bmark_years: beg_bio, end_bio, beg_selex, end_selex, beg_relF, end_relF, beg_recr_dist, end_recr_dist, beg_SRparm, end_SRparm");
//        temp_string = QString ("# ");
        line.append(QString(" (enter actual year, or values of 0 or -integer to be rel. endyr)"));
        chars += f_file->writeline(line);
        line.clear();
        temp_string = QString("# ");
        for (i = 0; i < 10; i++)
        {
            temp_int = fcast->get_benchmark_year(i);
            value = QString::number(temp_int);
            line.append(QString(QString(" %1").arg(value)));
//            if (temp_int <= 0)
            temp_string.append(QString(" %1").arg(QString::number(data->refyearvalue(temp_int))));// data->get_end_year() + temp_int)));
//            else
//                temp_string.append(QString(" %1").arg(value));
        }
        chars += f_file->writeline(line);
        temp_string.append(" # after processing ");
        chars += f_file->writeline(temp_string);

        chars += f_file->write_val(fcast->get_benchmark_rel_f(), 5,
                                   QString("Bmark_relF_Basis: 1 = use year range; 2 = set relF same as forecast below"));

        chars += f_file->writeline("#");

        chars += f_file->write_val(fcast->get_forecast(), 5, QString("Forecast: 0=none; 1=F(SPR); 2=F(MSY) 3=F(Btgt); 4=Ave F (uses first-last relF yrs); 5=input annual F scalar"));

        chars += f_file->write_val(fcast->get_num_forecast_years(), 5, QString("N forecast years"));

        chars += f_file->write_val(fcast->get_f_scalar(), 5, QString("F scalar (only used for Do_Forecast==5)"));

        line = QString("#_Fcast_years:  beg_selex, end_selex, beg_relF, end_relF, beg_recruits, end_recruits  (enter actual year, or values of 0 or -integer to be rel. endyr)");
        chars += f_file->writeline(line);
        line.clear();
        temp_string = QString("# ");
        for (int i = 0; i < 6; i++)
        {
            value = QString::number(fcast->get_forecast_year(i));
            temp_int = value.toInt();
            line.append(QString(QString(" %1").arg(value)));
//            if (fcast->get_forecast_year(i) <= 0)
            temp_string.append(QString(" %1").arg(QString::number(data->refyearvalue(temp_int))));//data->get_end_year() + fcast->get_forecast_year(i))));
//            else
//                temp_string.append(QString(" %1").arg(value));
        }
        chars += f_file->writeline(line);
        temp_string.append(" # after processing ");
        chars += f_file->writeline(temp_string);

        chars += f_file->write_val(fcast->getSelectivity(), 5, QString("Forecast selectivity (not yet implemented)"));
        chars += f_file->write_val(fcast->get_cr_method(), 5, QString("Control rule method (1=catch=f(SSB) west coast; 2=F=f(SSB))"));
        chars += f_file->write_val(fcast->get_cr_biomass_const_f(), 5, QString("Control rule Biomass level for constant F (as frac of Bzero, e.g. 0.40); (Must be > the no F level below)"));
        chars += f_file->write_val(fcast->get_cr_biomass_no_f(), 5, QString("Control rule Biomass level for no F (as frac of Bzero, e.g. 0.10)"));
        chars += f_file->write_val(fcast->get_cr_target(), 5, QString("Control rule target as fraction of Flimit (e.g. 0.75)"));

        chars += f_file->write_val(fcast->get_num_forecast_loops(), 5, QString("N forecast loops (1=OFL only; 2=ABC; 3=get F from forecast ABC catch with allocations applied)"));
        chars += f_file->write_val(fcast->get_forecast_loop_recruitment(), 5, QString("First forecast loop with stochastic recruitment"));
        chars += f_file->write_val(fcast->get_forecast_loop_ctl3(), 5, QString("Forecast loop control #3 (reserved for future bells&whistles)"));
        chars += f_file->write_val(fcast->get_forecast_loop_ctl4(), 5, QString("Forecast loop control #4 (reserved for future bells&whistles)"));
        chars += f_file->write_val(fcast->get_forecast_loop_ctl5(), 5, QString("Forecast loop control #5 (reserved for future bells&whistles)"));

        chars += f_file->write_val(fcast->get_caps_alloc_st_year(), 5, QString("First Year for caps and allocations (should be after years with fixed inputs)"));

        chars += f_file->write_val(fcast->get_log_catch_std_dev(), 5, QString("stddev of log(realized catch/target catch) in forecast (set value>0.0 to cause active impl_error)"));

        chars += f_file->write_val((fcast->get_do_rebuilder()? 1: 0), 5, QString("Do West Coast gfish rebuilder output (0/1)"));
        chars += f_file->write_val(fcast->get_rebuilder_first_year(), 5, QString ("Rebuilder: first year catch could have been set to zero (Ydecl)(-1 to set to 1999)"));
        chars += f_file->write_val(fcast->get_rebuilder_curr_year(), 5, QString ("Rebuilder: year for current age structure (Yinit) (-1 to set to endyear+1)"));

        chars += f_file->write_val(fcast->get_fleet_rel_f(), 5, QString ("fleet relative F: 1=use first-last alloc year; 2=read seas(row) x fleet(col) below"));
        line = QString("# Note that fleet allocation is used directly as average F if Do_Forecast=4");
        chars += f_file->writeline(line);

        chars += f_file->write_val(fcast->get_catch_tuning_basis(), 5, QString ("basis for fcast catch tuning and for fcast catch caps and allocation  (2=deadbio; 3=retainbio; 5=deadnum; 6=retainnum)"));

        line = QString("# Conditional input if relative F choice = 2");
        chars += f_file->writeline(line);
        line = QString("# Fleet relative F:  rows are seasons, columns are fleets");
        chars += f_file->writeline(line);
        line = QString ("#_Fleet: ");
        for (i = 1; i <= data->getNumActiveFleets(); i++)
        {
            line.append(QString(" %1").arg (data->getActiveFleet(i)->getName()));
        }
        chars += f_file->writeline(line);
        if (fcast->get_fleet_rel_f() == 2)
        {
            temp_string = QString("");
            for (int seas = 0; seas < data->get_num_seasons(); seas++)
            {
                str_lst = fcast->getSeasFleetRelF(seas);
                chars += f_file->write_vector(str_lst, 5, QString("Season %1").arg(QString::number(seas+1)));
            }
        }
        else
        {
            temp_string = QString("# ");
            for (i = 0; i < data->get_num_fisheries(); i++)
            {
                if (data->getFleet(i)->isActive())
                temp_string.append(" 1");
            }
            for (; i < data->get_num_fleets(); i++)
            {
                if (data->getFleet(i)->isActive())
                temp_string.append(" 0");
            }
            chars += f_file->writeline(temp_string);
        }
        temp_string.clear();

        line = QString("# enter list of fleet number and max for fleets with max annual catch; terminate with fleet=-9999");
        chars += f_file->writeline(line);
        str_lst = fcast->getMaxCatchFleets();
        line.clear();
        for (i = 0; i < fcast->get_num_fleets(); i++)
        {
            line.clear();
            value = str_lst.at(i);
            if (data->getFleet(i)->isActive())
            {
                if (value.toFloat() > 0)
                {
                    line.append(QString("%1 %2").arg(QString::number(i+1), value));
                    chars += f_file->writeline(line);
                }
            }
        }
        line = QString ("-9999 -1");
        chars += f_file->writeline(line);

        line = QString("# enter list of area ID and max annual catch; terminate with area=-9999");
        chars += f_file->writeline(line);
        str_lst = fcast->getMaxCatchAreas();
        for (i = 0; i < fcast->get_num_areas(); i++)
        {
            line.clear();
            value = str_lst.at(i);
            if (value.toFloat() != 0)
            {
                line.append(QString("%1 %2").arg(QString::number(i+1), value));
                chars += f_file->writeline(line);
            }
        }
        line = QString ("-9999 -1");
        chars += f_file->writeline(line);

        // allocation groups
        line = QString("# enter list of fleet number and allocation group assignment, if any; terminate with fleet=-9999");
        chars += f_file->writeline(line);
        str_lst = fcast->getAllocGrpList();
        line.clear();
        if (fcast->get_num_alloc_groups() > 0)
        {
            for (i = 0; i < fcast->get_num_fleets(); i++)
            {
                if (data->getFleet(i)->isActive())
                {
                    line.clear();
                    value = str_lst.at(i);
                    if (value.toInt() > 0)
                    {
                        line = QString("%1 %2").arg(QString::number(i+1), value);
                        chars += f_file->writeline(line);
                    }
                }
            }
        }
        line = QString ("-9999 -1");
        chars += f_file->writeline(line);

        fcast->getAllocFractModel()->sort(0);
        temp_string = QString::number(fcast->get_num_alloc_groups());
        line = QString("#_if N allocation groups >0, list year, allocation fraction for each group");
        chars += f_file->writeline(line);
        line = QString("# list sequentially because read values fill to end of N forecast");
        chars += f_file->writeline(line);
        line = QString("# terminate with -9999 in year field");
        chars += f_file->writeline(line);
        line.clear();
        if (fcast->get_num_alloc_groups() > 0)
        {
            line = QString(QString("#Yr alloc frac for each of: %1 alloc grps").arg(temp_string));
            chars += f_file->writeline(line);
            line.clear();
            for (yr = 0; yr < fcast->get_num_forecast_years(); yr++)
            {
                line.clear();
                str_lst = fcast->getAllocFractions(yr);
                for (i = 0; i < str_lst.count(); i++)
                {
                    line.append(QString(QString(" %1").arg(str_lst.at(i))));
                }
                chars += f_file->writeline(line);
            }
            line = QString("-9999");
            for (i = 0; i < fcast->get_num_alloc_groups(); i++)
                line.append(QString(" 1"));
            chars += f_file->writeline(line);
        }
        else
        {
            line = QString ("# no allocation groups");
            chars += f_file->writeline(line);
        }

        temp_int = fcast->get_input_catch_basis();
        chars += f_file->write_val(temp_int, 5, QString("basis for input Fcast catch: -1=read basis with each obs; 2=dead catch; 3=retained catch; 99=input Hrate(F)"));
        line = QString("#enter list of Fcast catches; terminate with line having year=-9999");
        chars += f_file->writeline(line);
        line = QString("#_Year Seas Fleet Catch(or_F)");
        if (temp_int < 0)
            line.append(QString(" Basis"));
        chars += f_file->writeline(line);
        num = fcast->getNumFixedFcastCatch();
        for (i = 0; i < num; i++)
        {
            str_lst = fcast->getFixedFcastCatch(i);
            line.clear();
            for (int j = 0; j < 4; j++)
                line.append(QString(" %1").arg(str_lst.at(j)));
            if (temp_int < 0)
            {
                if (str_lst.count() == 4)
                    str_lst.append(QString("2"));
            }
            chars += f_file->write_vector(str_lst, 5);
        }
        line = QString("-9999  1    1    0 ");
        if (temp_int < 0)
            line.append("   2");
        chars += f_file->writeline(line);

        chars += f_file->writeline("#");

        chars += f_file->write_val(END_OF_DATA, 6, QString("verify end of input"));

        f_file->close();
    }
    return chars;
}

bool read33_controlFile(ss_file *c_file, ss_model *data)
{
    int i, temp_int, index, num, num_vals;
    float temp_float;
    QString temp_string;
    QStringList datalist;
    population * pop = data->pPopulation;
    int flt;
    int num_fleets = data->get_num_fleets();
    int timevaryread;

    if(c_file->open(QIODevice::ReadOnly))
    {
        c_file->seek(0);
        c_file->read_comments();

        // read wtatage.ss
        temp_int = c_file->get_next_value("Read wtatage.ss?").toInt();
        data->setReadWtAtAge(temp_int);

        // growth patterns
        num = c_file->get_next_value("Num growth patterns").toInt();
        pop->Grow()->setNum_patterns(num);
        // morphs or platoons
        num = c_file->get_next_value("Num growth morphs").toInt();
        pop->Grow()->setNum_morphs(num); // 1, 3, and 5 are best, normal dist set
        num = pop->Grow()->getNum_morphs();
        pop->Grow()->setMorph_within_ratio(1.0);
        pop->Grow()->setMorph_dist(0, 1.0);
        if (num > 1)
        {
            temp_float = c_file->get_next_value("Morph between ratio").toFloat();
            pop->Grow()->setMorph_within_ratio (temp_float);
            temp_float = c_file->get_next_value("Morph dist").toFloat();
            if ((int)temp_float != -1) // normal dist is the default
            {
                float total = temp_float;
                pop->Grow()->setMorph_dist(0, temp_float);
                for (i = 1; i < num; i++)
                {
                    temp_float = c_file->get_next_value("Morph dist").toFloat();
                    total += temp_float;
                    pop->Grow()->setMorph_dist(i, temp_float);
                }
                if (total != 1.0)
                {
                    for (i = 0; i < num; i++) // normalizing values so total = 1.0
                    {
                        temp_float = pop->Grow()->getMorph_dist(i) / total;
                        pop->Grow()->setMorph_dist(i, temp_float);
                    }
                }
            }
            else
            {
                for (i = 1; i < num; i++)
                {
                    temp_float = c_file->get_next_value("Morph dist - ignored").toFloat();
                }
                pop->Grow()->setMorphDist();
            }
        }

        // recruitment designs
        index = c_file->get_next_value("Recr dist").toInt(); // recruitment distribution
        pop->SR()->setDistribMethod(index);
        temp_int = c_file->get_next_value("Recr dist area").toInt(); // recruitment dist area
        pop->SR()->setDistribArea(temp_int);
        num = c_file->get_next_value("Num recr assigns").toInt(); // num recr assignments
        pop->SR()->setNumAssignments(num);
        temp_int = c_file->get_next_value("Recr unused").toInt(); // read interact params?
        pop->SR()->setDoRecruitInteract(temp_int);
/*        switch (index)
        {
        case 1:
        case 2:
        case 3:
            break;
        case 4:
            pop->SR()->setNumAssignments(1);
            pop->SR()->setDoRecruitInteract(0);
            break;
        }*/

        for (i = 0; i < num; i++) // gr pat, month, area, age for each assignment
        {
            datalist.clear();
            for (int j = 0; j < 4; j++)
                datalist.append(c_file->get_next_value("Recr assign data"));
            pop->SR()->setAssignment(i, datalist);
        }

        // movement definitions
        pop->Move()->setNumDefs(0);
        pop->Move()->setFirstAge(0);
        if (data->get_num_areas() > 1)
        {
            num = c_file->get_next_value("Num move defs").toInt();
            pop->Move()->setNumDefs(num);
            if (num > 0)
            {
                temp_float = c_file->get_next_value("Move first age").toFloat();
                pop->Move()->setFirstAge(temp_float);
                for (i = 0; i < num; i++)
                {
                    datalist.clear();
                    for (int j = 0; j < 6; j++)
                        datalist.append(c_file->get_next_value("Move definition"));
                    pop->Move()->setDefinition(i, datalist);
                }
            }
        }

        // time block definitions
        num = c_file->get_next_value("Num block patterns").toInt();
        data->setNumBlockPatterns(num);
        if (num > 0)
        {
            for (i = 0; i < num; i++)
            {
                temp_int = c_file->get_next_value("Num blocks").toInt();
                data->getBlockPattern(i)->setNumBlocks(temp_int);
            }
            for (i = 0; i < num; i++)
            {
                for (int j = 0; j < data->getBlockPattern(i)->getNumBlocks(); j++)
                {
                    datalist.clear();
                    datalist.append(c_file->get_next_value("block start"));
                    datalist.append(c_file->get_next_value("block end"));
                    data->getBlockPattern(i)->setBlock(j, datalist);
                }
            }
        }
        // controls for time-varying params
        temp_int = c_file->get_next_value("time varying method").toInt();
        pop->Grow()->setTimeVaryMethod(temp_int);
        temp_int = c_file->get_next_value("time varying biology").toInt();
        pop->Grow()->setTimeVaryReadParams(temp_int);
        temp_int = c_file->get_next_value("time varying spawn-recr").toInt();
        pop->SR()->setTimeVaryReadParams(temp_int);
        temp_int = c_file->get_next_value("time varying fleet q").toInt();
        for (i = 0; i < data->get_num_fleets(); i++)
            data->getFleet(i)->setQTimeVaryReadParams(temp_int);
        temp_int = c_file->get_next_value("time varying tagging").toInt();
        data->setTagTimeVaryReadParams(temp_int); // for future capability
        temp_int = c_file->get_next_value("time varying fleet selex").toInt();
        data->getFleet(0)->setSelTimeVaryReadParams(temp_int);

        // natural Mort
        temp_int = c_file->get_next_value("Nat Mort type").toInt();
        pop->Grow()->setNatural_mortality_type(temp_int);
        switch (temp_int)
        {
        default:
        case 0:
            num_vals = 1;  // 1 parameter for each gender only
            break;
        case 1:
            num_vals = c_file->get_next_value("Num breakpts").toInt(); // num breakpoints
            pop->Grow()->setNatMortNumBreakPts(num_vals);
            datalist.clear();
            for (int i = 0; i < num_vals; i++) // vector of breakpoints
                datalist.append(c_file->get_next_value("Breakpts"));
            pop->Grow()->setNatMortBreakPts(datalist);
            num_vals = num_vals; // read N params for each gender for each GP
            break;
        case 2:
            temp_int = c_file->get_next_value("Lorenz ref age").toInt(); // ref age for Lorenzen
            pop->Grow()->setNaturnalMortLorenzenRef(temp_int);
            num_vals = 1; // read 1 param for each gender for each GP
            break;
        case 3:
        case 4:
            // age-specific M values by sex by growth pattern
            num = pop->Grow()->getNum_patterns();
            num_vals = data->get_age_composition()->getNumberBins();
            for (int i = 0; i < num; i++) // first female M for each growth pattern
            {
                datalist.clear();
                for (int j = 0; j < num_vals; j++)
                {
                    datalist.append(c_file->get_next_value("age-spec M vals female"));
                }
                pop->Grow()->getPattern(i)->setNatMFemAgeList(datalist);
            }
//            if (data->num_genders() > 1)
            {
                for (int i = 0; i < num; i++) // now male M for each growth pattern
                {
                    datalist.clear();
                    for (int j = 0; j < num_vals; j++)
                    {
                        datalist.append(c_file->get_next_value("age-spec M vals male"));
                    }
                    pop->Grow()->getPattern(i)->setNatMMaleAgeList(datalist);
                }
            }
            num_vals = 0; // read no additional parameters
            break;
        }
        for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
        {
            pop->Grow()->getPattern(i)->setNumNatMParams(num_vals);
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // growth model
        temp_int = c_file->get_next_value("Growth model").toInt();
        pop->Grow()->setModel(temp_int);

        temp_float = c_file->get_next_value("age for L1").toFloat();
        pop->Grow()->setAge_for_l1(temp_float);
        temp_float = c_file->get_next_value("age for L2").toFloat();
        pop->Grow()->setAge_for_l2(temp_float);
        if (pop->Grow()->getModel() == 3)
        {
            temp_float = c_file->get_next_value("age min for K").toFloat();
            pop->Grow()->setAgeMin_for_K(temp_float);
            temp_float = c_file->get_next_value("age max for K").toFloat();
            pop->Grow()->setAgeMax_for_K(temp_float);
        }
        // Exponential decay for growth above max age
        temp_float = c_file->get_next_value().toFloat();
        pop->Grow()->setExpDecay(temp_float);

        // future feature
        temp_float = c_file->get_next_value().toFloat();
        pop->Grow()->setFeature(temp_float);

        // SD add to LAA
        temp_float = c_file->get_next_value().toFloat();
        pop->Grow()->setSd_add(temp_float);
        // CV growth pattern
        temp_int = c_file->get_next_value().toInt();
        pop->Grow()->setCv_growth_pattern(temp_int);

        // maturity
        temp_int = c_file->get_next_value().toInt();
        pop->Grow()->setMaturity_option(temp_int);
        if (temp_int == 3 ||
            temp_int == 4)
        {
            datalist.clear();
            num = data->get_age_composition()->getNumber(); // num_ages() + 1;
            for (i = 0; i <= num; i++)
                datalist.append(c_file->get_next_value());
            pop->Grow()->setNumMatAgeValues(datalist.count());
            pop->Grow()->setMatAgeVals(datalist);
        }
        else if (temp_int == 6)
        {
            datalist.clear();
            num = data->get_length_composition()->getNumberBins();
            for (i = 0; i < num; i++)
                datalist.append(c_file->get_next_value());
            pop->Grow()->setNumMatAgeValues(datalist.count());
            pop->Grow()->setMatAgeVals(datalist);
        }

        temp_float = c_file->get_next_value().toFloat();
        pop->Grow()->setFirst_mature_age(temp_float);

        // fecundity
        temp_int = c_file->get_next_value().toInt();
        pop->Fec()->setMethod(temp_int);

        temp_int = c_file->get_next_value().toInt();
        pop->Fec()->setHermaphroditism(temp_int);
        if (temp_int != 0)
        {
            temp_float = c_file->get_next_value().toFloat();
            pop->Fec()->setHermSeason(temp_float);
            temp_int = c_file->get_next_value().toInt();
            pop->Fec()->setHermIncludeMales(temp_int);
        }

        temp_int = c_file->get_next_value().toInt();
        pop->Grow()->setParam_offset_method(temp_int);

        // mortality growth parameters
        num = 0;
        index = 0;
        for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
        {
            growthPattern *gp = pop->Grow()->getPattern(i);
//            gp->setNumNatMParams(0);
            // nat Mort
            temp_int = pop->Grow()->getNatural_mortality_type();
            switch (temp_int)
            {
            case 0:
            case 2:
                datalist = readParameter (c_file);
                gp->setNatMParam(0, datalist);
                gp->setNatMParamHeader(0, QString("NatM_p_1_Fem_GP_%1").arg(QString::number(i+1)));
                break;
            case 1:
                num_vals = pop->Grow()->getNatMortNumBreakPts();
                for (int j = 0; j < num_vals; j++)
                {
                    datalist = readParameter (c_file);
                    gp->setNatMParam(j, datalist);
                    gp->getNatMParams()->setRowHeader(j, QString("NatM_p_%1_Fem_GP_%2").arg(QString::number(j+1), QString::number(i+1)));
                    index++;
                }
                break;
            case 3:
            case 4:
                break;
            }

            gp->setNumGrowthParams(3);
                                              // Female parameters
            datalist = readParameter(c_file); // L at Amin
            gp->setGrowthParam(0, datalist);
            gp->setGrowthParamHeader(0, QString("L_at_Amin_Fem_GP_%1").arg(QString::number(i+1)));
            datalist = readParameter(c_file); // L at Amax
            gp->setGrowthParam(1, datalist);
            gp->setGrowthParamHeader(1, QString("L_at_Amax_Fem_GP_%1").arg(QString::number(i+1)));
            datalist = readParameter(c_file); // von Bertalanffy
            gp->setGrowthParam(2, datalist);
            gp->setGrowthParamHeader(2, QString("VonBert_K_Fem_GP_%1").arg(QString::number(i+1)));
            if (pop->Grow()->getModel() == 2)
            {
                datalist = readParameter(c_file); // Richards coefficient
                gp->setGrowthParam(3, datalist);
                gp->setGrowthParamHeader(3, QString("Richards_Fem_GP_%1").arg(QString::number(i+1)));
            }
            if (pop->Grow()->getModel() == 3)
            {
                for (int k = 0; k < data->get_num_ages(); k++)
                {
                    datalist = readParameter(c_file); // K deviations per age
                    gp->setGrowthParam(k+3, datalist);
                    gp->setGrowthParamHeader(k+3, QString("Dev_age_Fem_GP_%1").arg(QString::number(i+1)));
                }
            }
            gp->setNumCVParams(2);
            datalist = readParameter(c_file); // CV young
            gp->setCVParam(0, datalist);
            gp->getCVParams()->setRowHeader(0, QString("CV_young_Fem_GP_%1").arg(QString::number(i+1)));
            datalist = readParameter(c_file); // CV old
            gp->setCVParam(1, datalist);
            gp->getCVParams()->setRowHeader(1, QString("CV_old_Fem_GP_%1").arg(QString::number(i+1)));
        }

        num = 0;
        pop->Grow()->setNumWtLenParams(2);
        datalist = readParameter(c_file); // fem_wt_len_1
        pop->Grow()->setWtLenParam(num, datalist);
        pop->Grow()->setWtLenParamHeader(num, QString("Wtlen_1_Fem"));
        num++;
        datalist = readParameter(c_file); // fem_wt_len_2
        pop->Grow()->setWtLenParam(num, datalist);
        pop->Grow()->setWtLenParamHeader(num, QString("Wtlen_2_Fem"));

        num = 0;
        datalist = readParameter(c_file); // fem_mat_inflect
        pop->Fec()->setFemaleParam(num, datalist);
        pop->Fec()->getFemaleParams()->setRowHeader(num, QString("Mat50%_Fem"));
        num++;
        datalist = readParameter(c_file); // fem_mat_slope
        pop->Fec()->setFemaleParam(num, datalist);
        pop->Fec()->getFemaleParams()->setRowHeader(num, QString("Mat_slope_Fem"));
        num++;
        datalist = readParameter(c_file); // fem_fec_alpha
        pop->Fec()->setFemaleParam(num, datalist);
        pop->Fec()->getFemaleParams()->setRowHeader(num, QString("Eggs/kg_inter_Fem"));
        num++;
        datalist = readParameter(c_file); // fem_fec_beta
        pop->Fec()->setFemaleParam(num, datalist);
        pop->Fec()->getFemaleParams()->setRowHeader(num, QString("Eggs/kg_slope_wt_Fem"));
        num++;

        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        if (data->get_num_genders() > 1)          // Male parameters
        {
            for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
            {
                growthPattern *gp = pop->Grow()->getPattern(i);
                temp_int = pop->Grow()->getNatural_mortality_type();
                switch (temp_int)
                {
                case 0:
                case 2:
                    num = gp->getNatMParams()->rowCount();
                    datalist = readParameter (c_file);
                    gp->setNatMParam(num, datalist);
                    gp->setNatMParamHeader(num, QString("NatM_p_1_Mal_GP_%1").arg(QString::number(i+1)));
                    break;
                case 1:
                    num = gp->getNatMParams()->rowCount();
                    num_vals = pop->Grow()->getNatMortNumBreakPts();
                    for (int j = 0; j < num_vals; j++)
                    {
                        datalist = readParameter (c_file);
                        gp->setNatMParam(j+num, datalist);
                        gp->setNatMParamHeader(j+num, QString("NatM_p_%1_Mal_GP_%2").arg(QString::number(j+1), QString::number(i+1)));
                    }
                    break;
                case 3:
                case 4:
                    break;
                }

                num = gp->getNumGrowthParams();
                datalist = readParameter(c_file); // L at Amin
                gp->setGrowthParam(num, datalist);
                gp->setGrowthParamHeader(num, QString("L_at_Amin_Mal_GP_%1").arg(QString::number(i+1)));
                num++;
                datalist = readParameter(c_file); // L at Amax
                gp->setGrowthParam(num, datalist);
                gp->setGrowthParamHeader(num, QString("L_at_Amax_Mal_GP_%1").arg(QString::number(i+1)));
                num++;
                datalist = readParameter(c_file); // von Bertalanffy
                gp->addGrowthParam(datalist);
                gp->setGrowthParamHeader(num, QString("VonBert_K_Mal_GP_%1").arg(QString::number(i+1)));
                num++;
                if (pop->Grow()->getModel() == 2)
                {
                    datalist = readParameter(c_file); // Richards coefficient
                    gp->addGrowthParam(datalist);
                    gp->setGrowthParamHeader(num, QString("Richards_Mal_GP_%1").arg(QString::number(i+1)));
                    num++;
                }
                if (pop->Grow()->getModel() == 3)
                {
                    for (int k = 0; k < data->get_num_ages(); k++)
                    {
                        datalist = readParameter(c_file); // K deviations per age
                        gp->setGrowthParam(k+3, datalist);
                        gp->setGrowthParamHeader(k+3, QString("Dev_age_Mal_GP_%1").arg(QString::number(i+1)));
                    }
                }
                gp->setNumCVParams(4);
                datalist = readParameter(c_file); // CV young
                gp->setCVParam(2, datalist);
                gp->getCVParams()->setRowHeader(2, QString("CV_young_Mal_GP_%1").arg(QString::number(i+1)));
                datalist = readParameter(c_file); // CV old
                gp->setCVParam(3, datalist);
                gp->getCVParams()->setRowHeader(3, QString("CV_old_Mal_GP_%1").arg(QString::number(i+1)));
            }

            num = pop->Grow()->getNumWtLenParams();
            datalist = readParameter(c_file); // male_wt_len_1
            pop->Grow()->setWtLenParam(num, datalist);
            pop->Grow()->setWtLenParamHeader(num, QString("Wtlen_1_Mal"));
            num++;
            datalist = readParameter(c_file); // male_wt_len_2
            pop->Grow()->setWtLenParam(num, datalist);
            pop->Grow()->setWtLenParamHeader(num, QString("Wtlen_2_Mal"));
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;


        if (pop->Fec()->getHermaphroditism())
        {
            i = 0;
            datalist = readParameter(c_file); // hermaph_inflect
            pop->Fec()->setHermParam(i, datalist);
            pop->Fec()->getHermParams()->setRowHeader(i, QString("Hermaph_inflect_age"));
            i++;
            datalist = readParameter(c_file); // hermaph_sd
            pop->Fec()->setHermParam(i, datalist);
            pop->Fec()->getHermParams()->setRowHeader(i, QString("Hermaph_std_dev"));
            i++;
            datalist = readParameter(c_file); // hermaph_asymptotic
            pop->Fec()->setHermParam(i, datalist);
            pop->Fec()->getHermParams()->setRowHeader(i, QString("Hermaph_asymp_rate"));
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;


        num_vals = (pop->Grow()->getNum_patterns() + data->get_num_areas() + data->get_num_seasons());
        pop->SR()->setNumDistParams(num_vals);
        for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
        {
            datalist = readParameter(c_file); // recr apportion main
            pop->SR()->setDistParam(i, datalist);
            pop->SR()->getDistParams()->setRowHeader(i, QString("RecrDist_GP_%1").arg(QString::number(i+1)));
        }
        for (num = 0; num < data->get_num_areas(); num++, i++)
        {
            datalist = readParameter(c_file); // recr apportion to areas
            pop->SR()->setDistParam(i, datalist);
            pop->SR()->getDistParams()->setRowHeader(i, QString("RecrDist_Area_%1").arg(QString::number(num+1)));
        }
        index = pop->SR()->getDistribMethod();
        num_vals = pop->SR()->getNumAssignments();
        if (index == 2)
        {
            datalist = readParameter(c_file); // recr apportion to settlement events
            pop->SR()->setDistParam(i, datalist);
            pop->SR()->getDistParams()->setRowHeader(i, QString("RecrDist_timing_1"));
            pop->SR()->setNumDistParams(i+1);
        }
        else if (index == 3 || (index == 4 && (num_vals * pop->Grow()->getNum_patterns() > 1)))
        {
            for (num = 0; num < num_vals; num++, i++)
            {
                datalist = readParameter(c_file); // recr apportion to settlement events
                pop->SR()->setDistParam(i, datalist);
                pop->SR()->getDistParams()->setRowHeader(i, QString("RecrDist_Settlement_%1").arg(QString::number(num+1)));
            }
            pop->SR()->setNumDistParams(i);
        }
        if (pop->SR()->getDoRecruitInteract())
        {
            index = 0;
            pop->SR()->setNumInteractParams(num_vals);
            for (i = 0; i < pop->Grow()->getNum_patterns(); i++, index++)
            {
                datalist = readParameter(c_file); // recr interaction growth pattern
                pop->SR()->setInteractParam(index, datalist);
                pop->SR()->getInteractParams()->setRowHeader(index, QString("RecrDist_GP_%1").arg(QString::number(i+1)));
            }
            for (i = 0; i < data->get_num_areas(); i++, index++)
            {
                datalist = readParameter(c_file); // recr interaction area
                pop->SR()->setInteractParam(index, datalist);
                pop->SR()->getInteractParams()->setRowHeader(index, QString("RecrDist_Area_%1").arg(QString::number(i+1)));
            }
            for (i = 0; i < data->get_num_seasons(); i++, index++)
            {
                datalist = readParameter(c_file); // recr interaction season
                pop->SR()->setInteractParam(index, datalist);
                pop->SR()->getInteractParams()->setRowHeader(index, QString("RecrDist_Bseas_%1").arg(QString::number(i+1)));
            }
        }
        datalist = readParameter(c_file); // cohort growth deviation
        pop->Grow()->setCohortParam(datalist);
        pop->Grow()->getCohortParams()->setRowHeader(0, QString("CohortGrowDev"));
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;


        // movement parameters (2 per definition)
        num = pop->Move()->getNumDefs();
        for (i = 0; i < num; i++)
        {
            QStringList def = pop->Move()->getDefinition(i);
            temp_string = QString("seas_%1_GP_%2from_%3to_%4").arg(
                        def.at(0), def.at(1), def.at(2), def.at(3));
            int par = i * 2;
            datalist = readParameter(c_file);    // parameter A
            pop->Move()->setParameter (par, datalist);
            pop->Move()->setParamHeader(par, QString("MoveParm_A_%1").arg(temp_string));
            datalist = readParameter(c_file);    // parameter B
            pop->Move()->setParameter (par + 1, datalist);
            pop->Move()->setParamHeader(par+1, QString("MoveParm_B_%1").arg(temp_string));
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // ageing error if requested
        if (data->get_age_composition()->getUseParameters())
        {
            for (i = 0; i < 7; i++)
            {
                datalist = readParameter(c_file); // parameters for age error matrix
                data->get_age_composition()->setErrorParam(i, datalist);
            }
        }

        // Fraction Female paramters
        for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
        {
            datalist = readParameter(c_file);
            pop->Grow()->getPattern(i)->setFractionFemaleParam(datalist);
            pop->Grow()->getPattern(i)->getFractionFemaleParams()->setRowHeader(0, QString("FracFemale_GP_%1").arg(QString::number(i+1)));
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // timevary MG Parameters
        timevaryread = pop->Grow()->getTimeVaryReadParams();
        if (timevaryread > 0)
        {
            growthPattern *gp;
            num_vals = 1;
            if (data->get_num_genders() > 1)
                num_vals = 2;
            for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
            {
                gp = pop->Grow()->getPattern(i);

                readTimeVaryParams(c_file, data, gp->getNatMParams(), timevaryread, gp->getNatMTVParams());
                readTimeVaryParams(c_file, data, gp->getGrowthParams(), timevaryread, gp->getGrowthTVParams());
            }
            readTimeVaryParams(c_file, data, pop->Grow()->getWtLenParams(), timevaryread, pop->Grow()->getWtLenTVParams());
        }


        // seasonal_effects_on_biology_parms
        datalist.clear();
        for (i = 0; i < 10; i++)
        {
            temp_string = c_file->get_next_value("seasonal effects setup");
            datalist.append(temp_string);
        }
        pop->setSeasParamSetup(datalist);
        temp_int = pop->getNumSeasParams();
        for (i = 0; i < datalist.count(); i++)
        {
            QStringList parm;
            temp_string = datalist.at(i);
            temp_int = temp_string.toInt();
            if (temp_int != 0)
            {
                parm = readShortParameter (c_file);
                pop->setSeasonalParam(i, parm);
            }
        }

        // Spawner-recruitment
        temp_int = c_file->get_next_value().toInt();
        pop->SR()->setMethod (temp_int);
        temp_int = c_file->get_next_value().toInt();
        pop->SR()->setUseSteepness (temp_int);
        // future feature
        temp_int = c_file->get_next_value().toInt();
        pop->SR()->setFeature (temp_int);
        i = 0;
        // SR params
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_LN(R0)");
        }
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_BH_steep");
        }
        if (temp_int == 5 ||
                temp_int == 7 ||
                temp_int == 8)
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_3rd_PARM");
        }
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_sigmaR");
        }
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_regime");
        }
        {
            datalist = readParameter(c_file);
            pop->SR()->setFullParameter(i, datalist);
            pop->SR()->setFullParameterHeader(i++, "SR_autocorr");
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // SR time vary params
        timevaryread = pop->SR()->getTimeVaryReadParams();
        readTimeVaryParams(c_file, data, pop->SR()->getFullParameters(), timevaryread, pop->SR()->getTVParameterModel()->getVarParamTable());

        // Recruitment deviations
        pop->SR()->setRecDevCode(c_file->get_next_value("Recr dev code").toInt());
        pop->SR()->setRecDevStartYr(c_file->get_next_value("Recr dev start yr").toInt());
        pop->SR()->setRecDevEndYr(c_file->get_next_value("Recr dev end yr").toInt());
        pop->SR()->setRecDevPhase(c_file->get_next_value("Recr dev phase").toInt());

        // SR advanced opts
        pop->SR()->setAdvancedOpts(c_file->get_next_value("Read advanced options?").compare("0") != 0);
        if (pop->SR()->getAdvancedOpts())
        {
            pop->SR()->setRecDevEarlyStart(c_file->get_next_value().toInt());
            pop->SR()->setRecDevEarlyPhase(c_file->get_next_value().toInt());
            pop->SR()->setFcastRecPhase(c_file->get_next_value().toInt());
            pop->SR()->setFcastLambda(c_file->get_next_value().toFloat());
            pop->SR()->setNobiasLastEarlyYr(c_file->get_next_value().toInt());
            pop->SR()->setFullbiasFirstYr(c_file->get_next_value().toInt());
            pop->SR()->setFullbiasLastYr(c_file->get_next_value().toInt());
            pop->SR()->setNobiasFirstRecentYr(c_file->get_next_value().toInt());
            pop->SR()->setMaxBiasAdjust(c_file->get_next_value().toFloat());
            pop->SR()->setRecCycles(c_file->get_next_value().toInt());
            pop->SR()->setRecDevMin(c_file->get_next_value().toInt());
            pop->SR()->setRecDevMax(c_file->get_next_value().toInt());
            pop->SR()->setNumRecDev(c_file->get_next_value().toInt());

            pop->SR()->setNumCycleParams(pop->SR()->getRecCycles());
            for (i = 0; i < pop->SR()->getRecCycles(); i++)
            {
                datalist = readParameter(c_file);
                pop->SR()->setCycleParam(i, datalist);
            }

            for (i = 0; i < pop->SR()->getNumRecDev(); i++)
            {
                datalist.clear();
                datalist.append(c_file->get_next_value());
                datalist.append(c_file->get_next_value());
                pop->SR()->setRecruitDev(i, datalist);
            }
        }

        // Fishing mortality
        num = 0;
        pop->M()->setNumFisheries(data->get_num_fisheries());
        for (i = 0; i < num_fleets; i++)
        {
            if (data->getFleet(i)->catch_equil() != 0)
                num++;
        }
        pop->M()->setNumInitialParams(num);
        temp_float = c_file->get_next_value("Mort ballpark F").toFloat();
        pop->M()->setBparkF(temp_float); // bparkF ;
        temp_int = c_file->get_next_value("Mort ballpark year").toInt();
        pop->M()->setBparkYr(temp_int); // bparkYr
        temp_int = c_file->get_next_value("Mort method").toInt();
        pop->M()->setMethod(temp_int); // method
        temp_float = c_file->get_next_value("Mort maxF").toFloat();
        pop->M()->setMaxF(temp_float); // maxF
        pop->M()->setStartF(0.0); //startF = 0;
        pop->M()->setPhase(0); // phase = 0;
        pop->M()->setNumInputs(0); // numInputs = 0;
        pop->M()->setNumTuningIters(0); // numTuningIters = 0;
        switch (pop->M()->getMethod())
        {
        case 2:
            temp_float = c_file->get_next_value("Mort start F").toFloat();
            pop->M()->setStartF(temp_float); // startF
            temp_int = c_file->get_next_value("Mort phase").toInt();
            pop->M()->setPhase(temp_int); // phase
            temp_int = c_file->get_next_value("Mort num inputs").toInt();
            pop->M()->setNumInputs(temp_int); // numInputs
            break;
        case 3:
            temp_int = c_file->get_next_value("Mort num tuning iters").toInt();
            pop->M()->setNumTuningIters(temp_int); // numTuningIters
            break;
        }

        for (i = 0; i < pop->M()->getNumInputs(); i++)
        {
            datalist.clear();
            for (int j = 0; j < 6; j++)
            {
                datalist.append(c_file->get_next_value("Mort inputs"));
            }
            pop->M()->setInputLine (i, datalist);
        }

        for (i = 0; i < pop->M()->getNumInitialParams(); i++)
        {
            datalist = readShortParameter(c_file);
            pop->M()->setInitialParam(i, datalist);
            pop->M()->getInitialParams()->setRowHeader(i, QString("Fleet%1").arg(QString::number(i+1)));
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;


        // Q setup
        do
        {
            datalist.clear();
            flt = c_file->get_next_value().toInt();

            for (int j = 0; j < 5; j++)
                datalist.append(c_file->get_next_value(QString("Q setup")));
            if (flt > 0)
            {
                data->getFleet(flt - 1)->Q()->setSetup(datalist);
                data->getFleet(flt - 1)->setQSetupRead(true);
            }
            if (datalist.at(0).compare("EOF") == 0)
                return false;
        } while (flt > 0);

        for (i = 0; i < data->get_num_fleets(); i++)
        {
            if (data->getFleet(i)->getType() != Fleet::Survey)
            {
                if (data->getFleet(i)->getQSetupRead())
                    showInputMessage (QString("Q Setup line for non-survey fleet %1").arg(data->getFleet(i)->getName()));
            }
            else
            {
                if (!data->getFleet(i)->getQSetupRead() && data->getFleet(i)->abundance_count() > 0)
                    showInputMessage (QString("No Q Setup line for survey %1").arg(data->getFleet(i)->getName()));
            }
        }

        // Q parameters
        int id = 0;

        for (i = 0; i < num_fleets; i++)
        {
            Fleet *fleet = data->getFleet(i);
            if (fleet->getQSetupRead())
            {
                // Q Base
                if (fleet->Q()->getSetupTable()->getRowData(0).at(0).toInt() > 0)
                {
                    datalist = readParameter(c_file);
                    fleet->Q()->setBase(datalist);
                }
                // Q Power
                if (fleet->Q()->getDoPower())
                {
                    datalist = readParameter(c_file);
                    fleet->Q()->setPower(datalist);
                }
                // Q Extra SD
                if (fleet->Q()->getDoExtraSD())
                {
                    datalist = readParameter(c_file);
                    fleet->Q()->setExtra(datalist);
                }
            }
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // Q timevary params
        timevaryread = data->getFleet(0)->getQTimeVaryReadParams();
        if (timevaryread > 0)
        {
        for (i = 0; i < data->get_num_fleets(); i++)
        {
            if (data->getFleet(i)->getQSetupRead())
                readTimeVaryParams(c_file, data,
                           data->getFleet(i)->Q()->getParamTable(),
                           timevaryread, data->getFleet(i)->Q()->getTVParams());
            data->getFleet(i)->setName(data->getFleet(i)->getName());
        }
        }

        {
        selectivity *sizesel;
        selectivity *agesel;
        // Size selectivity setup
        for (int i = 0; i < num_fleets; i++)
        {
            sizesel = data->getFleet(i)->getSizeSelectivity();
            datalist.clear();
            for (int j = 0; j < 4; j++)
                datalist.append(c_file->get_next_value("size selex setup"));
            sizesel->setSetup(datalist);
        }
        // Age selectivity setup
        for (int i = 0; i < num_fleets; i++)
        {
            agesel = data->getFleet(i)->getAgeSelectivity();
            temp_int = c_file->get_next_value("age selex pattern").toInt();
            agesel->setPattern(temp_int);
            temp_int = c_file->get_next_value("age selex discard").toInt();
            agesel->setDiscard(temp_int);
            temp_int = c_file->get_next_value("age selex male").toInt();
            agesel->setMale(temp_int);
            temp_int = c_file->get_next_value("age selex special").toInt();
            agesel->setSpecial(temp_int);
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        // read size selectivity parameters
        for (int i = 0; i < num_fleets; i++)
        {
            sizesel = data->getFleet(i)->getSizeSelectivity();
            int num = sizesel->getNumParameters();
            //read num_params parameters
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter (c_file);
                sizesel->setParameter(j, datalist);
                sizesel->setParameterLabel(j,
                           QString("SizeSel_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            // read retention and discard parameters
            num = sizesel->getNumRetainParameters();
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter (c_file);
                sizesel->setRetainParameter(j, datalist);
                sizesel->setRetainParameterLabel(j,
                           QString("Retain_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            num = sizesel->getNumDiscardParameters();
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter (c_file);
                sizesel->setDiscardParameter(j, datalist);
                sizesel->setDiscardParameterLabel(j,
                           QString("DiscMort_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            // read male offset
            num = sizesel->getNumMaleParameters();
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter (c_file);
                sizesel->setMaleParameter(j, datalist);
                sizesel->setMaleParameterLabel(j,
                           QString("Male_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            // check special
            num = sizesel->getSpecial();
            if (sizesel->getPattern() == 7)
                sizesel->setSpecial(1);
        }
        // read age selectivity parameters
        for (int i = 0; i < num_fleets; i++)
        {
            agesel = data->getFleet(i)->getAgeSelectivity();
            int num = agesel->getNumParameters();
            //read num_params parameters
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter(c_file);
                agesel->setParameter(j, datalist);
                agesel->setParameterLabel(j,
                           QString("AgeSel_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            // check discard
            if (agesel->getDiscard() != 0)
                agesel->setDiscard(0);
            // read male offset
            num = agesel->getNumMaleParameters();
            for (int j = 0; j < num; j++)
            {
                datalist = readParameter (c_file);
                agesel->setMaleParameter(j, datalist);
                agesel->setMaleParameterLabel(j,
                           QString("Male_P%1_%2(%3)").arg(
                           QString::number(j+1),
                           data->getFleet(i)->getName(),
                           QString::number(i+1)));
            }
            // check special
            num = agesel->getSpecial();
            if (temp_int == 7)
                agesel->setSpecial(1);
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        timevaryread = data->getFleet(0)->getSelTimeVaryReadParams();
        if (timevaryread > 0)
        {
            tablemodel *paramtable;
            tablemodel *varParamtable;

            for (i = 0; i < data->get_num_fleets(); i++)
            {
                // size selex time varying
                sizesel = data->getFleet(i)->getSizeSelectivity();
                paramtable = data->getFleet(i)->getSizeSelexModel();
                varParamtable = sizesel->getTimeVaryParameterModel();
                readTimeVaryParams(c_file, data, paramtable, timevaryread, varParamtable);
            }
            for (i = 0; i < data->get_num_fleets(); i++)
            {
                agesel = data->getFleet(i)->getAgeSelectivity();
                paramtable = agesel->getParameterModel();
                varParamtable = agesel->getTimeVaryParameterModel();
                readTimeVaryParams(c_file, data, paramtable, timevaryread, varParamtable);
            }
        }
        }

        // 2D-AR1 smoother
        temp_int = c_file->get_next_value().toInt();
        for (i = 0; i < data->get_num_fleets(); i++)
        {
            data->getFleet(i)->setAr1SelSmoother(temp_int);
        }

        // Tag loss and Tag reporting parameters go next
        temp_int = c_file->get_next_value().toInt();
        data->setTagLoss(temp_int);
        if (temp_int == 1)
        {
            // tag loss init
            // tag loss chronic
            // tag overdispersion
            // tag report fleet
            // tag report decay
            data->setTagLossParameter(c_file->read_line());
        }

        // #_Variance_adjustments_to_input_value
        data->setInputValueVariance(0);
        do
        {
            temp_string = c_file->get_next_value(QString("input var id"));
            id = temp_string.toInt();
            flt = c_file->get_next_value(QString("input var fleet")).toInt();
            temp_float = c_file->get_next_value(QString("input var value")).toFloat();
            if (temp_string.compare("EOF") == 0)
                return false;
            if (id == -9999)
                break;
            data->setInputValueVariance(1);
            switch (id)
            {
            case 1:
                data->getFleet(flt-1)->setAddToSurveyCV(temp_float);
                break;
            case 2:
                data->getFleet(flt-1)->setAddToDiscardSD(temp_float);
                break;
            case 3:
                data->getFleet(flt-1)->setAddToBodyWtCV(temp_float);
                break;
            case 4:
                data->getFleet(flt-1)->setMultByLenCompN(temp_float);
                break;
            case 5:
                data->getFleet(flt-1)->setMultByAgeCompN(temp_float);
                break;
            case 6:
                data->getFleet(flt-1)->setMultBySAA(temp_float);
                break;
            case 7:
            default:
                break;
            }
        } while (id != -9999);

        // Max lambda phase
        temp_int = c_file->get_next_value("Lambda max phase").toInt();
        data->setLambdaMaxPhase(temp_int);

        // sd offset
        temp_int = c_file->get_next_value("Lambda sd offset").toInt();
        data->setLambdaSdOffset(temp_int);

        // lambda changes
        // component, fleet, phase, value, sizefreq method
        do
        {
            int flt = 1;
            datalist.clear();
            datalist.append(c_file->get_next_value(QString("lambda component")));
            if (datalist.contains("EOF"))
                return false;
            flt = abs(c_file->get_next_value(QString("lambda fleet")).toInt());
            datalist.append(c_file->get_next_value(QString("lambda phase")));
            datalist.append(c_file->get_next_value(QString("lambda value")));
            datalist.append(c_file->get_next_value(QString("lambda sizefq method")));
            if (datalist.contains("EOF"))
                return false;
            if (datalist.at(0).toInt() != -9999)
                data->getFleet(flt-1)->appendLambda(datalist);
        } while (datalist.at(0).toInt() != -9999);

        temp_int = c_file->get_next_value("additional sd reporting").toInt();
        data->getAddSdReporting()->setActive(temp_int);
        if (temp_int == 1)
        {
            // read 9 more values
            datalist.clear();
            for (i = 0; i < 9; i++)
                datalist.append(c_file->get_next_value());
            data->getAddSdReporting()->setSpecs(datalist);

            datalist.clear();
            temp_int = data->getAddSdReporting()->getNumSelexBins();
            for (i = 0; i < temp_int; i++)
                datalist.append(c_file->get_next_value());
            data->getAddSdReporting()->setSelexBins(datalist);

            datalist.clear();
            temp_int = data->getAddSdReporting()->getNumGrowthBins();
            for (i = 0; i < temp_int; i++)
                datalist.append(c_file->get_next_value());
            data->getAddSdReporting()->setGrowthBins(datalist);

            datalist.clear();
            temp_int = data->getAddSdReporting()->getNumNatAgeBins();
            for (i = 0; i < temp_int; i++)
                datalist.append(c_file->get_next_value());
            data->getAddSdReporting()->setNatAgeBins(datalist);
        }
        if (QString(datalist.last()).compare("EOF") == 0)
            return false;

        temp_int = c_file->get_next_value().toInt();
        if (temp_int != 999)
        {
            temp_string = QString("Problem reading control file. The end-of-data token '%1' does not match '999'").arg(
                        QString::number(temp_int));
            c_file->error(temp_string);
        }

        c_file->close();
    }
    else
    {
        c_file->error(QString("Control file does not exist or is unreadable."));
    }
    return 1;
}

int write33_controlFile(ss_file *c_file, ss_model *data)
{
    int temp_int, num, num_vals, chars = 0;
    int i, j, index;
    float temp_float;
    QString line, temp_string;
    QStringList str_list;
    population * pop = data->pPopulation;

    if(c_file->open(QIODevice::WriteOnly))
    {
//        chars += c_file->writeline(QString("#V%1").arg(getDatafileVersionString()));
        chars += writeVersionComment(c_file);
        chars += c_file->write_comments();

        line = QString(QString("#_data_and_control_files: %1 // %2").arg(
                           data->getDataFileName(),
                           data->getControlFileName()));
        chars += c_file->writeline(line);
        line = QString("#");
        chars += c_file->writeline(line);

        // read wtatage.ss
        chars += c_file->write_val(data->getReadWtAtAge(), 1,
                    QString(" 0 means do not read wtatage.ss; 1 means read and use wtatage.ss and also read and use growth parameters"));

        // growth patterns
        num = pop->Grow()->getNum_patterns();
        line = QString(QString ("%1 #_N_Growth_Patterns").arg (
                           num));
        chars += c_file->writeline(line);
        line.clear();
        // morphs or platoons
        num = pop->Grow()->getNum_morphs();
        line.append(QString("%1 ").arg(
                         QString::number(num)));

        line.append(QString("#_N_platoons_Within_GrowthPattern "));
        chars += c_file->writeline(line);
        line.clear();
        if (num > 1)
        {
            temp_float = pop->Grow()->getMorph_within_ratio();
            line.append(QString("%1 ").arg(QString::number(temp_float)));
            line.append(QString("#_Morph_between/within_stdev_ratio (no read if N_morphs=1)"));
            chars += c_file->writeline(line);
            line.clear();
            for (int i = 0; i < num; i++)
            {
                temp_float = pop->Grow()->getMorph_dist(i);
                line.append(QString("%1 ").arg(
                         QString::number(temp_float)));
            }
            line.append(QString("#vector_Morphdist_(-1_in_first_val_gives_normal_approx)"));
            chars += c_file->writeline(line);
            chars += c_file->writeline("#");
        }
        else
        {
            line.append(QString("#_Cond 1 "));
            line.append(QString("#_Morph_between/within_stdev_ratio (no read if N_morphs=1)"));
            chars += c_file->writeline(line);
            line.clear();
            line.append(QString("#_Cond 1 "));
            line.append(QString("# vector_Morphdist_(-1_in_first_val_gives_normal_approx)"));
            chars += c_file->writeline(line);
            chars += c_file->writeline("#");
        }

        // recruitment designs
        line.clear();
        temp_int = pop->SR()->getDistribMethod();
        line = QString (QString("%1 # recr_dist_method for parameters:").arg(QString::number(temp_int)));
        line.append(QString(" 1=like 3.24; 2=main effects for GP, Settle timing, Area; 3=each Settle entity;"));
        chars += c_file->writeline(line);
        temp_int = pop->SR()->getDistribArea();
        line = QString (QString("%1 # Recruitment: 1=global; 2=by area (future option)").arg(QString::number(temp_int)));
        chars += c_file->writeline(line);
        num = pop->SR()->getNumAssignments();
        line = QString (QString("%1 # number of recruitment settlement assignments ").arg(QString::number(num)));
        chars += c_file->writeline(line);
        temp_int = pop->SR()->getDoRecruitInteract()? 1: 0;
        line = QString (QString("%1 # unused quantity (formerly recr_dist_inx)").arg(QString::number(temp_int)));
        chars += c_file->writeline(line);
        line = QString ("#GPat month area age (for each settlement assignment)");
        chars += c_file->writeline(line);
        for (i = 0; i < num; i++)
        {
//            line.clear();
            str_list = pop->SR()->getAssignment(i);
            chars += c_file->write_vector(str_list, 2);
/*            for (int j = 0; j < str_list.count(); j++)
                line.append(QString(" %1").arg(str_list.at(j)));

            chars += c_file->writeline(line);*/
        }
        chars += c_file->writeline("#");

        // movement definitions
        line.clear();
        if (data->get_num_areas() > 1)
        {
            num = pop->Move()->getNumDefs();
            line = QString(QString("%1 #_N_movement_definitions").arg(
                            QString::number(num)));
            chars += c_file->writeline(line);
            temp_float = pop->Move()->getFirstAge();
            line = QString(QString("%1 # first age that moves (real age at begin of season, not integer)").
                           arg(QString::number(temp_float)));
            chars += c_file->writeline(line);
            line = QString("# seas,GP,source_area,dest_area,minage,maxage");
            chars += c_file->writeline(line);
            for (int i = 0; i < num; i++)
            {
//                line.clear();
                str_list = pop->Move()->getDefinition(i);
                chars += c_file->write_vector(str_list, 4);
/*                for (int j = 0; j < str_list.count(); j++) // should be 6
                {
                    line.append(QString(" %1").arg(str_list.at(j)));
                }
                chars += c_file->writeline(line);*/
            }
        }
        else
        {
            line = QString("#_Cond 0 #_N_movement_definitions goes here if N_areas > 1");
            chars += c_file->writeline(line);
            line = QString("#_Cond 1.0 # first age that moves (real age at begin of season, not integer) also cond on do_migration>0");
            chars += c_file->writeline(line);
            line = QString("#_Cond 1 1 1 2 4 10 # example move definition for seas=1, morph=1, source=1 dest=2, age1=4, age2=10");
            chars += c_file->writeline(line);
        }
        chars += c_file->writeline("#");


        // time block patterns
        num = data->getNumBlockPatterns();
        line = QString(QString("%1 #_N_block_Patterns").arg (
                           num));
        chars += c_file->writeline(line);
        line.clear();
        for (int i = 0; i < num; i++)
        {
            BlockPattern *blk = data->getBlockPattern(i);
            temp_int = blk->getNumBlocks();
            line.append(QString("%1 ").arg(temp_int));
        }
        if (line.isEmpty())
        {
            line.append("#_Cond 0 no time blocks defined ");
            chars += c_file->writeline(line);
        }
        else
        {
            line.append(QString("#_N_blocks_per_pattern "));
            chars += c_file->writeline(line);
            line = QString("# begin and end years of blocks");
            chars += c_file->writeline(line);
            line.clear();
            for (int i = 0; i < num; i++)
            {
                line.clear();
                BlockPattern *blk = data->getBlockPattern(i);
                temp_int = blk->getNumBlocks();
                for (int j = 0; j < temp_int; j++)
                    line.append(blk->getBlockText(j));
                line.append(QString(" # pattern %1").arg(QString::number(i+1)));
                chars += c_file->writeline(line);
            }
        }
        chars += c_file->writeline("#");

        // controls for time-varying parameters
        line = QString("# controls for all timevary parameters ");
        chars += c_file->writeline(line);
        temp_int = pop->Grow()->getTimeVaryMethod();
        line = QString(QString("%1 #_env/block/dev_adjust_method for all time-vary parms (1=warn relative to base parm bounds; 3=no bound check)").arg (
                           QString::number(temp_int)));
        chars += c_file->writeline(line);
        line.clear();
        temp_int = pop->Grow()->getTimeVaryReadParams();
        line.append(QString("%1 ").arg(QString::number(temp_int)));
        temp_int = pop->SR()->getTimeVaryReadParams();
        line.append(QString("%1 ").arg(QString::number(temp_int)));
        temp_int = data->getFleet(0)->getQTimeVaryReadParams();
        line.append(QString("%1 ").arg(QString::number(temp_int)));
        temp_int = data->getTagTimeVaryReadParams();
        line.append(QString("%1 ").arg(QString::number(temp_int)));
        temp_int = data->getFleet(0)->getSelTimeVaryReadParams();
        line.append(QString("%1 ").arg(QString::number(temp_int)));
        line.append(QString("# autogen"));//0=autogenerate time-varying parameters; 1=read each time-varying parameter line"));
        chars += c_file->writeline(line);
        line = QString("# where: 0 = autogen all time-varying parms; 1 = read each time-varying parm line; 2 = read then autogen if min=-12345");
        chars += c_file->writeline(line);
        line = QString("# 1st element for biology, 2nd for SR, 3rd for Q, 4th reserved, 5th for selex");
        chars += c_file->writeline(line);
        chars += c_file->writeline("#");
        line = QString("# setup for M, growth, maturity, fecundity, recruitment distibution, movement ");
        chars += c_file->writeline(line);
        chars += c_file->writeline("#");

        // natM and growth
        temp_int = pop->Grow()->getNatural_mortality_type();
        line = QString(QString("%1 #_natM_type:_0=1Parm; 1=N_breakpoints;_2=Lorenzen;_3=agespecific;_4=agespec_withseasinterpolate").arg(
                           temp_int));
        chars += c_file->writeline (line);
        switch (temp_int)
        {
        case 0:
            line = QString ("  #_no additional input for selected M option; read 1P per morph");
            chars += c_file->writeline(line);
            num_vals = 1;
            break;
        case 1:
            num_vals = pop->Grow()->getNatMortNumBreakPts();
            line = QString("%1 #_N_breakpoints").arg(QString::number(num));
            chars += c_file->writeline(line);
            line.clear();
            str_list = pop->Grow()->getNatMortBreakPts();
            chars += c_file->write_vector(str_list, 2, QString("age(real) at M breakpoints"));
            break;
        case 2:
            num = pop->Grow()->getNaturalMortLorenzenRef();
            line = QString("%1 #_Lorenzen ref age ").arg(num);
            chars += c_file->writeline(line);
            num_vals = 1;
            break;
        case 3:
        case 4:
            line.clear();
            str_list = pop->Grow()->getNatMortAges();
            num_vals = pop->Grow()->getNum_patterns();
            for (i = 0; i < num_vals; i++)
            {
                str_list = pop->Grow()->getPattern(i)->getNatMFemAgeList();
                chars += c_file->write_vector(str_list, 2, QString("_Fem_M_GP%1").arg(QString::number(i+1)));
            }
            if (data->get_num_genders() > 1)
                for (int j = 0; j < num_vals; j++, i++)
                {
                    str_list = pop->Grow()->getPattern(i)->getNatMMaleAgeList();
                    chars += c_file->write_vector(str_list, 2, QString("_Male_M_GP%1").arg(QString::number(i+1)));
                }
            num_vals = 0;
            break;
        }

        // growth model
        temp_int = pop->Grow()->getModel();
        line = QString(QString("%1 # GrowthModel: 1=vonBert with L1&L2; 2=Richards with L1&L2; 3=age_specific_K; 4=not implemented").arg(
                           temp_int));
        chars += c_file->writeline (line);
        temp_float = pop->Grow()->getAge_for_l1();
        line = QString(QString("%1 #_Growth_Age_for_L1").arg(
                           QString::number(temp_float)));
        chars += c_file->writeline (line);
        temp_float = pop->Grow()->getAge_for_l2();
        line = QString(QString("%1 #_Growth_Age_for_L2 (999 to use as Linf)").arg(
                           QString::number(temp_float)));
        chars += c_file->writeline (line);
        if (temp_int == 3)
        {
            temp_float = pop->Grow()->getAgeMin_for_K();
            line = QString(QString("%1 #_Min age for age-specific K").arg(
                               QString::number(temp_float)));
            chars += c_file->writeline (line);
            temp_float = pop->Grow()->getAgeMax_for_K();
            line = QString(QString("%1 #_Max age for age-specific K").arg(
                               QString::number(temp_float)));
            chars += c_file->writeline (line);
        }
        temp_float = pop->Grow()->getExpDecay();
        line = QString(QString("%1 #_exponential decay for growth above maxage (fixed at 0.2 in 3.24; value should approx initial Z; -999 replicates 3.24)").arg (
                           QString::number(temp_float)));
        chars += c_file->writeline (line);

        temp_float = pop->Grow()->getFeature();
        line = QString(QString("%1 #_placeholder for future growth feature").arg(
                           QString::number(temp_float)));
        chars += c_file->writeline (line);

        temp_float = pop->Grow()->getSd_add();
        line = QString(QString("%1 #_SD_add_to_LAA (set to 0.1 for SS2 V1.x compatibility)").arg(
                           temp_float));
        chars += c_file->writeline (line);
        temp_int = pop->Grow()->getCv_growth_pattern();
        line = QString(QString("%1 #_CV_Growth_Pattern:  0 CV=f(LAA); 1 CV=F(A); 2 SD=F(LAA); 3 SD=F(A); 4 logSD=F(A)").arg(
                           temp_int));
        chars += c_file->writeline (line);
        temp_int = pop->Grow()->getMaturity_option();
        line = QString(QString("%1 #_maturity_option:  1=length logistic; 2=age logistic; 3=read age-maturity matrix by growth_pattern; 4=read age-fecundity; 5=disabled; 6=read length-maturity").arg(
                           temp_int));
        chars += c_file->writeline (line);
        if (temp_int == 3 ||  // age specific maturity
            temp_int == 4)
        {
            line.clear();
            str_list = pop->Grow()->getMatAgeVals();
            for (int i = 0; i < str_list.count(); i++)
                line.append(QString(" %1").arg(str_list.at(i).toFloat()));

            chars += c_file->writeline (line);
        }
        else if (temp_int == 6)  // length specific maturity
        {
            line.clear();
            str_list = pop->Grow()->getMatAgeVals();
            for (int i = 0; i < str_list.count(); i++)
                line.append(QString(" %1").arg(str_list.at(i).toFloat()));

            chars += c_file->writeline (line);
        }
/*        else
        {
            line = QString("#_placeholder for empirical age-maturity by growth pattern");
            chars += c_file->writeline (line);
        }*/

        temp_float = pop->Grow()->getFirst_mature_age();
        line = QString(QString("%1 #_First_Mature_Age").arg(
                           QString::number(temp_float)));
        chars += c_file->writeline (line);
        temp_int = pop->Fec()->getMethod();
        line = QString(QString("%1 #_fecundity option:(1)eggs=Wt*(a+b*Wt);(2)eggs=a*L^b;(3)eggs=a*Wt^b; (4)eggs=a+b*L; (5)eggs=a+b*W").arg(
                           temp_int));
        chars += c_file->writeline (line);
        temp_int = pop->Fec()->getHermaphroditism();
        line = QString(QString("%1 #_hermaphroditism option:  0=none; 1=female-to-male age-specific fxn; -1=male-to-female age-specific fxn").arg(
                           temp_int));
        chars += c_file->writeline (line);
        if (temp_int == 1)
        {
            temp_float = pop->Fec()->getHermSeason();
            line = QString(QString("%1 #_hermaphroditism Season:  -1 trans at end of each seas; or specific seas").arg(
                               QString::number(temp_float)));
            chars += c_file->writeline (line);
            temp_int = pop->Fec()->getHermIncludeMales();
            line = QString(QString("%1 #_include males in spawning biomass:  0=no males; 1=add males to females; xx=reserved.").arg(
                               temp_int));
            chars += c_file->writeline (line);
        }
        temp_int = pop->Grow()->getParam_offset_method();
        line = QString(QString("%1 #_parameter_offset_approach (1=none, 2= M, G, CV_G as offset from female-GP1, 3=like SS2 V1.x)").arg(
                           temp_int));
        chars += c_file->writeline (line);
        chars += c_file->writeline("#");

        // growth parameters
        line = QString("#_growth_parms");
        chars += c_file->writeline(line);
        line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE env_var&link dev_link dev_minyr dev_maxyr dev_PH Block Block_Fxn");
        chars += c_file->writeline(line);
        num = pop->Grow()->getNum_patterns();
        for (i = 0; i < num; i++)
        {
            growthPattern *gp = pop->Grow()->getPattern(i);
            QString gpstr (QString("GP_%1").arg(QString::number(i + 1)));
            QString genstr, parstr;
            if (data->get_num_genders() > 1)
                num_vals = 2;
            else
                num_vals = 1;
            {
                int numpar = 0;
                switch (pop->Grow()->getNatural_mortality_type())
                {
                case 0:
                    numpar = 1;
                    break;
                case 1:
                    numpar = pop->Grow()->getNatMortNumBreakPts();
                    break;
                case 2:
                    numpar = 2;
                    break;
                default:
                    numpar = 0;
                }

/*                if (pop->Grow()->getNatural_mortality_type() == 1)
                    numpar = pop->Grow()->getNatMortNumBreakPts();
                else if (pop->Grow()->getNatural_mortality_type() > 2)
                    numpar = 0;*/
                genstr = QString ("Fem");
//                numpar *= data->get_num_genders() > 1? 2: 1;//gp->getNumNatMParams()/num_vals;
                for (int k = 0; k < numpar; k++)
                {
                    parstr = QString (gp->getNatMParams()->getRowHeader(k));
                    line.clear();
                    str_list = gp->getNatMParam(k);
                    for (int l = 0; l < str_list.count(); l++)
                        line.append(QString(" %1").arg(str_list.at(l)));
                    line.append(QString(" # %1").arg (parstr));
                    chars += c_file->writeline (line);
                }
                for (int k = 0; k < gp->getNumGrowthParams()/num_vals; k++)
                {
                    parstr = QString(gp->getGrowthParams()->getRowHeader(k));
                    line.clear();
                    str_list = gp->getGrowthParam(k);
                    for (int l = 0; l < str_list.count(); l++)
                        line.append(QString(" %1").arg(str_list.at(l)));
                    line.append(QString(" # %1").arg (parstr));
                    chars += c_file->writeline (line);
                }
                for (int k = 0; k < gp->getNumCVParams()/num_vals; k++)
                {
                    parstr = QString(gp->getCVParams()->getRowHeader(k));
                    line.clear();
                    str_list = gp->getCVParam(k);
                    for (int l = 0; l < str_list.count(); l++)
                        line.append(QString(" %1").arg(str_list.at(l)));
                    line.append(QString(" # %1").arg (parstr));
                    chars += c_file->writeline (line);
                }
                for (int k = 0; k < 2; k++)
                {
                    parstr = QString(pop->Grow()->getWtLenParams()->getRowHeader(k));
                    line.clear();
                    str_list = pop->Grow()->getWtLenParam(k);
                    for (int l = 0; l < str_list.count(); l++)
                        line.append(QString(" %1").arg(str_list.at(l)));
                    line.append(QString(" # %1").arg (parstr));
                    chars += c_file->writeline (line);
                }
                for (int k = 0; k < pop->Fec()->getFemaleParams()->rowCount(); k++)
                {
                    line.clear();
                    str_list = pop->Fec()->getFemaleParam(k);
                    for (int l = 0; l < str_list.count(); l++)
                        line.append(QString(" %1").arg(str_list.at(l)));
                    line.append(QString(" # %1").arg(pop->Fec()->getFemaleParams()->getRowHeader(k)));
                    chars += c_file->writeline (line);
                }

                if (num_vals > 1)
                {
                    genstr = QString ("Male");
                    for (int k = gp->getNumNatMParams()/2; k < gp->getNumNatMParams(); k++)
                    {
                        parstr = QString (gp->getNatMParams()->getRowHeader(k));
                        line.clear();
                        str_list = gp->getNatMParam(k);
                        for (int l = 0; l < str_list.count(); l++)
                            line.append(QString(" %1").arg(str_list.at(l)));
                        line.append(QString(" # %1").arg (parstr));
                        chars += c_file->writeline (line);
                    }
                    for (int k = gp->getNumGrowthParams()/num_vals; k < gp->getNumGrowthParams(); k++)
                    {
                        parstr = QString(gp->getGrowthParams()->getRowHeader(k));
                        line.clear();
                        str_list = gp->getGrowthParam(k);
                        for (int l = 0; l < str_list.count(); l++)
                            line.append(QString(" %1").arg(str_list.at(l)));
                        line.append(QString(" # %1").arg (parstr));
                        chars += c_file->writeline (line);
                    }
                    for (int k = gp->getNumCVParams()/num_vals; k < gp->getNumCVParams(); k++)
                    {
                        parstr = QString(gp->getCVParams()->getRowHeader(k));
                        line.clear();
                        str_list = gp->getCVParam(k);
                        for (int l = 0; l < str_list.count(); l++)
                            line.append(QString(" %1").arg(str_list.at(l)));
                        line.append(QString(" # %1").arg (parstr));
                        chars += c_file->writeline (line);
                    }
                    for (int k = 2; k < 4; k++)
                    {
                        parstr = QString(pop->Grow()->getWtLenParams()->getRowHeader(k));
                        line.clear();
                        str_list = pop->Grow()->getWtLenParam(k);
                        for (int l = 0; l < str_list.count(); l++)
                            line.append(QString(" %1").arg(str_list.at(l)));
                        line.append(QString(" # %1").arg (parstr));
                        chars += c_file->writeline (line);
                    }
                }
            }
        }

        if (pop->Fec()->getHermaphroditism() != 0)
        {
            for (i = 0; i < 3; i++)
            {
                line.clear();
                str_list = pop->Fec()->getHermParam(i); // hermaph_inflect, sd, asymptotic
                for (int l = 0; l < str_list.count(); l++)
                    line.append(QString(" %1").arg(str_list.at(l)));
                line.append(QString(" # Hermaph_p_%1").arg(QString::number(i + 1)));
                chars += c_file->writeline (line);
            }
        }

        num_vals = pop->SR()->getNumDistParams();
        for (i = 0; i < num_vals; i++)
        {
            chars += c_file->write_vector(pop->SR()->getDistParam(i), 2,
                              pop->SR()->getDistParams()->getRowHeader(i));
        }
/*        num_vals = pop->Grow()->getNum_patterns();
        for (i = 0; i < num_vals; i++, index++)
        {
        }
        num_vals = data->get_num_areas();
        for (i = 0; i < num_vals; i++, index++)
        {
            chars += c_file->write_vector(pop->SR()->getDistParam(index), 2,
                              pop->SR()->getDistParams()->getRowHeader(index));
        }
        num = pop->SR()->getDistribMethod();
        num_vals = pop->SR()->getNumAssignments();
        if (num == 2)
        {
            chars += c_file->write_vector(pop->SR()->getDistParam(index), 2,
                               pop->SR()->getDistParams()->getRowHeader(index++));
        }
        else if (num == 3 || (num == 4 && (num_vals * pop->Grow()->getNum_patterns() > 1)))
        {
            for (i = 0; i < pop->SR()->getNumAssignments(); i++, index++)
            {
                chars += c_file->write_vector(pop->SR()->getDistParam(index), 1,
                                 pop->SR()->getDistParams()->getRowHeader(index));
            }
        }*/

        if (pop->SR()->getDoRecruitInteract())
        {
            num = pop->SR()->getNumInteractParams();
            for (i = 0; i < num; i++)
            {
                line.clear();
                str_list = pop->SR()->getInteractParam(i);
                for (int l = 0; l < str_list.count(); l++)
                    line.append(QString(" %1").arg(str_list.at(l)));
                line.append(" # ");
                line.append(pop->SR()->getInteractParams()->getRowHeader(i));
                chars += c_file->writeline (line);
            }
        }
        line.clear();
        str_list = pop->Grow()->getCohortParam();
        for (int l = 0; l < str_list.count(); l++)
            line.append(QString(" %1").arg(str_list.at(l)));
        line.append(QString(" # CohortGrowDev"));
        chars += c_file->writeline (line);

        // movement parameters
        num = pop->Move()->getNumDefs();
        for (i = 0; i < num; i++)
        {
            str_list = pop->Move()->getDefinition(i);
            QString seas = str_list.at(0);
            QString gp = str_list.at(1);
            QString from = str_list.at(2);
            QString to = str_list.at(3);
            int par = i * 2;
            line.clear();
            str_list = pop->Move()->getParameter(par);
            for (int l = 0; l < str_list.count(); l++)
                line.append(QString(" %1").arg(str_list.at(l))); ;
            line.append(QString(" # %1").arg(pop->Move()->getMovementParams()->getRowHeader(par)));
            chars += c_file->writeline (line);
            line.clear();
            str_list = pop->Move()->getParameter(par + 1);
            for (int l = 0; l < str_list.count(); l++)
                line.append(QString(" %1").arg(str_list.at(l)));
            line.append(QString(" # %1").arg(pop->Move()->getMovementParams()->getRowHeader(par+1)));
            chars += c_file->writeline (line);
        }

        //ageing error parameters
        if (data->get_age_composition()->getUseParameters())
        {
            for (i = 0; i < 7; i++)
            {
                line.clear();
                str_list = data->get_age_composition()->getErrorParam(i);
                for (int l = 0; l < str_list.count(); l++)
                    line.append(QString(" %1").arg(str_list.at(l)));
                line.append(QString(" # AgeKeyParm%1").arg(QString::number(i+1)));
                chars += c_file->writeline (line);
            }
        }

        // Fraction Female
        num = pop->Grow()->getNum_patterns();
        for (i = 0; i < num; i++)
        {
            line.clear();
            str_list = pop->Grow()->getPattern(i)->getFractionFemaleParam();
            for (int l = 0; l < str_list.count(); l++)
                line.append(QString(" %1").arg(str_list.at(l)));
            line.append(QString(" # FracFemale_GP_%1").arg(QString::number(i + 1)));
            chars += c_file->writeline (line);
        }

        // time varying MG parameters
        chars += c_file->writeline ("#");
        line = QString ("# timevary MG parameters ");
        chars += c_file->writeline (line);
        line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE");
        chars += c_file->writeline (line);
        temp_int = 0;
        if (pop->Grow()->getTimeVaryReadParams())
        {
        for (i = 0; i < pop->Grow()->getNum_patterns(); i++)
        {
            tablemodel *parmmodel;
            int natmIndex = 0, natmTot = 0;
            int grthIndex = 0, grthTot = 0;
            int wtlnIndex = 0, wtlnTot = 0;
            num_vals = (data->get_num_genders() > 1)? 2: 1;
//            tablemodel *params;
            QStringList param;
            QString headr;
            growthPattern * gp = pop->Grow()->getPattern(i);
//            params = gp->getNatMParams();
//            num = params->rowCount();
            // female fish
            parmmodel = gp->getNatMTVParams();
            natmTot = gp->getNumNatMTVParams();//parmmodel->getNumParamVars();
            for (natmIndex = 0; natmIndex < natmTot/num_vals; natmIndex++)
            {
                param = gp->getNatMTVParam(natmIndex);//parmmodel->getParamVarData(natmIndex);
                headr = gp->getNatMTVParams()->getRowHeader(natmIndex);//parmmodel->getParamVarHeader(natmIndex);
                chars += c_file->write_vector(param, 2, headr);
            }
            parmmodel = gp->getGrowthTVParams();//getGrowthParamModel();
            grthTot = parmmodel->rowCount();
            for (grthIndex = 0; grthIndex < grthTot/num_vals; grthIndex++)
            {
                param = parmmodel->getRowData(grthIndex);
                headr = parmmodel->getRowHeader(grthIndex);
                chars += c_file->write_vector(param, 2, headr);
            }
            parmmodel = pop->Grow()->getWtLenTVParams();
            wtlnTot = parmmodel->rowCount();
            for (wtlnIndex = 0; wtlnIndex < wtlnTot/num_vals; wtlnIndex++)
            {
                param = parmmodel->getRowData(wtlnIndex);
                headr = parmmodel->getRowHeader(wtlnIndex);
                chars += c_file->write_vector(param, 2, headr);
            }
            // male fish
            if (num_vals == 2)
            {
                parmmodel = gp->getNatMTVParams();
//                num = parmmodel->getNumParamVars();
                for (; natmIndex < natmTot; natmIndex++)
                {
                    param = parmmodel->getRowData(natmIndex);
                    headr = parmmodel->getRowHeader(natmIndex);
                    chars += c_file->write_vector(param, 2, headr);
                }
                temp_int += natmTot;
                parmmodel = gp->getGrowthTVParams();
//                num = parmmodel->getNumParamVars();
                for (; grthIndex < grthTot; grthIndex++)
                {
                    param = parmmodel->getRowData(grthIndex);
                    headr = parmmodel->getRowHeader(grthIndex);
                    chars += c_file->write_vector(param, 2, headr);
                }
                temp_int += grthTot;
                parmmodel = pop->Grow()->getWtLenTVParams();
//                num = parmmodel->getNumParamVars();
                for (; wtlnIndex < wtlnTot; wtlnIndex++)
                {
                    param = parmmodel->getRowData(wtlnIndex);
                    headr = parmmodel->getRowHeader(wtlnIndex);
                    chars += c_file->write_vector(param, 2, headr);
                }
                temp_int += wtlnTot;
            }
        }

        }

        else
        {
            line = QString("#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no time-vary parameters");
            chars += c_file->writeline (line);
        }
        line = QString ("# info on dev vectors created for MGparms are reported with other devs after tag parameter section ");
        chars += c_file->writeline (line);
        chars += c_file->writeline ("#");

        line = QString("#_seasonal_effects_on_biology_parms");
        chars += c_file->writeline(line);
        line.clear();
        str_list = pop->getSeasParamSetup();
        chars += c_file->write (QByteArray(" "));
        chars += c_file->write_vector(str_list, 2, "femwtlen1,femwtlen2,mat1,mat2,fec1,fec2,Malewtlen1,malewtlen2,L1,K");
/*        line.append(QString(" %1").arg(QString::number(pop->getFemWtLen1())));
        line.append(QString(" %1").arg(QString::number(pop->getFemWtLen2())));
        line.append(QString(" %1").arg(QString::number(pop->getMaturity1())));
        line.append(QString(" %1").arg(QString::number(pop->getMaturity2())));
        line.append(QString(" %1").arg(QString::number(pop->getFecund1())));
        line.append(QString(" %1").arg(QString::number(pop->getFecund2())));
        line.append(QString(" %1").arg(QString::number(pop->getMaleWtLen1())));
        line.append(QString(" %1").arg(QString::number(pop->getMaleWtLen2())));
        line.append(QString(" %1").arg(QString::number(pop->getL1())));
        line.append(QString(" %1").arg(QString::number(pop->getK())));
        line.append(QString(" #_femwtlen1,femwtlen2,mat1,mat2,fec1,fec2,Malewtlen1,malewtlen2,L1,K"));
        chars += c_file->writeline(line);*/

        line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE");
        chars += c_file->writeline(line);
        num = pop->getNumSeasParams();
        if (num == 0)
        {
            line = QString("#_Cond -2 2 0 0 -1 99 -2 #_placeholder when no seasonal MG parameters");
            chars += c_file->writeline(line);
        }
        else
        {
            for (i = 0; i < num; i++)
            {
                line.clear();
                str_list = pop->getSeasonalParam(i);
                chars += c_file->write_vector(str_list, 2, QString("MG-environ param %1").arg(QString::number(i)));
/*                for (int l = 0; l < str_list.count(); l++)
                    line.append(QString(" %1").arg(str_list.at(l)));
                line.append(QString(" # MG-environ param %1").arg(QString::number(i)));
                chars += c_file->writeline (line);*/
            }
        }
        chars += c_file->writeline ("#");

        // Spawner-recruitment
        line = QString("#_Spawner-Recruitment");
        chars += c_file->writeline(line);
        line = QString(QString("%1 #_SR_function").arg(
                           QString::number(pop->SR()->getMethod())));
        line.append(": 2=Ricker; 3=std_B-H; 4=SCAA; 5=Hockey; 6=B-H_flattop; 7=survival_3Parm; 8=Shepard_3Parm");
        chars += c_file->writeline(line);
        line = QString(QString("%1 # 0/1 to use steepness in initial equ recruitment calculation").arg(
                           QString::number(pop->SR()->getUseSteepness())));
        chars += c_file->writeline(line);
        line = QString(QString("%1 # future feature:  0/1 to make realized sigmaR a function of SR curvature").arg(
                           QString::number(pop->SR()->getFeature())));
        chars += c_file->writeline(line);

        line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE env-var use_dev dev_mnyr dev_mxyr dev_PH Block Blk_Fxn  #  parm_name");
        chars += c_file->writeline(line);

        num = 0;
        {
        tablemodel *params = pop->SR()->getFullParameters();
        num = params->rowCount();
        for (j = 0; j < num; j++)
        {
            chars += c_file->write_vector(params->getRowData(j), 4, params->getRowHeader(j));
        }
        }
        if (pop->SR()->getTimeVaryReadParams())
        {
            timeVaryParameterModel *params = pop->SR()->getTVParameterModel();
            num = params->getNumVarParams();
            if (num > 0)
            {
                line = QString("#_Time-varying recruitment params");
                chars += c_file->writeline(line);
                line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE  #  parm_name");
                chars += c_file->writeline(line);
                for (j = 0; j < num; j++)
                {
                    chars += c_file->write_vector(params->getVarParameter(j), 4, params->getVarParamHeader(j));
                }
            }
            else
            {
                line = QString("#_No time-vary recruitment params");
                chars += c_file->writeline(line);
            }
        }
/*        line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
        line.append(pop->SR()->full_parameters->getRowHeader(num++));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
        line.append(pop->SR()->full_parameters->getRowHeader(num++));
        chars += c_file->writeline(line);
        if (pop->SR()->method == 5 ||
                pop->SR()->method == 7 ||
                pop->SR()->method == 8)
        {
            line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
            line.append(pop->SR()->full_parameters->getRowHeader(num++));
            chars += c_file->writeline(line);
        }
        line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
        line.append(pop->SR()->full_parameters->getRowHeader(num++));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
        line.append(pop->SR()->full_parameters->getRowHeader(num++));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # ").arg(pop->SR()->full_parameters->getRowText(num)));
        chars += c_file->writeline(line);
        line.append(pop->SR()->full_parameters->getRowHeader(num++));

        line = QString("#Next are short parm lines, if requested, for env effects on R0, steepness, and annual dev");
        chars += c_file->writeline(line);
        line = QString("#Then short parm lines, if requested, for block/trend effects on R0, steepness, and annual dev");
        chars += c_file->writeline(line);*/

        line = QString(QString ("%1 #do_recdev:  0=none; 1=devvector; 2=simple deviations").arg(
                           QString::number(pop->SR()->getRecDevCode())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # first year of main recr_devs; early devs can preceed this era").arg(
                           QString::number(pop->SR()->getRecDevStartYr())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # last year of main recr_devs; forecast devs start in following year").arg(
                           QString::number(pop->SR()->getRecDevEndYr())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 #_recdev phase ").arg(
                           QString::number(pop->SR()->getRecDevPhase())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # (0/1) to read 13 advanced options").arg(
                           pop->SR()->getAdvancedOpts()? "1":"0"));
        chars += c_file->writeline(line);

        if (pop->SR()->getAdvancedOpts())
        {
            line = QString(QString (" %1 #_recdev_early_start (0=none; neg value makes relative to recdev_start)").arg(
                               QString::number(pop->SR()->getRecDevEarlyStart())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_recdev_early_phase").arg(
                               QString::number(pop->SR()->getRecDevEarlyPhase())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_forecast_recruitment phase (incl. late recr) (0 value resets to maxphase+1)").arg(
                               QString::number(pop->SR()->getFcastRecPhase())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_lambda for Fcast_recr_like occurring before endyr+1").arg(
                               QString::number(pop->SR()->getFcastLambda())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_last_early_yr_nobias_adj_in_MPD").arg(
                               QString::number(pop->SR()->getNobiasLastEarlyYr())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_first_yr_fullbias_adj_in_MPD").arg(
                               QString::number(pop->SR()->getFullbiasFirstYr())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_last_yr_fullbias_adj_in_MPD").arg(
                               QString::number(pop->SR()->getFullbiasLastYr())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_first_recent_yr_nobias_adj_in_MPD").arg(
                               QString::number(pop->SR()->getNobiasFirstRecentYr())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_max_bias_adj_in_MPD (-1 to override ramp and set biasadj=1.0 for all estimated recdevs)").arg(
                               QString::number(pop->SR()->getMaxBiasAdjust())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_period of cycles in recruitment (N parms read below)").arg(
                               QString::number(pop->SR()->getRecCycles())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_min rec_dev").arg(
                               QString::number(pop->SR()->getRecDevMin())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_max rec_dev").arg(
                               QString::number(pop->SR()->getRecDevMax())));
            chars += c_file->writeline(line);
            line = QString(QString (" %1 #_read recdevs").arg(
                               QString::number(pop->SR()->getNumRecDev())));
            chars += c_file->writeline(line);
            line = QString(QString ("#_end of advanced SR options"));
            chars += c_file->writeline(line);
        }
        line = QString("#");
        chars += c_file->writeline(line);

        if (pop->SR()->getRecCycles() == 0)
        {
            line = QString("#_placeholder for full parameter lines for recruitment cycles");
            chars += c_file->writeline(line);
        }
        else
        {
            for (i = 0; i < pop->SR()->getRecCycles(); i++)
            {
                line.clear();
                str_list = pop->SR()->getCycleParam(i);
                for (int j = 0; j < 14; j++)
                    line.append(QString(" %1").arg(str_list.at(j)));
                line.append(QString (" # "));
                chars += c_file->writeline(line);
            }
        }

        line = QString("# read specified recr devs");
        chars += c_file->writeline(line);
        line = QString("#_Yr Input_value");
        chars += c_file->writeline(line);
        for (i = 0; i < pop->SR()->getNumRecDev(); i++)
        {
            str_list = pop->SR()->getRecruitDev(i);
            chars += c_file->write_vector(str_list, 4);
        }
/*        for (std::map<int,float>::iterator itr = pop->SR()->yearly_devs.begin(); itr != pop->SR()->yearly_devs.end(); itr++)
        {
            line.clear();
            temp_int = itr->first;
            temp_float = itr->second;
            line.append(QString("%1 %2").arg(
                               QString::number(temp_int),
                               QString::number(temp_float)));
            chars += c_file->writeline(line);
        }*/
        line = QString("#");
        chars += c_file->writeline(line);
        c_file->newline();

        // mortality
        line = QString("#Fishing Mortality info ");
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # F ballpark").arg(
                           QString::number(pop->M()->getBparkF())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # F ballpark year (neg value to disable)").arg(
                           QString::number(pop->M()->getBparkYr())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # F_Method: 1=Pope; 2=instan. F; 3=hybrid (hybrid is recommended)").arg(
                           QString::number(pop->M()->getMethod())));
        chars += c_file->writeline(line);
        line = QString(QString ("%1 # max F or harvest rate, depends on F_Method").arg(
                           QString::number(pop->M()->getMaxF())));
        chars += c_file->writeline(line);
        line = QString("# no additional F input needed for Fmethod 1");
        chars += c_file->writeline(line);
        line = QString("# if Fmethod=2; read overall start F value; overall phase; N detailed inputs to read");
        chars += c_file->writeline(line);
        line = QString("# if Fmethod=3; read N iterations for tuning for Fmethod 3");
        chars += c_file->writeline(line);

        switch (pop->M()->getMethod())
        {
        case 1:
            break;
        case 2:
            line = QString(QString ("%1 # overall start F value").arg(
                               QString::number(pop->M()->getStartF())));
            chars += c_file->writeline(line);
            line = QString(QString ("%1 # overall phase").arg(
                               QString::number(pop->M()->getPhase())));
            chars += c_file->writeline(line);
            line = QString(QString ("%1 # N detailed inputs").arg(
                               QString::number(pop->M()->getNumInputs())));
            chars += c_file->writeline(line);
            break;
        case 3:
            line = QString(QString ("%1 # N iterations for tuning F in hybrid method (recommend 3 to 7)").arg(
                               QString::number(pop->M()->getNumTuningIters())));
            chars += c_file->writeline(line);
        }

        line = QString ("# Fleet Year Seas F_value se phase (for detailed setup of F_Method=2)");
        chars += c_file->writeline(line);
        temp_int = pop->M()->getNumInputs();
        for (i = 0; i < temp_int; i++)
        {
            str_list = pop->M()->getInputLine(i);
            for (int j = 0; j < str_list.count(); j++)
            {
                line = QString(QString(" %1").arg(str_list.at(j)));
                chars += c_file->writeline(line);
            }
            line = QString(" # ");
            chars += c_file->writeline(line);
        }
        chars += c_file->writeline("#");

        num = pop->M()->getNumInitialParams();
        line = QString(QString("#_initial_F_parms; count = %1").arg (QString::number(num)));
        chars += c_file->writeline(line);
        line = QString("#_ LO HI INIT PRIOR PR_SD PR_type PHASE");
        chars += c_file->writeline(line);
        for (i = 0; i < num; i++)
        {
            str_list = pop->M()->getInitialParam(i);
            temp_string = pop->M()->getInitialParams()->getRowHeader(i);
            temp_int = temp_string.section('t', 1, -1).toInt();
            chars += c_file->write_vector(str_list, 4,
                                          QString("InitF_seas_%1_flt_%2%3").arg
                                          (QString::number(1),
                                           QString::number(temp_int),
                                           data->getFleet(temp_int-1)->getName()));
        }
        line = QString("# ");
        chars += c_file->writeline(line);
        c_file->newline();

        // Q_setup
        line = QString("#_Q_setup");
        chars += c_file->writeline(line);
        line = QString("#_fleet   link link_info extra_se biasadj float # fleetname");
        chars += c_file->writeline(line);
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            if (/*data->getFleet(i)->getType() == Fleet::Survey &&*/
                    data->getFleet(i)->getQSetupRead())
            {
                line = QString(QString("      %1").arg(i + 1));
                line.append(data->getFleet(i)->Q()->getSetup());
                line.append(QString("  #  %1").arg(
                                data->getFleet(i)->getName()));
                chars += c_file->writeline(line);
            }
        }
        line = QString ("  -9999      0      0      0      0      0  #  terminator");
        chars += c_file->writeline(line);
        line = QString ("#");
        chars += c_file->writeline(line);
        // Q_parameters
        line = QString("#_Q_parms(if_any);Qunits_are_ln(q)");
        chars += c_file->writeline(line);
        line = QString("# LO HI INIT PRIOR PR_SD PR_type PHASE env-var use_dev dev_mnyr dev_mxyr dev_PH Block Blk_Fxn # parm_name");
        chars += c_file->writeline(line);
        num = data->get_num_fleets();
        for (int i = 0; i < num; i++)
        {
            Fleet *fleet = data->getFleet(i);
            // Q Base
            if (fleet->getQSetupRead())
            {
                line.clear();
                temp_string = fleet->Q()->getBase();
//                for (int k = 0; k < str_list.count(); k++)
//                    line.append(QString("  ").arg(str_list.at(k)));
                line.append(QString("%1 #  LnQ_base_%2(%3)").arg(
                                temp_string,
                                data->getFleet(i)->getName(),
                                QString::number(i+1)));
                chars += c_file->writeline(line);
            }
            // Q Power
            if (fleet->getQSetupRead() && fleet->Q()->getDoPower())
            {
                line.clear();
                temp_string = fleet->Q()->getPower();
                line.append(QString("%1  #  Q_power_%2(%3)").arg(
                                temp_string,
                                data->getFleet(i)->getName(),
                                QString::number(i+1)));
                chars += c_file->writeline(line);
            }
            // Q Extra SD
            if (fleet->getQSetupRead() && fleet->Q()->getDoExtraSD())
            {
                line.clear();
                temp_string = fleet->Q()->getExtra();
                line.append(QString("%1  #  Q_extraSD_%2(%3)").arg(
                                temp_string,
                                data->getFleet(i)->getName(),
                                QString::number(i+1)));
                chars += c_file->writeline(line);
            }
        }

        // Q_timevary
        line = QString("# timevary Q parameters");
        chars += c_file->writeline(line);
        line = QString("#_      LO        HI      INIT     PRIOR   PR_SD  PR_type      PHASE");
        chars += c_file->writeline(line);
        if (data->getFleet(0)->getQTimeVaryReadParams() > 0)
        {
        for (i = 0; i < data->get_num_fleets(); i++)
        {
            tablemodel *tvParams = data->getFleet(i)->Q()->getTVParams();
            int num = tvParams->rowCount();
            for (int j = 0; j < num; j++)
            {
                chars += c_file->write_vector(tvParams->getRowData(j), 4, tvParams->getRowHeader(j));
            }
        }
        }
        line = QString("# info on dev vectors created for Q parms are reported with other devs after tag parameter section ");
        chars += c_file->writeline(line);


        // Selectivity
        line = QString ("#");
        chars += c_file->writeline(line);
        line = QString("#_size_selex_types");
        chars += c_file->writeline(line);
        line = QString("#discard_options:_0=none;_1=define_retention;_2=retention&mortality;_3=all_discarded_dead;_4=define_dome-shaped_retention");
        chars += c_file->writeline(line);
        line = QString("#_Pattern Discard Male Special");
        chars += c_file->writeline(line);
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            chars += c_file->write_vector(data->getFleet(i)->getSizeSelectivity()->getSetup(),
                                 3, QString("%1 %2").arg(QString::number(i + 1),
                                                         data->getFleet(i)->getName()));
        }

        line = QString ("#");
        chars += c_file->writeline(line);
        line = QString("#_age_selex_types");
        chars += c_file->writeline(line);
        line = QString("#_Pattern Discard Male Special");
        chars += c_file->writeline(line);
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            chars += c_file->write_vector(data->getFleet(i)->getAgeSelectivity()->getSetup(),
                                 3, QString("%1 %2").arg(QString::number(i + 1),
                                                         data->getFleet(i)->getName()));
        }
        line = QString ("#");
        chars += c_file->writeline(line);
        line = QString ("#_ LO   HI   INIT  PRIOR PR_SD PR_type PHASE env-var use_dev dev_mnyr dev_mxyr dev_PH Block Blk_Fxn # parm_name");
        chars += c_file->writeline(line);
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            selectivity *slx = data->getFleet(i)->getSizeSelectivity();
            for (int j = 0; j < slx->getNumParameters(); j++)
            {
                chars += c_file->write_vector(slx->getParameter(j), 6, slx->getParameterLabel(j));
            }
            for (int j = 0; j < slx->getNumRetainParameters(); j++)
            {
                chars += c_file->write_vector(slx->getRetainParameter(j), 6, slx->getRetainParameterLabel(j));
            }
            for (int j = 0; j < slx->getNumDiscardParameters(); j++)
            {
                chars += c_file->write_vector(slx->getDiscardParameter(j), 6, slx->getDiscardParameterLabel(j));
            }
            for (int j = 0; j < slx->getNumMaleParameters(); j++)
            {
                chars += c_file->write_vector(slx->getMaleParameter(j), 6, slx->getMaleParameterLabel(j));
            }
        }

        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            selectivity *slx = data->getFleet(i)->getAgeSelectivity();
            for (int j = 0; j < slx->getNumParameters(); j++)
            {
                chars += c_file->write_vector(slx->getParameter(j), 6, slx->getParameterLabel(j));
            }
            for (int j = 0; j < slx->getNumMaleParameters(); j++)
            {
                chars += c_file->write_vector(slx->getMaleParameter(j), 6, slx->getMaleParameterLabel(j));
            }
        }

        line = QString(QString("# timevary selex parameters"));
        chars += c_file->writeline(line);
        line = QString(QString("#_   LO       HI     INIT    PRIOR    PR_SD    PR_type  PHASE"));
        chars += c_file->writeline(line);
        if (data->getFleet(0)->getSelTimeVaryReadParams())
        {
        for (i = 0; i < data->get_num_fleets(); i++)
        { // size selectivity
            selectivity *slx = data->getFleet(i)->getSizeSelectivity();
            num = slx->getNumTimeVaryParameters();
            for (int j = 0; j < num; j++)
            {
                chars += c_file->write_vector(slx->getTimeVaryParameter(j), 8, slx->getTimeVaryParameterLabel(j));
            }
        }
        for (i = 0; i < data->get_num_fleets(); i++)
        { // age selectivity
            selectivity *slx = data->getFleet(i)->getAgeSelectivity();
            num = slx->getNumTimeVaryParameters();
            for (int j = 0; j < num; j++)
            {
                chars += c_file->write_vector(slx->getTimeVaryParameter(j), 8, slx->getTimeVaryParameterLabel(j));
            }
        }
        }

        line = QString(QString("# info on dev vectors created for selex parms are reported with other devs after tag parameter section "));
        chars += c_file->writeline(line);
        chars += c_file->writeline("#");

        // 2D-AR1 smoother
        temp_int = data->getFleet(0)->getAr1SelSmoother();
        chars += c_file->write_val(temp_int, 0, QString("use 2D_AR1 selectivity(0/1):  experimental feature"));
        if (temp_int == 0)
            chars += c_file->writeline(QString("#_no 2D_AR1 selex offset used"));
        chars += c_file->writeline(QString("#"));

        // Tag Recapture Parameters
        temp_int = data->getTagLoss();
        line = QString(QString("# Tag loss and Tag reporting parameters go next"));
        chars += c_file->writeline(line);
        line = QString(QString("%1 # TG_custom:  0=no read; 1=read if tags exist").arg(
                           QString::number(temp_int)));
        chars += c_file->writeline(line);
        if (temp_int == 1)
        {
            line = QString(QString(" %1 # tag loss parameter").arg(
                               data->getTagLossParameter()->toText()));
        }
        else
        {
            line = QString(QString("#_Cond -6 6 1 1 2 0.01 -4 0 0 0 0 0 0 0  #_placeholder if no parameters"));
        }
        chars += c_file->writeline(line);

        chars += c_file->writeline("#");
        temp_int = data->getInputValueVariance();
        line = QString("# Input variance adjustments factors: ");
        chars += c_file->writeline(line);
        line = QString(" #_1=add_to_survey_CV");
        chars += c_file->writeline(line);
        line = QString(" #_2=add_to_discard_stddev");
        chars += c_file->writeline(line);
        line = QString(" #_3=add_to_bodywt_CV");
        chars += c_file->writeline(line);
        line = QString(" #_4=mult_by_lencomp_N");
        chars += c_file->writeline(line);
        line = QString(" #_5=mult_by_agecomp_N");
        chars += c_file->writeline(line);
        line = QString(" #_6=mult_by_size-at-age_N");
        chars += c_file->writeline(line);
        line = QString(" #_7=mult_by_generalized_sizecomp");
        chars += c_file->writeline(line);
        line = QString("#_Factor  Fleet  Value");
        chars += c_file->writeline(line);
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            temp_float = data->getFleet(i)->getAddToSurveyCV();
            if (!floatEquals(temp_float, 0.0))
            {
                line = QString(QString("    1    %1    %2  #_add_to_survey_CV").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
        }
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            temp_float = data->getFleet(i)->getAddToSurveyCV();
            if (!floatEquals(temp_float, 0.0))
            {
                line = QString(QString("    2    %1    %2  #_add_to_discard_stddev").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
            temp_float = data->getFleet(i)->getAddToBodyWtCV();
            if (!floatEquals(temp_float, 0.0))
            {
                line = QString(QString("    3    %1    %2  #_add_to_bodywt_CV").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
            temp_float = data->getFleet(i)->getMultByLenCompN();
            if (!floatEquals(temp_float, 1.0))
            {
                line = QString(QString("    4    %1    %2  #_mult_by_lencomp_N").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
            temp_float = data->getFleet(i)->getMultByAgeCompN();
            if (!floatEquals(temp_float, 1.0))
            {
                line = QString(QString("    5    %1    %2  #_mult_by_agecomp_N").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
            temp_float = data->getFleet(i)->getMultBySAA();
            if (!floatEquals(temp_float, 1.0))
            {
                line = QString(QString("    6    %1    %2  #_mult_by_size-at-age_N").arg(
                                   QString::number(i+1),
                                   QString::number(temp_float)));
                chars += c_file->writeline(line);
            }
        }
        line = QString (" -9999    1     0  # terminator");
        chars += c_file->writeline(line);


        chars += c_file->writeline("#");
        line = QString(QString("%1 #_maxlambdaphase").arg(
                           QString::number(data->getLambdaMaxPhase())));
        chars += c_file->writeline(line);
        line = QString(QString("%2 #_sd_offset").arg(
                           QString::number(data->getLambdaSdOffset())));
        chars += c_file->writeline(line);
        line = QString(QString("# read %1 changes to default Lambdas (default value is 1.0)").arg(QString::number(data->getLambdaNumChanges())));
        chars += c_file->writeline(line);
        line = QString(QString("# Like_comp codes:  1=surv; 2=disc; 3=mnwt; 4=length; 5=age; 6=SizeFreq; 7=sizeage; 8=catch; 9=init_equ_catch; "));
        chars += c_file->writeline(line);
        line = QString(QString("# 10=recrdev; 11=parm_prior; 12=parm_dev; 13=CrashPen; 14=Morphcomp; 15=Tag-comp; 16=Tag-negbin; 17=F_ballpark"));
        chars += c_file->writeline(line);
        line = QString(QString("#like_comp  fleet  phase  value  sizefreq_method"));
        chars += c_file->writeline(line);
        num = 0;
        for (int i = 0; i < data->get_num_fleets(); i++)
        {
            int j = 0;
            for (j = 0; j < data->getFleet(i)->getNumLambdas(); j++)
            {
                line.clear();
                str_list = data->getFleet(i)->getLambda(j);
                line.append(QString(" %1 ").arg(str_list.at(0)));
                line.append(QString::number(i + 1));
                line.append(QString(" %1").arg(str_list.at(1)));
                line.append(QString(" %1").arg(str_list.at(2)));
                line.append(QString(" %1").arg(str_list.at(3)));

                chars += c_file->writeline(line);
            }
            num += j;
        }
        if (num > data->getLambdaNumChanges())
        {
            c_file->error(QString("Problem writing control file. Lambda changes do not match."));
        }
        line = QString (" -9999  1  1  1  1  # terminator");
        chars += c_file->writeline(line);

        temp_int = data->getAddSdReporting()->getActive();
        if (temp_int == 1)
        {
            line = QString("1 # (0/1) read specs for more stddev reporting ");
            chars += c_file->writeline(line);
            line = QString (QString("%1 # selex type, len/age, year, N selex bins, Growth pattern, N growth ages, NatAge_area(-1 for all), NatAge_yr, N Natages").arg(
                                data->getAddVarSetupToText()));
            chars += c_file->writeline(line);
            line.clear();
            str_list = data->getAddSdReprtSelex();
            for (int i = 0; i < str_list.count(); i++)
            {
                if (!str_list.at(i).isEmpty())
                    line.append(QString (" %1").arg(str_list.at(i)));
            }
            line.append(" # vector with selex std bin picks (-1 in first bin to self-generate)");
            chars += c_file->writeline(line);
            line.clear();
            str_list = data->getAddSdReprtGrwth();
            for (int i = 0; i < str_list.count(); i++)
            {
                if (!str_list.at(i).isEmpty())
                    line.append(QString(" %1").arg(str_list.at(i)));
            }
            line.append(" # vector with growth std bin picks (-1 in first bin to self-generate)");
            chars += c_file->writeline(line);
            line.clear();
            str_list = data->getAddSdReprtAtAge();
            for (int i = 0; i < str_list.count(); i++)
            {
                if (!str_list.at(i).isEmpty())
                    line.append(QString(" %1").arg(str_list.at(i)));
            }
            line.append(" # vector with NatAge std bin picks (-1 in first bin to self-generate)");
            chars += c_file->writeline(line);
        }
        else
        {
            line = QString("0 # (0/1) read specs for more stddev reporting ");
            chars += c_file->writeline(line);
            line = QString("#_Cond_0 # 0 1 -1 5 1 5 1 -1 5 # placeholder for selex type, len/age, year, N selex bins, Growth pattern, N growth ages, NatAge_area(-1 for all), NatAge_yr, N Natages");
            chars += c_file->writeline(line);
            line = QString("#_Cond_0 # placeholder for vector of selex bins to be reported");
            chars += c_file->writeline(line);
            line = QString("#_Cond_0 # placeholder for vector of growth ages to be reported");
            chars += c_file->writeline(line);
            line = QString("#_Cond_0 # placeholder for vector of NatAges ages to be reported");
            chars += c_file->writeline(line);
        }

        line = QString::number(END_OF_DATA);
        chars += c_file->writeline(line);
        chars += c_file->writeline();

        c_file->close();
    }
    return chars;
}

bool read33_parameterFile(ss_file *pr_file, ss_model *data)
{
    bool flag = false;
    if(pr_file->open(QIODevice::ReadOnly))
    {
        flag = true;
        pr_file->seek(0);
        pr_file->read_comments();

        data->setALKTol(data->getALKTol());

        pr_file->close();
    }
    else
        pr_file->error("Parameter file does not exist or is not readable.");

    return flag;
}

int write33_parameterFile(ss_file *pr_file, ss_model *data)
{
    int chars = 0;

    if(pr_file->open(QIODevice::WriteOnly))
    {
        pr_file->write_comments();

        data->setALKTol(data->getALKTol());

        pr_file->close();
    }
    else
        pr_file->error("Parameter file is not writeable.");
    return chars;
}

bool read33_userDataFile (ss_file *ud_file, ss_model *data)
{
    bool flag = false;
    if(ud_file->open(QIODevice::ReadOnly))
    {
        flag = true;
        ud_file->seek(0);
        ud_file->read_comments();

        data->setALKTol(data->getALKTol());

        ud_file->close();
    }
    else
        ud_file->error("User data file does not exist or is not readable.");

    return flag;
}

int write33_userDataFile (ss_file *ud_file, ss_model *data)
{
    int chars = 0;

    if(ud_file->open(QIODevice::WriteOnly))
    {
        ud_file->write_comments();

        data->setALKTol(data->getALKTol());

        ud_file->close();
    }
    else
        ud_file->error("User data file is not writeable.");
    return chars;
}

bool read33_profileFile (ss_file *pf_file, ss_model *data)
{
    bool okay;


    if(pf_file->open(QIODevice::ReadOnly))
    {
        pf_file->seek(0);
        pf_file->read_comments();

        data->setALKTol(data->getALKTol());

        pf_file->close();
        okay = true;
    }
    else
    {
        pf_file->error(QString("Profile file does not exist or is not readable."));
        okay = false;
    }

    return okay;
}

int write33_profileFile (ss_file *pf_file, ss_model *data)
{
    int code = 0;

    if(pf_file->open(QIODevice::WriteOnly))
    {
        pf_file->write_comments();

        data->setALKTol(data->getALKTol());

        pf_file->close();
    }
    else
    {
        pf_file->error(QString("Profile file is not writeable."));
        code = 1;
    }

    return code;
}


void readTimeVaryParams (ss_file *infile, ss_model *data, tablemodel *paramTable, int varRead, tablemodel *varParamTable)
{
    QStringList param;
    QString header;

    int value = 0;
    int row = 0;
    if (varRead > 0)
    {
        for (int i = 0; i < paramTable->rowCount(); i++)
        {
            param = paramTable->getRowData(i);
            header = paramTable->getRowHeader(i);
            value = param.at(0).toInt();

            if (varRead > 0)
            {
                // read time varying parameters
                // blocks
                value = param.at(12).toInt();
                if (value != 0)
                    row = readTimeVaryBlockParams (infile, data, value, param.at(13).toInt(), header, row, varParamTable);

                // devs
                value = param.at(8).toInt();
                if (value != 0)
                    row = readTimeVaryDevParams (infile, data, value, header, row, varParamTable);

                // env link
                value = param.at(7).toInt();
                if (value != 0)
                    row = readTimeVaryEnvParams (infile, data, value, header, row, varParamTable);
            }
        }
        varParamTable->setRowCount(row);
    }
}

int readTimeVaryBlockParams(ss_file *infile, ss_model *data, int value, int fnx, QString hdr, int row, tablemodel *varParamTable)
{
    BlockPattern *bp;
    int numBlocks = 0;
    int beg;
    QStringList varParam;
//    QString varHeader(hdr);

    if (value > 0)
    {
        bp = data->getBlockPattern(value-1);
        numBlocks = bp->getNumBlocks();
        for (int i = 0; i < numBlocks; i++, row++)
        {
            beg = bp->getBlockBegin(i);
            varParam = readShortParameter(infile);
            varParamTable->setRowData(row, varParam);
            if (fnx == 0) // mult
            {
                varParamTable->setRowHeader(row, QString("%1_BLK%2mult_%3").arg(hdr, QString::number(i+1), QString::number(beg)));
            }
            else if (fnx == 1) // add
            {
                varParamTable->setRowHeader(row, QString("%1_BLK%2add_%3").arg(hdr, QString::number(i+1), QString::number(beg)));
            }
            else if (fnx == 2) // replace
            {
                varParamTable->setRowHeader(row, QString("%1_BLK%2repl_%3").arg(hdr, QString::number(i+1), QString::number(beg)));
            }
            else if (fnx == 3) // delta
            {
                varParamTable->setRowHeader(row, QString("%1_BLK%2delta_%3").arg(hdr, QString::number(i+1), QString::number(beg)));
            }
        }
    }
    else if (value == -1) // trend - offset
    {
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendFinal_LogstOffset").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendInfl_LogstOffset").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendWidth_yrs").arg(hdr));
    }
    else if (value == -2) // trend - direct
    {
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendFinal_direct").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendInfl_yr").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendWidth_yr").arg(hdr));
    }
    else if (value == -3) // trend - fraction
    {
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendFinal_frac").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendInfl_frac").arg(hdr));
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row++, QString("%1_TrendWidth_yr").arg(hdr));
    }
    else if (value <= -4) // trend - seasonal
    {
        int seas = data->get_num_seasons();
        for (int i = 0; i < seas; i++, row++)
        {
            varParam = readShortParameter(infile);
            varParamTable->setRowData(row, varParam);
            varParamTable->setRowHeader(row, QString("%1_TrendFinal_seas%2").arg(hdr, QString::number(i+1)));
        }
    }
    return (row);
}

int readTimeVaryDevParams(ss_file *infile, ss_model *data, int value, QString hdr, int row, tablemodel *varParamTable)
{
    QStringList varParam;
    QString varHeader(hdr);

    switch (value)
    {
    case 1:
    case 2:
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row, QString("%1_dev_se").arg(varHeader));
        row++;
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        varParamTable->setRowHeader(row, QString("%1_dev_autocorr").arg(varHeader));
        break;
    }
    return (row + 1);
}

int readTimeVaryEnvParams(ss_file *infile, ss_model *data, int value, QString hdr, int row, tablemodel *varParamTable)
{
    int fnx = value / 100;
    int var = value - (fnx * 100);
    QStringList varParam;
    QString varHeader(hdr);

    if (var > 0)
    {
        varParam = readShortParameter(infile);
        varParamTable->setRowData(row, varParam);
        switch (fnx)
        {
        default:
        case 0:
            varHeader.append(QString("_ENV_none"));
            break;
        case 1:
            varHeader.append(QString("_ENV_add"));
            break;
        case 2:
            varHeader.append(QString("_ENV_mult"));
            break;
        case 3:
            varHeader.append(QString("_ENV_offset"));
            break;
        case 4:
            varHeader.append(QString("_ENV_lgst_slope"));
            break;
        }
        varParamTable->setRowHeader(row, varHeader);
    }
    return (row + 1);
}

int writeTimeVaryParams(ss_file *outfile, ss_model *data, tablemodel *table, QStringList parmlist, QString header)
{
    QStringList p_list;
    QString rheader;
    int index = 0;
//    int last_index = table->rowCount();
    int chars = 0;
    int blk = parmlist.at(12).toInt();

    // write parameters for block or trend
    if (blk > 0)
    {
        // write block parameters
        int n_blks = data->getBlockPattern(blk-1)->getNumBlocks();
        for (int i = 0; i < n_blks; i++)
        {
            p_list = table->getRowData(index);
            rheader = table->getRowHeader(index++);
            chars += outfile->write_vector(p_list, 2, rheader);
        }
    }
    //  or write trend parameters
    else if (blk < 0)
    {
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
    }

    // write parameter for env
    if (parmlist.at(7).toInt() != 0)
    {
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
    }

    // write parameters for dev
    if (parmlist.at(8).toInt() != 0)
    {
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
        p_list = table->getRowData(index);
        rheader = table->getRowHeader(index++);
        chars += outfile->write_vector(p_list, 2, rheader);
    }
    return chars;
}
