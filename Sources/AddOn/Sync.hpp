//------------ kuvbur 2022 ------------
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"SyncSettings.hpp"
#include	"Helpers.hpp"
// --------------------------------------------------------------------
// ��������� ��� �������� ������ �������
// ���������� ��. SyncString
// --------------------------------------------------------------------
typedef struct {
	GS::UniString paramNameFrom = "";
	API_PropertyDefinition paramFrom = {};
	GS::UniString paramNameTo = "";
	API_PropertyDefinition paramTo = {};
	GS::Array<GS::UniString> ignorevals;
	GS::UniString templatestring = "";
	int synctype = 0;
	int syncdirection = 0;
} SyncRule;

typedef struct {
	API_Guid guidTo = APINULLGuid;
	API_Guid guidFrom = APINULLGuid;
	ParamValue paramFrom;
	ParamValue paramTo;
	GS::Array<GS::UniString> ignorevals;
	GS::UniString stringformat = ""; //������ ������ (������� � ������� #mm ��� #0)
	bool toSub = false;
	bool fromSub = false;
} WriteData;

// ������� � ����������� ��� ������
typedef GS::HashTable<API_Guid, GS::Array <WriteData>> WriteDict;

// --------------------------------------------------------------------
// ��������� ��� �������� ������ �� ������� �����������
// ���������� ��. SyncPropAndMatGetComponents
// --------------------------------------------------------------------
typedef struct {
	API_Attribute					buildingMaterial;
	GS::Array<API_PropertyDefinition>	definitions;
	GS::UniString						templatestring = "";
	double								fillThick = 0.0;
} LayerConstr;


// -----------------------------------------------------------------------------
// ��������� ��������� ���� ��������, �������� � ���������
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// ��������� ��������� ���� ��������� ��������� ����
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings, short& nPhase);

void SyncElement(const API_Guid& elemGuid, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// ��������� ��������� ���������, �������� � ���������
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings);

void RunParamSelected(const SyncSettings& syncSettings);

void RunParam(const API_Guid& elemGuid, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// ����� � ������������� ������� ��������� ���������
// --------------------------------------------------------------------
bool SyncRelationsElement(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// ������������� ������ �������� �������� ��������� � �������� �������
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids);

void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// ������ ������� � �������
// --------------------------------------------------------------------
void SyncAddRule(const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// ������ ��������� � �������
// --------------------------------------------------------------------
void SyncAddParam(const ParamValue& param, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// ������ �������� ��������
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, bool& hasSub);

// -----------------------------------------------------------------------------
// ������ �������� ��������
// -----------------------------------------------------------------------------
bool SyncString(const API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, GS::UniString& stringformat);

// -----------------------------------------------------------------------------
// ���� � ������ - ������� �������� � ���������� ������ �����������
// ������ ������� �� �����
//			%��� ��������% ����� %��� ������/��� ��������.5mm%
// ������ ������� �� ������
//			@1@ ����� @2@#.5mm#
// ���� �������� �� �������, %��� ��������% �������� �� ������� ("")
// -----------------------------------------------------------------------------
GSErrCode  SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

GSErrCode  SyncPropAndMatOneGetComponent(const API_AttributeIndex& constrinx, const double& fillThick, LayerConstr& component);

// --------------------------------------------------------------------
// ����������� ��, ��� �����, �� ���������� � ������� ��������
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components);

void SyncPropAndMatReplaceValue(const double& var, const GS::UniString& patternstring, GS::UniString& outstring);

void SyncPropAndMatReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring);

// --------------------------------------------------------------------
// �������� � ������ templatestring ��� ��������� @1@...@n@ �� �������� �������
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatWriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n);

// -----------------------------------------------------------------------------
// ������ � �������� ������ � ���������
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// ��������� - �������� �� ������ ������������ ��������
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);

// -----------------------------------------------------------------------------
// ��������� - �������� �� �������� ������������ ���������
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);

// -----------------------------------------------------------------------------
// ��������� - �������� �� �������� ������������ ���������
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_IFCProperty& property);


#endif
