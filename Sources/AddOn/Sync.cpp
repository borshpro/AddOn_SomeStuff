﻿//------------ kuvbur 2022 ------------
#include	<stdlib.h> /* atoi */
#include	<time.h>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Sync.hpp"
#include	"ResetProperty.hpp"
#include	"Dimensions.hpp"

#define SYNC_NO 0
#define SYNC_FROM 1
#define SYNC_TO 2
#define SYNC_TO_SUB 3
#define SYNC_FROM_SUB 4

Int32 nLib = 0;

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings) {
	if (ResetAllProperty()) syncSettings.syncAll = false;
	GS::UniString	title("Sync All");
	bool flag_chanel = false;
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		short nPhase = 1;
		ACAPI_Interface(APIIo_InitProcessWindowID, &title, &nPhase);
		long time_start = clock();
		if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ObjectID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_WindowID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.widoS) flag_chanel = SyncByType(API_DoorID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.objS) flag_chanel = SyncByType(API_ZoneID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_WallID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_SlabID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_ColumnID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_BeamID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_RoofID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MeshID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.wallS) flag_chanel = SyncByType(API_MorphID, syncSettings, nPhase);
		nPhase = nPhase + 1;
		if (!flag_chanel && syncSettings.cwallS) flag_chanel = SyncByType(API_CurtainWallID, syncSettings, nPhase);
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		msg_rep("SyncAll", time, NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
		return NoError;
							  });
}

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings, short& nPhase) {
	GS::UniString		subtitle;
	GSErrCode			err = NoError;
	GS::Array<API_Guid>	guidArray;
	bool flag_chanel = false;
	long time_start = clock();
	ACAPI_Element_GetElemList(elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight);
	if (!guidArray.IsEmpty()) {
#ifdef AC_26
		API_ElemType elemType;
		elemType.typeID = elementType;
		ACAPI_Goodies_GetElemTypeName(elemType, subtitle);
#else
		ACAPI_Goodies(APIAny_GetElemTypeNameID, (void*)elementType, &subtitle);
#endif // AC_26
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			SyncElement(guidArray[i], syncSettings);
			if (i % 10 > 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) {
				flag_chanel = true;
				msg_rep("SyncByType", subtitle + u8" - отмена", NoError, APINULLGuid);
				return flag_chanel;
			}
		}
		GS::UniString intString = GS::UniString::Printf(" %d qty", guidArray.GetSize());
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		msg_rep("SyncByType", subtitle + intString + time, NoError, APINULLGuid);
	}
	return flag_chanel;
}

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
void SyncElement(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	API_ElemTypeID elementType;
	GSErrCode err = GetTypeByGUID(elemGuid, elementType);
	if (err != NoError) { return; }

	// Получаем список связанных элементов
	GS::Array<API_Guid> subelemGuids;
	GetRelationsElement(elemGuid, elementType, syncSettings, subelemGuids);
	SyncData(elemGuid, syncSettings, subelemGuids);
	if (!subelemGuids.IsEmpty() && SyncRelationsElement(elementType, syncSettings)) {
		for (UInt32 i = 0; i < subelemGuids.GetSize(); ++i) {
			API_Guid subelemGuid = subelemGuids[i];
			if (subelemGuid != elemGuid) {
				GS::Array<API_Guid> epm;
				SyncData(subelemGuid, syncSettings, epm);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings) {
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoSyncId, ACAPI_GetOwnResModule());
	GS::UniString fmane = "Sync";
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		CallOnSelectedElemSettings(SyncElement, false, true, syncSettings, fmane);
		return NoError;
							  });
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected(const SyncSettings& syncSettings) {
	GS::UniString fmane = "Run parameter script";
	CallOnSelectedElemSettings(RunParam, false, true, syncSettings, fmane);
}

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	API_Elem_Head	tElemHead;
	BNZeroMemory(&tElemHead, sizeof(API_Elem_Head));
	tElemHead.guid = elemGuid;
	GSErrCode	err = ACAPI_Element_GetHeader(&tElemHead);
	if (err != NoError) return;
	err = ACAPI_Goodies(APIAny_RunGDLParScriptID, &tElemHead, 0);
	if (err != NoError) {
		msg_rep("RunParam", "APIAny_RunGDLParScriptID", err, elemGuid);
		return;
	}
}

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement(const API_ElemTypeID& elementType, const SyncSettings& syncSettings) {
	bool flag_sync = false;
	switch (elementType) {
		case API_WindowID:
		case API_DoorID:
			if (syncSettings.widoS) flag_sync = true;
			break;
		case API_CurtainWallSegmentID:
		case API_CurtainWallFrameID:
		case API_CurtainWallJunctionID:
		case API_CurtainWallAccessoryID:
		case API_CurtainWallPanelID:
		case API_CurtainWallID:
			if (syncSettings.cwallS) flag_sync = true;
			break;
		default:
			if (syncSettings.wallS) flag_sync = true;
			break;
	}
	return flag_sync;
}

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids) {
	GSErrCode	err = NoError;
	if (!IsElementEditable(elemGuid, syncSettings, true)) return;

	// Если включён мониторинг - привязываем элемент к отслеживанию
	if (syncSettings.syncMon) {
		err = AttachObserver(elemGuid, syncSettings);
		if (err == APIERR_LINKEXIST)
			err = NoError;
		if (err != NoError) {
			msg_rep("SyncData", "AttachObserver", err, elemGuid);
			return;
		}
	}
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) {
		msg_rep("SyncData", "ACAPI_Element_GetPropertyDefinitions", err, elemGuid);
		return;
	}

	// Синхронизация данных
	if (GetElemState(elemGuid, definitions, "Sync_flag")) { // Проверяем - не отключена ли синхронизация у данного объекта
		API_ElemTypeID elementType;
		err = GetTypeByGUID(elemGuid, elementType);
		if (err != NoError) { return; }
		GS::Array <WriteData> mainsyncRules;
		bool hasSub = false;
		for (UInt32 i = 0; i < definitions.GetSize(); i++) {

			// Получаем список правил синхронизации из всех свойств
			ParseSyncString(elemGuid, elementType, definitions[i], mainsyncRules, hasSub); // Парсим описание свойства
		}
		if (mainsyncRules.IsEmpty()) return;

		// Заполняем правила синхронизации с учётом субэлементов, попутно заполняем словарь параметров для чтения/записи
		WriteDict syncRules; // Словарь с правилами для каждого элемента
		ParamDictElement paramToRead; // Словарь с параметрами для чтения
		SyncAddSubelement(subelemGuids, mainsyncRules, syncRules, paramToRead);
		mainsyncRules.Clear();
		subelemGuids.Push(elemGuid); // Это теперь список всех элементов для синхронизации

		// Читаем все возможные свойства
		ParamHelpers::ElementRead(paramToRead);
		ParamDictElement paramToWrite; // Словарь с параметрами для записи

		// Выбираем по-элементно параметры для чтения и записи, формируем словарь
		for (UInt32 i = 0; i < subelemGuids.GetSize(); i++) {
			API_Guid elemGuid = subelemGuids[i];
			GS::Array <WriteData> writeSubs;
			if (syncRules.ContainsKey(elemGuid)) writeSubs = syncRules.Get(elemGuid);
			if (!writeSubs.IsEmpty()) {

				// Заполняем значения параметров чтения/записи из словаря
				for (UInt32 j = 0; j < writeSubs.GetSize(); j++) {
					WriteData writeSub = writeSubs.Get(j);
					API_Guid elemGuidTo = writeSub.guidTo;
					API_Guid elemGuidFrom = writeSub.guidFrom;

					// Проверяем - есть ли вообще эти элементы в словаре параметров
					if (paramToRead.ContainsKey(elemGuidTo) && paramToRead.ContainsKey(elemGuidFrom)) {
						GS::UniString rawNameFrom = writeSub.paramFrom.rawName;
						GS::UniString rawNameTo = writeSub.paramTo.rawName;
						ParamDictValue paramsTo = paramToRead.Get(elemGuidTo);
						ParamDictValue paramsFrom;
						if (elemGuidFrom == elemGuidTo) {
							paramsFrom = paramsTo;
						} else {
							paramsFrom = paramToRead.Get(elemGuidFrom);
						}

						// Проверяем наличие имён в словаре параметров
						if (paramsTo.ContainsKey(rawNameTo) && paramsFrom.ContainsKey(rawNameFrom)) {
							ParamValue paramFrom = paramsFrom.Get(rawNameFrom);
							ParamValue paramTo = paramsTo.Get(rawNameTo);

							//Сопоставляем и записываем, если значения отличаются
							//TODO Добавить обработку writeOne.stringformat
							if (paramFrom.isValid && paramFrom != paramTo) {
								paramTo.val = paramFrom.val; // Записываем только значения
								paramTo.isValid = true;
								SyncAddParam(paramTo, paramToWrite);
							}
						}
					}
				}
			}
		}
		if (!paramToWrite.IsEmpty()) {
			ParamHelpers::ElementWrite(paramToWrite);
		}
	}
}

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead) {
	for (UInt32 i = 0; i < mainsyncRules.GetSize(); i++) {
		if (!mainsyncRules[i].fromSub && !mainsyncRules[i].toSub) {
			WriteData writeSub = mainsyncRules.Get(i);
			SyncAddRule(writeSub, syncRules, paramToRead);
		}
		if (mainsyncRules[i].fromSub) {
			for (UInt32 j = 0; j < subelemGuids.GetSize(); j++) {
				WriteData writeSub = mainsyncRules.Get(i);
				API_Guid subelemGuid = subelemGuids.Get(j);
				writeSub.fromSub = false;
				writeSub.guidFrom = subelemGuid;
				writeSub.paramFrom.fromGuid = subelemGuid;
				SyncAddRule(writeSub, syncRules, paramToRead);
			}
		}
		if (mainsyncRules[i].toSub) {
			for (UInt32 j = 0; j < subelemGuids.GetSize(); j++) {
				WriteData writeSub = mainsyncRules.Get(i);
				API_Guid subelemGuid = subelemGuids.Get(j);
				writeSub.toSub = false;
				writeSub.guidTo = subelemGuid;
				writeSub.paramTo.fromGuid = subelemGuid;
				SyncAddRule(writeSub, syncRules, paramToRead);
			}
		}
	}
}

// --------------------------------------------------------------------
// Запись правила в словарь, попутно заполняем словарь с параметрами
// --------------------------------------------------------------------
void SyncAddRule(const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead) {
	API_Guid elemGuid = writeSub.guidTo;
	if (syncRules.ContainsKey(elemGuid)) {
		syncRules.Get(elemGuid).Push(writeSub);
	} else {
		GS::Array <WriteData> rules;
		rules.Push(writeSub);
		syncRules.Add(elemGuid, rules);
	}
	SyncAddParam(writeSub.paramFrom, paramToRead);
	SyncAddParam(writeSub.paramTo, paramToRead);
}

// --------------------------------------------------------------------
// Запись параметра в словарь
// --------------------------------------------------------------------
void SyncAddParam(const ParamValue& param, ParamDictElement& paramToRead) {
	API_Guid elemGuid = param.fromGuid;
	GS::UniString rawName = param.rawName;
	if (paramToRead.ContainsKey(elemGuid)) {
		if (!paramToRead.Get(elemGuid).ContainsKey(rawName)) {
			paramToRead.Get(elemGuid).Add(rawName, param);
		}
	} else {
		ParamDictValue params;
		params.Add(rawName, param);
		paramToRead.Add(elemGuid, params);
	}
}

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, bool& hasSub) {
	GS::UniString description_string = definition.description;
	if (description_string.IsEmpty()) {
		return false;
	}
	if (description_string.Contains("Sync_flag")) {
		return false;
	}

	// Если указан сброс данных - синхронизировать не будем
	if (description_string.Contains("Sync_reset")) {
		return false;
	}
	bool hasRule = false;
	if (description_string.Contains("Sync_") && description_string.Contains("{") && description_string.Contains("}")) {
		GS::Array<GS::UniString> rulestring;
		UInt32 nrule = StringSplt(description_string, "Sync_", rulestring, "{"); // Проверяем количество правил

		//Проходим по каждому правилу и извлекаем из него правило синхронизации (WriteDict syncRules) и словарь уникальных параметров для чтения/записи (ParamDictElement paramToRead)
		for (UInt32 i = 0; i < nrule; i++) {
			ParamValue param;
			int syncdirection = SYNC_NO; // Направление синхронизации
			GS::UniString rawparamName = ""; //Имя параметра/свойства с указанием типа синхронизации, для ключа словаря
			GS::Array<GS::UniString> ignorevals; //Игнорируемые значения
			GS::UniString stringformat = ""; //Строка с форматом числа
			if (SyncString(elementType, rulestring[i], syncdirection, param, ignorevals, stringformat)) {
				hasRule = true;
				ParamValue paramdef; //Свойство, из которого получено правило
				ParamHelpers::ConvValue(paramdef, definition);
				paramdef.fromGuid = elemGuid;
				WriteData writeOne;
				writeOne.stringformat = stringformat;
				writeOne.ignorevals = ignorevals;
				if (syncdirection == SYNC_TO || syncdirection == SYNC_TO_SUB) {
					if (syncdirection == SYNC_TO_SUB) {
						hasSub = true;
						writeOne.toSub = true;
					} else {
						writeOne.guidTo = elemGuid;
						param.fromGuid = elemGuid;
					}
					writeOne.guidFrom = elemGuid;
					writeOne.paramFrom = paramdef;
					writeOne.paramTo = param;
				}
				if (syncdirection == SYNC_FROM || syncdirection == SYNC_FROM_SUB) {
					if (syncdirection == SYNC_FROM_SUB) {
						hasSub = true;
						writeOne.fromSub = true;
					} else {
						writeOne.guidFrom = elemGuid;
						param.fromGuid = elemGuid;
					}
					writeOne.guidTo = elemGuid;
					writeOne.paramTo = paramdef;
					writeOne.paramFrom = param;
				}
				syncRules.Push(writeOne);
			}
		}
	}
	return hasRule;
}

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString(const  API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, GS::UniString& stringformat) {
	syncdirection = SYNC_NO;

	// Выбор направления синхронизации
	// Копировать в субэлементы или из субэлементов
	if (syncdirection == SYNC_NO && rulestring_one.Contains("to_sub{")) syncdirection = SYNC_TO_SUB;
	if (syncdirection == SYNC_NO && rulestring_one.Contains("from_sub{")) syncdirection = SYNC_FROM_SUB;

	// Копировать параметр в свойство или свойство в параметр
	if (syncdirection == SYNC_NO && rulestring_one.Contains("from{")) syncdirection = SYNC_FROM;
	if (syncdirection == SYNC_NO && rulestring_one.Contains("to{")) syncdirection = SYNC_TO;

	//Если направление синхронизации не нашли - выходим
	if (syncdirection == SYNC_NO) return false;

	GS::UniString paramNamePrefix = "";

	//Выбор типа копируемого свойства
	bool synctypefind = false;

	//GS::Array<GS::UniString> paramTypesList;
	//GetParamTypeList(paramTypesList);
	//Я не очень понял - умеет ли с++ в ленивые вычисления, поэтому сделаю вложенные условия, чтобы избежать ненужного поиска по строке
	//TODO переписать всё это с использованием GetParamTypeList
	if (synctypefind == false) {
		if (!rulestring_one.Contains(":") || rulestring_one.Contains("escription:") || rulestring_one.Contains("esc:")) {
			if (rulestring_one.Contains("escription:") || rulestring_one.Contains("esc:")) {
				param.fromGDLdescription = true;
				rulestring_one.ReplaceAll("description:", "");
				rulestring_one.ReplaceAll("Description:", "");
				rulestring_one.ReplaceAll("desc:", "");
				rulestring_one.ReplaceAll("Desc:", "");
			}
			paramNamePrefix = "{GDL:";
			param.fromGDLparam = true;
			synctypefind = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Property:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Property:", "");
			paramNamePrefix = "{Property:";
			param.fromProperty = true;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Material:") && rulestring_one.Contains('"')) {

			//TODO Проверить на файле из видео обработку материалов
			synctypefind = true;
			rulestring_one.ReplaceAll("Material:", "");
			paramNamePrefix = "{Material:";
			GS::UniString templatestring = rulestring_one.GetSubstring('"', '"', 0);
			param.val.uniStringValue = templatestring;
			rulestring_one.ReplaceAll(templatestring, "");
			param.fromMaterial = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Morph:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Morph:", "");
			paramNamePrefix = "{Morph:";
			param.fromMorph = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Coord:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Coord:", "");
			paramNamePrefix = "{Coord:";
			param.fromCoord = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("Info:") && rulestring_one.Contains('"')) {
			synctypefind = true;
			rulestring_one.ReplaceAll("Info:", "");
			paramNamePrefix = "{Info:";
			param.fromInfo = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) {
		if (rulestring_one.Contains("IFC:")) {
			synctypefind = true;
			rulestring_one.ReplaceAll("IFC:", "");
			paramNamePrefix = "{IFC:";
			param.fromIFCProperty = true;
			syncdirection = SYNC_FROM;
		}
	}
	if (synctypefind == false) return false;

	//Проверка допустимости правила для типа элемента
	if (param.fromGDLparam) {
		if (elementType == API_WallID ||
			elementType == API_SlabID ||
			elementType == API_ColumnID ||
			elementType == API_BeamID ||
			elementType == API_RoofID ||
			elementType == API_ShellID ||
			elementType == API_MorphID) synctypefind = false;
	}
	if (param.fromGDLdescription) {
		if (elementType != API_ObjectID) synctypefind = false;
	}
	if (param.fromMaterial) {
		if (elementType != API_WallID &&
			elementType != API_SlabID &&
			elementType != API_ColumnID &&
			elementType != API_BeamID &&
			elementType != API_RoofID &&
			elementType != API_ShellID) synctypefind = false;
	}
	if (param.fromMorph) {
		if (elementType != API_MorphID) synctypefind = false;
	}

	//Если тип свойства не нашли - выходим
	if (synctypefind == false) return false;

	GS::UniString tparamName = rulestring_one.GetSubstring('{', '}', 0);
	GS::Array<GS::UniString> params;
	UInt32 nparam = StringSplt(tparamName, ";", params);

	// Параметры не найдены - выходим
	if (nparam == 0) return false;
	GS::UniString paramName = params[0];
	if (paramName.Contains("#")) {
		GS::Array<GS::UniString> tparams;
		UInt32 tnparam = StringSplt(paramName, "#", tparams);
		if (tnparam == 2) {
			paramName = tparams[0];
			stringformat = tparams[1];
		}
	}
	paramName.ReplaceAll("\\/", "/");
	param.rawName = paramNamePrefix + paramName.ToLowerCase() + "}";
	param.name = paramName;
	if (nparam > 1) {
		for (UInt32 j = 1; j < nparam; j++) {
			GS::UniString ignoreval;
			if (params[j].Contains('"')) {
				ignoreval = params[j].GetSubstring('"', '"', 0);
			} else {
				ignoreval = params[j];
			}
			ignorevals.Push(ignoreval);
		}
	}
	return true;
}