#ifndef SS_RECRUITMENT_H
#define SS_RECRUITMENT_H

#include "short_parameter.h"
#include "long_parameter.h"
#include "input_file.h"
#include "parametermodelTV.h"
#include "fileIOgeneral.h"

#include "ss_observation.h"

#include <QList>
#include <QString>
#include <map>

class spawn_recruit
{
public:
    spawn_recruit(ss_model *parent = 0);
    ~spawn_recruit();

    int   method; // 2=Ricker; 3=std_B-H; 4=SCAA; 5=Hockey; 6=B-H_flattop; 7=survival_3Parm; 8=Shepard_3Parm
    int   use_steepness; // 0/1 to use steepness in initial equ recruitment calculation
    float env_link;     //
    int   env_target;   // 0=none;1=devs;_2=R0;_3=steepness
    int   rec_dev;      // recruitment deviations: 0=none; 1=devvector; 2=simple deviations
    int   rec_dev_start_yr; // first year of main recr_devs
    int   rec_dev_end_yr;   // last year of main recr_devs
    int   rec_dev_phase;    // rec_dev phase
    bool  advanced_opts; // use 13 advanced options
    int   rec_dev_early_start; // 0=none, neg value makes relative to rec_dev_start_yr
    int   rec_dev_early_phase; //
    int   fcast_rec_phase;  // forecast recruitment phase, 0 resets to maxphase + 1
    float fcast_lambda;  //
    int   nobias_last_early_yr; //
    int   fullbias_first_yr; //
    int   fullbias_last_yr; //
    int   nobias_first_recent_yr; //
    float max_bias_adjust;  // max bias adjustment in MPD
    int   rec_cycles;   // period of cycles in recruitment
    int   rec_dev_min;  // min num of rec devs
    int   rec_dev_max;  // max
    int   num_rec_dev;  // recruitment deviations specified by year

    int distrib_method;
    int distrib_area;
    tablemodel *assignments;
    bool doRecruitInteract;
    QStringList header;

    void reset ();

    void setMethod (int value) {method = value;}
    int getMethod () {return method;}

    void setUseSteepness (int value) {use_steepness = value;}
    int getUseSteepness () {return use_steepness;}

//    shortParameter parameters[6];
//    parametermodel *parameters;
//    parametermodel *getSetupModel() {return parameters;}
//    void setNumSetupLines(int lines) {parameters->setRowCount(lines);}
//    int getNumSetupLlines() {return parameters->rowCount();}
//    void setParameter (int index, QStringList values) {parameters->setRowData(index, values);}
//    QStringList getParameter(int index) {return parameters->getRowData(index);}

//    void setParameter(int index, QString line);
    QString toText();
//    QString toXML();
  //  QString toJSON();
    void fromFile(ss_file *file);
//    void fromXML (xml_file *file);
  //  void fromJSON (json_file *file);

    parameterModelTV *full_parameters;
    parameterModel *getFullParameterModel() {return full_parameters->getParameters();}
    void setNumFullParameters(int num) {full_parameters->setRowCount(num);}
    int getNumFullParameters() {return full_parameters->rowCount();}
    void setFullParameter(int index, QStringList values) {full_parameters->setRowData(index, values);}
    void insertFullParameter (int index) {full_parameters->getParameters()->insertRow(index, 0);}
    void removeFullParameter (int index) {full_parameters->getParameters()->removeRow(index);}
    void setFullParameterHeader(int index, QString hdr) {full_parameters->setRowHeader(index, hdr);}
    QStringList getFullParameter(int index) {return full_parameters->getRowData(index);}

//    QList<longParameter *> full_parameters;
    std::map<int, float> yearly_devs;

    QString sr_text;

    int getDistribMethod() const {return distrib_method;}
    void setDistribMethod (int value) {distrib_method = value;}
    int getDistribArea () const {return distrib_area;}
    void setDistribArea (int value) {distrib_area = value;}
    bool getDoRecruitInteract() const;
    void setDoRecruitInteract(bool value);
    void setDoRecruitInteract (int value);
    int getNumAssignments();
    void setNumAssignments(int rows);
    tablemodel *getAssignments() const;
    void setAssignment(int row, QStringList data);
    QStringList getAssignment(int row);
    parameterModelTV *assignmentParams;
    tablemodel *getAssignmentParams () {return assignmentParams->getParameters();}
    void setNumAssignmentParams (int num) {assignmentParams->setRowCount(num);}
    int getNumAssignmentParams () {return assignmentParams->rowCount();}
    void addAssignmentParam (QStringList data) {setInteractParam(getNumInteractParams(), data);}
    void setAssignmentParam (int index, QStringList data);
    QStringList getAssignmentParam(int index) {return assignmentParams->getRowData(index);}


    recruitDevs *recruitDeviations;
    recruitDevs *getRecruitDevs () {return recruitDeviations;}

    parameterModelTV *interactParams;
    tablemodel *getInteractParams () {return interactParams->getParameters();}
    void setNumInteractParams (int num) {interactParams->setRowCount(num);}
    int getNumInteractParams () {return interactParams->rowCount();}
    void addInteractParam (QStringList data) {setInteractParam(getNumInteractParams(), data);}
    void setInteractParam (int index, QStringList data);
    QStringList getInteractParam(int index) {return interactParams->getRowData(index);}


    parameterModelTV *recruitDistParams;
    tablemodel *getRecruitDistParams() {return recruitDistParams->getParameters();}
    void setNumRecruitDistParams (int num) {recruitDistParams->setRowCount(num);}
    int getNumRecruitDistParams () {return recruitDistParams->rowCount();}
    void addRecruitDistParam (QStringList data) {setRecruitDistParam(getNumRecruitDistParams(), data);}
    void setRecruitDistParam (int index, QStringList data);
    QStringList getRecruitDistParam(int index) {return recruitDistParams->getRowData(index);}

    int readtvparams;
    void setTimeVaryReadParams(int flag) {readtvparams = flag;}
    int getTimeVaryReadParams() {return readtvparams;}
    shortParameterModel *tvParameters;
    shortParameterModel *getTVParameterModel() {return tvParameters;}
    void setNumTVParameters(int num) {tvParameters->setRowCount(num);}
    int getNumTVParameters() {return tvParameters->rowCount();}
    void addTVParameter(QStringList values) {setTVParameter(tvParameters->rowCount(), values);}
    void setTVParameter(int index, QStringList values);
    void setTVParameterHeader(int index, QString hdr) {tvParameters->setRowHeader(index, hdr);}
    QStringList getTVParameter(int index) {return tvParameters->getRowData(index);}

private:
    ss_model *parnt;
};

#endif // SS_RECRUITMENT_H
