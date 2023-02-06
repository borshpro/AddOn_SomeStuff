//------------ kuvbur 2022 ------------
#pragma once

#ifndef HELPERS_HPP
#define	HELPERS_HPP

#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"StringConversion.hpp"
#include	"ResourceIds.hpp"
#include	"SyncSettings.hpp"
#include	"exprtk.hpp"

#define ELEMSTR_LEN				256
#define	CURR_ADDON_VERS			0x0006
#define	 Menu_MonAll		1
#define	 Menu_SyncAll		2
#define	 Menu_SyncSelect	3
#define	 Menu_wallS			4
#define	 Menu_widoS			5
#define	 Menu_objS			6
#define	 Menu_cwallS		7
#define	 Menu_ReNum			8
#define	 Menu_Sum			9
#define	 Menu_Log			10
#define	 Menu_LogShow		11

#define SYNC_GDL 1
#define SYNC_PROPERTY 2
#define SYNC_MATERIAL 3
#define SYNC_INFO 4
#define SYNC_IFC 5
#define SYNC_MORPH 6
#define SYNC_CLASS 7

static const GSResID AddOnInfoID = ID_ADDON_INFO;
static const short AddOnMenuID = ID_ADDON_MENU;
static const short AddOnPromtID = ID_ADDON_PROMT;

static const GSResID AddOnStringsID = ID_ADDON_STRINGS;

static const Int32 AddOnNameID = 1;
static const Int32 AddOnDescriptionID = 2;

static const Int32 MonAll_CommandID = 1;
static const Int32 SyncAll_CommandID = 2;
static const Int32 SyncSelect_CommandID = 3;
static const Int32 wallS_CommandID = 4;
static const Int32 widoS_CommandID = 5;
static const Int32 objS_CommandID = 6;
static const Int32 cwallS_CommandID = 7;
static const Int32 ReNum_CommandID = 8;
static const Int32 Sum_CommandID = 9;
static const Int32 Log_CommandID = 10;
static const Int32 LogShow_CommandID = 11;

static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 6;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;
static const Int32 ErrorSelectID = 6;

static const Int32 UndoDimRound = 7;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

typedef struct {
	GS::Array <API_Guid>	guid;
} SortGUID;

typedef struct {
	GS::Array <UInt32>	inx;
} SortInx;

// �������� ������ ���������
// type - API_VariantType (��� � �������)
// name - ��� ��� ������
// uniStringValue, intValue, boolValue, doubleValue - ��������
// canCalculate - ����� �� ������������ � �������������� �����������
typedef struct {
	API_VariantType type = API_PropertyUndefinedValueType;
	GS::UniString rawName = ""; // ��� ��� ������������� � ������� - � ��������� ������ �����
	GS::UniString name = ""; //��������� ��� ��� ������
	// ���������� ��������
	GS::UniString uniStringValue = "";
	GS::Int32 intValue = 0;
	bool boolValue = false;
	double doubleValue = 0.0;
	// -----------------
	bool canCalculate = false; // ����� ���� ����������� � ��������
	bool isValid = false; // ���������� (��� ������ ��� ������)
	API_PropertyDefinition definition = {}; // �������� ��������, ��� ��������� ������/������
	API_Property property = {}; // ���� ��������, ��� ��������� ������/������
	// ��� ������ ������, ������� ����� �������� ��������
	bool fromGDLparam = false; // ������ � ��� ����������
	bool fromGDLdescription = false; // ������ �� ��������
	bool fromProperty = false; // ������ � ���������
	bool fromMorph = false; // ������ ��������� �����
	bool fromInfo = false; // ������ � ���� � �������
	bool fromIFCProperty = false;
	bool fromCoord = false; //����������
	bool fromPropertyDefinition = false; //����� ������������, ������ �� �����
	bool fromMaterial = false; // ����� ���� �� ������� �����������
	API_Guid fromGuid = APINULLGuid; //������ ���������
} ParamValue;

// ������� � ������� ������������ ������� � �������� ������ ��������
typedef GS::HashTable<GS::UniString, ParamValue> ParamDictValue;

// ������� � ����������� ��� ����������
// ������ ��� ������������ ����������� ������ ������� � ����������
typedef GS::HashTable<GS::UniString, bool> ParamDict;

// ������� � ����������� ��� ���������
typedef GS::HashTable<API_Guid, ParamDictValue> ParamDictElement;

bool is_equal(double x, double y);

// --------------------------------------------------------------------
// �������� �� �������� ��������� ������ ������������
// --------------------------------------------------------------------
bool CheckIgnoreVal(const std::string& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// �������� �� �������� ��������� ������ ������������
// --------------------------------------------------------------------
bool CheckIgnoreVal(const GS::UniString& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// �������� �� �������� ��������� ������ ������������
// --------------------------------------------------------------------
bool CheckIgnoreVal(const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val);

// --------------------------------------------------------------------
// ������� ������, �������� ����� double � �� Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM(const double& value);

// --------------------------------------------------------------------
// ��������� ����� n ����� �� ���������� ������ �����, �������� k
// --------------------------------------------------------------------
Int32 ceil_mod(Int32 n, Int32 k);


// -----------------------------------------------------------------------------
// �������� ������� � ��������� ID ������������ Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork(bool& isteamwork, short& userid);

// -----------------------------------------------------------------------------
// ���������� ������������ (��� ������ ������)
// -----------------------------------------------------------------------------
GSErrCode	AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// ��������� - �������� �� ��� �������� � ��� ��������� �������������
// --------------------------------------------------------------------
bool CheckElementType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// ��������� ����������� �������������� ������� (�� ��������� � ������, �������������, ��������������)
// -----------------------------------------------------------------------------
bool IsElementEditable(const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType);

// -----------------------------------------------------------------------------
// �����������, ������������, ������� - ������ ������� �������������
// ������������, ��� ����� ��� ���������� - ������ ��������� � ������.
// -----------------------------------------------------------------------------
bool ReserveElement(const API_Guid& objectId, GSErrCode& err);

// -----------------------------------------------------------------------------
// ���������� ���������� � ���� �������
// -----------------------------------------------------------------------------
void MenuSetState(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// ����� ��������� � �����
// -----------------------------------------------------------------------------
void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// �������� ������ Guid ��������� ���������
// -----------------------------------------------------------------------------
void MenuItemCheckAC(short itemInd, bool checked);

// -----------------------------------------------------------------------------
// �������� ������ Guid ��������� ���������
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/);

// -----------------------------------------------------------------------------
// ����� ������� ��� ��������� ���������
//	(������� ������ ��������� � ������� ��������� API_Guid SyncSettings
// -----------------------------------------------------------------------------
void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// ����� ������� ��� ��������� ���������
//	(������� ������ ��������� � ������� ��������� API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/);

// -----------------------------------------------------------------------------
// ��������� ���� ������� �� ��� API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType);

#ifdef AC_26
bool GetElementTypeString(API_ElemType elemType, char* elemStr);
#else
bool GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
#endif

// --------------------------------------------------------------------
// ����� ��������� ���������
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

// --------------------------------------------------------------------
// ����� ��������� ���������
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

// -----------------------------------------------------------------------------
// ������ �������� �������� � ��������� �������
// ���� ���������� ������ GLOB_ID
// -----------------------------------------------------------------------------
GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property);

// -----------------------------------------------------------------------------
// ����� ������ �� �����������, ���������� ���-�� ������
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);

// -----------------------------------------------------------------------------
// ����� ������ �� �����������, ���������� ���-�� ������
// ���������� � ������ ������ �����, ���������� ������ filter
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter);

// --------------------------------------------------------------------
// ��������� ������ GUID �������, ��� � ����������� �������� �����
// --------------------------------------------------------------------
GSErrCode GetCWElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids);

// --------------------------------------------------------------------
// ��������� ������ GUID ��������� ����������
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing(const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids);

// -----------------------------------------------------------------------------
// ��������� �������� IFC �������� �� ����� ��������
// -----------------------------------------------------------------------------
GSErrCode GetIFCPropertyByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_IFCProperty& property);

// -----------------------------------------------------------------------------
// ��������� �������� �����
// ��������� ������� ParamDictValue& pdictvalue �� ����������
// -----------------------------------------------------------------------------
GSErrCode GetMorphParam(const API_Element& element, ParamDictValue& pdictvalue);

// -----------------------------------------------------------------------------
// ���������� elemType � elemGuid ��� ����������� ������ ���������� ��������� �������� �����
// -----------------------------------------------------------------------------
void GetGDLParametersHead(const API_Element& element, const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// ���������� ������ ���������� API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_ElemTypeID& elemType, const API_Guid& elemGuid, API_AddParType**& params);

// -----------------------------------------------------------------------------
// ��������� ��������� �������
// Level 3
// symb_pos_x , symb_pos_y, symb_pos_z
// ��� ������� �������� ����� ���������� ����� ������
// ��� ������� ��� ������� - ����� ������� � ���. ����
// ��� ���� - ����� ���� (��� �������, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool FindLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);

// -----------------------------------------------------------------------------
// ��������� ���������� ����� � ������ ��������� � ����� ��������
// ������� �� ����� paramName ��������� ������� ���������
// ���������� ������ ��� ������������ ������� NumToStig
// -----------------------------------------------------------------------------
GS::UniString GetFormatString(GS::UniString& paramName);

// -----------------------------------------------------------------------------
// ��������� ���� ������ � �����
// $ - ������� ������
// # - ������� ����� �����
// �� ��������� - double
// ������� �� ����� paramName ��������� �������� �� ��� ������
// -----------------------------------------------------------------------------
API_VariantType GetTypeString(GS::UniString& paramName);

// -----------------------------------------------------------------------------
// ��������� �� ������ ��� ����� ������� ��� ����������, ����������� � ����� %
// -----------------------------------------------------------------------------
bool GetParamNameDict(const GS::UniString& expression, ParamDict& paramDict);

// -----------------------------------------------------------------------------
// ������ ���� ���������� �� �������� � ���������
// �������� ���������� �������, ���������� �������� ��. GetParamValueDict
// -----------------------------------------------------------------------------
bool ReplaceParamInExpression(const ParamDictValue& pdictvalue, GS::UniString& expression);

// -----------------------------------------------------------------------------
// ���������� ���������, ����������� � < >
// ��� �� ����� ��������� - ������� �� �������
// -----------------------------------------------------------------------------
bool EvalExpression(GS::UniString& unistring_expression);

// -----------------------------------------------------------------------------
// ������ ��������� ��������� ����� ����������
// -----------------------------------------------------------------------------
void GetParamTypeList(GS::Array<GS::UniString>& paramTypesList);

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark(short menuResID, short itemIndex);

namespace PropertyTestHelpers
{
	GS::UniString NumToString(const double& var, const GS::UniString stringformat);
	GS::UniString ToString(const API_Variant& variant, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Variant& variant);
	GS::UniString ToString(const API_Property& property, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Property& property);
}

bool operator== (const ParamValue& lhs, const ParamValue& rhs);

bool operator== (const API_Variant& lhs, const API_Variant& rhs);

bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs);

bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs);

bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs);

#if defined(AC_24)
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs);
#endif

bool Equals(const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType);

bool Equals(const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType);

bool operator== (const API_PropertyGroup& lhs, const API_PropertyGroup& rhs);

bool operator== (const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs);

bool operator== (const API_Property& lhs, const API_Property& rhs);

template <typename T>
bool operator!= (const T& lhs, const T& rhs)
{
	return !(lhs == rhs);
}

// -----------------------------------------------------------------------------
// �������� ������ ������ �� ��������
// -----------------------------------------------------------------------------
void DeleteElementUserData(const API_Guid& elemguid);

// -----------------------------------------------------------------------------
// �������� ������ ������ �� ���� ���������
// -----------------------------------------------------------------------------
void DeleteElementsUserData();

void UnhideUnlockAllLayer(void);

// -----------------------------------------------------------------------------
// ����������� �������� ParamValue � ��������, ����������� � ��
// ���������� true ���� �������� ����������
// -----------------------------------------------------------------------------
bool ParamValueToProperty(ParamValue& pvalue);


// -----------------------------------------------------------------------------
// ������������� ParamValue � API_Property
// ���������� true � �������������� ��� ������ �������� � ������ �������
// TODO ���������� �� ��� ������ ParamValue
// -----------------------------------------------------------------------------
bool ParamValueToProperty(const ParamValue& pvalue, API_Property& property);

//--------------------------------------------------------------------------------------------------------------------------
//���� �������� property_flag_name � �������� � �� �������� ���������� - ����� �� ������������ �������
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name);

// --------------------------------------------------------------------
// ������ ������� ���������� ��� ��������� ���������
// --------------------------------------------------------------------
void ParamDictElementWrite(ParamDictElement& paramToWrite);

// --------------------------------------------------------------------
// ������ ���������� � ���� �������
// --------------------------------------------------------------------
void ParamDictWrite(const API_Guid& elemGuid, ParamDictValue& params);

void ParamDictSetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// ���������� ������� ���������� ��� ��������� ���������
// --------------------------------------------------------------------
void ParamDictElementRead(ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// ���������� ������� � �����������
// --------------------------------------------------------------------
void ParamDictRead(const API_Guid& elemGuid, ParamDictValue& params);

// -----------------------------------------------------------------------------
// ��������� ����������� �������� �� ����� ��������
// ������ ����� ������/���_��������
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const GS::UniString& propertyname, API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// ��������� ����������� �������� �� ����� �������� ���� ��������� ��������
// ������ ����� ������/���_��������
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_PropertyDefinition& definition);

// --------------------------------------------------------------------
// �������� ��� ��������� �������� � �������� ParamDictValue
// --------------------------------------------------------------------
void GetAllPropertyDefinitionToParamDict(ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// �������� ����� ����������� �������
// --------------------------------------------------------------------
void ParamDictGetPropertyDefinition(ParamDictValue& params);

// --------------------------------------------------------------------
// ������������� ���� �������� ParamDictValue
// --------------------------------------------------------------------
void ParamDictCompare(const ParamDictValue& paramsFrom, ParamDictValue& paramsTo);

// --------------------------------------------------------------------
// ������ �������� ������� � ParamDictValue
// --------------------------------------------------------------------
bool ParamDictGetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

// -----------------------------------------------------------------------------
// �������� �������� GDL ��������� �� ��� ����� ��� �������� � ParamValue
// -----------------------------------------------------------------------------
bool ParamDictGetGDLValues(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

// -----------------------------------------------------------------------------
// ����� �� �������� GDL ���������
// ������ ������ �� �������� � ���������� �������� ����
// -----------------------------------------------------------------------------
bool FindGDLParamByDescription(const API_Element& element, ParamDictValue& params);

// -----------------------------------------------------------------------------
// ����� �� ����� GDL ���������
// -----------------------------------------------------------------------------
bool FindGDLParamByName(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

// -----------------------------------------------------------------------------
// �������� ������ ��� �������� (������� ��� ������)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name);

// -----------------------------------------------------------------------------
// ����������� ���������� ������������� �������� � ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_AddParType& nthParameter);

// -----------------------------------------------------------------------------
// ����������� �������� � ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_Property& property);

// -----------------------------------------------------------------------------
// ����������� ����������� �������� � ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// ����������� ������ ����� � ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const Int32 intValue);

// -----------------------------------------------------------------------------
// ����������� double � ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const double doubleValue);

#endif