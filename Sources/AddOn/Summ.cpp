﻿//------------ kuvbur 2022 ------------
#include	<map>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Summ.hpp"
typedef std::unordered_map <std::string, SortInx> SumCriteria;

// -----------------------------------------------------------------------------------------------------------------------
// Объединение уникальных значений свойств выделенных элементов
// Во многом похоже на модуль ReNum
// Флаг суммирования делать не буду - ибо смысла не вижу.
// Будем суммировать все выделенные элементы, у которых задано правило
// 1. Получаем список объектов, в свойствах которых ищем
//					Sum{*имя свойства-значения*; *имя свойства-критерия*}
// 2. Собираем правила суммирования
// 3. Для каждого правила отбираем элементы
// 4. Средни них ищем уникальные значения
// 5. Записываем их через разделитель
// TODO Попробовать сделать диапазоны 1...10 и т.д.
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode SumSelected(SyncSettings& syncSettings) {
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true, syncSettings, true);
	if (guidArray.IsEmpty()) return NoError;
	long time_start = clock();
	GS::UniString funcname = "Summation";
	GS::UniString subtitle = "read data...";
	short nPhase = 1; short i = 1;
	ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
	ParamDictElement paramToWriteelem;
	if (!GetSumValuesOfElements(guidArray, paramToWriteelem)) {
		msg_rep("SumSelected", "No data to write", NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
		return NoError;
	}
	subtitle = "write data..."; i = 2;
	ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		ParamHelpers::ElementsWrite(paramToWriteelem);
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d ", guidArray.GetSize()) + GS::UniString::Printf("wrtite to - %d", paramToWriteelem.GetSize()) + time;
		msg_rep("SumSelected", intString, NoError, APINULLGuid);
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
		return NoError;
							  });
	return NoError;
}

bool GetSumValuesOfElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem) {
	if (guidArray.IsEmpty()) return false;
	SumRules rules;
	ParamDictElement paramToReadelem;

	// Получаем список правил суммирования
	bool hasSum = false;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
		GS::Array<API_PropertyDefinition>	definitions;
		GSErrCode err = ACAPI_Element_GetPropertyDefinitions(guidArray[i], API_PropertyDefinitionFilter_UserDefined, definitions);
		if (err == NoError && !definitions.IsEmpty()) {
			ParamDictValue propertyParams;
			ParamHelpers::GetAllPropertyDefinitionToParamDict(propertyParams, definitions);
			if (!propertyParams.IsEmpty()) {
				if (Sum_GetElement(guidArray[i], propertyParams, paramToReadelem, rules)) hasSum = true;
			}
		}
	}
	if (!hasSum) return false;
	ParamDictValue propertyParams; // Все свойства уже считаны, поэтому словарь просто пустой
	ParamHelpers::ElementsRead(paramToReadelem, propertyParams); // Читаем значения

	// Получаем данные об округлении и типе расчёта
	API_CalcUnitPrefs unitPrefs1;
	ACAPI_Environment(APIEnv_GetPreferencesID, &unitPrefs1, (void*)APIPrefs_CalcUnitsID);

	API_WorkingUnitPrefs unitPrefs;
	ACAPI_Environment(APIEnv_GetPreferencesID, &unitPrefs, (void*)APIPrefs_WorkingUnitsID);
	if (unitPrefs1.useDisplayedValues) {
		unitPrefs.roundInch = 1;
	}
	else {
		unitPrefs.roundInch = 0;
	}

	// Суммируем, заполняе словарь для записи
	for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
		const SumRule& rule = *cIt->value;
		if (!rule.elemts.IsEmpty()) Sum_OneRule(rule, unitPrefs, paramToReadelem, paramToWriteelem);
	}
	return !paramToWriteelem.IsEmpty();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictElement& paramToReadelem, SumRules& rules) {
	bool has_sum = false;
	for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = propertyParams.EnumeratePairs(); cIt != NULL; ++cIt) {
		ParamValue& param = *cIt->value;
		API_PropertyDefinition& definition = param.definition;

		// Является ли свойство описанием системы суммирования?
		if (param.definition.description.Contains("Sum") && param.definition.description.Contains("{") && param.definition.description.Contains("}")) {
			bool flag_add = false;
			if (!rules.ContainsKey(definition.guid)) {
				SumRule paramtype = {};
				if (Sum_Rule(elemGuid, definition, propertyParams, paramtype)) {
					paramtype.position = param.rawName;
					rules.Add(definition.guid, paramtype);
					flag_add = true;
				}
			}
			else {
				flag_add = true;
			}
			if (flag_add == true) {

				// Дописываем элемент в правило
				rules.Get(definition.guid).elemts.Push(elemGuid);

				// Добавляем свойства для чтения в словарь
				SumRule paramtype = rules.Get(definition.guid);
				if (!paramtype.position.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.position), paramToReadelem);
				if (!paramtype.value.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.value), paramToReadelem);
				if (!paramtype.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDictElement(elemGuid, propertyParams.Get(paramtype.criteria), paramToReadelem);
				has_sum = true;
			}
		}
	}
	return has_sum;
}// Sum_GetElement

// -----------------------------------------------------------------------------------------------------------------------
// Функция возвращает заполненное правило суммирования SumRule
// SumRule.sum_type - тип суммирования (TextSum / NumSum)
// SumRule.value - имя свойства в формате rawname
// SumRule.criteria - критерий суммирования (разбивки)
// SumRule.delimetr - разделитель для текстовой суммы (конкатенации)
// SumRule.ignore_val - игнорируемые значения
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_Rule(const API_Guid& elemGuid, const API_PropertyDefinition& definition, ParamDictValue& propertyParams, SumRule& paramtype) {

	// По типу данных свойства определим тим суммирования
	// Если строковый тип - объединяем уникальные значения, если тип числовой - суммируем
	paramtype.sum_type = 0;
	if (definition.valueType == API_PropertyStringValueType) paramtype.sum_type = TextSum;
	if (definition.valueType == API_PropertyRealValueType || definition.valueType == API_PropertyIntegerValueType) paramtype.sum_type = NumSum;
	if (!paramtype.sum_type) return false;

	GS::UniString paramName = definition.description.GetSubstring('{', '}', 0);
	GS::Array<GS::UniString>	partstring;
	int nparam = StringSplt(paramName.ToLowerCase(), ";", partstring);
	if (nparam == 0) return false;

	if (propertyParams.ContainsKey("{" + partstring[0] + "}")) {
		paramtype.value = "{" + partstring[0] + "}";
	}
	else {
		return false;
	}

	// Ищём определение свойства-критерия
	if (nparam > 1) {
		if (propertyParams.ContainsKey("{" + partstring[1] + "}")) {
			paramtype.criteria = "{" + partstring[1] + "}";
		}
		else {
			paramtype.delimetr = partstring[1].ToCStr().Get();
		}
	}

	// Если задан и разделитель - пропишем его
	if (nparam == 3) {
		if (paramtype.delimetr.empty()) {
			paramtype.delimetr = partstring[3].ToCStr().Get();
		}
		else {
			paramtype.ignore_val = partstring[3].ToCStr().Get();
		}
	}

	// Если заданы игнорируемые значения
	if (nparam == 4) paramtype.ignore_val = partstring[4].ToCStr().Get();
	return true;
} // ReNumRule

void Sum_OneRule(const SumRule& rule, const API_WorkingUnitPrefs& unitPrefs, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem) {
	SumCriteria criteriaList;
	GS::UniString delimetr = GS::UniString(rule.delimetr.c_str());

	// Выбираем значения критериев
	for (UInt32 i = 0; i < rule.elemts.GetSize(); i++) {
		if (paramToReadelem.ContainsKey(rule.elemts[i])) {
			ParamDictValue params = paramToReadelem.Get(rule.elemts[i]);
			if (!rule.criteria.IsEmpty() && params.ContainsKey(rule.criteria)) {
				std::string criteria = params.Get(rule.criteria).val.uniStringValue.ToCStr(0, MaxUSize, GChCode).Get();
				criteriaList[criteria].inx.Push(i);
			}
			else {
				criteriaList["all"].inx.Push(i);
			}
		}
	}

	// Проходим по словарю с критериями и суммируем
	for (SumCriteria::iterator i = criteriaList.begin(); i != criteriaList.end(); ++i) {
		GS::Array<UInt32> eleminpos = i->second.inx;
		ParamValue summ; // Для суммирования числовых значений
		bool has_sum = false;
		for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
			API_Guid elemGuid = rule.elemts[eleminpos[j]];
			ParamDictValue params = paramToReadelem.Get(elemGuid);
			if (params.ContainsKey(rule.value)) {

				// Проверяем - было ли считано значение
				ParamValue param = params.Get(rule.value);
				if (param.isValid) {
					has_sum = true;
					if (rule.sum_type == TextSum) {
						summ.val.uniStringValue = summ.val.uniStringValue + param.val.uniStringValue;
						if (j < eleminpos.GetSize() - 1) summ.val.uniStringValue = summ.val.uniStringValue + delimetr;
					}
					else {
						double val = param.val.doubleValue;
						if (unitPrefs.roundInch) { // В зависимости от настроек - складываем с округлением.
							val = int(val * 100 + 0.5) / 100.0;
						}
						summ.val.doubleValue = summ.val.doubleValue + val;
						summ.val.intValue = summ.val.intValue + param.val.intValue;
						summ.val.boolValue = summ.val.boolValue && param.val.boolValue;
					}
				}
			}
		}

		// Для конкатенации текста определим уникальные значения
		if (rule.sum_type == TextSum) {
			GS::UniString unic = StringUnic(summ.val.uniStringValue, delimetr);
			summ.val.uniStringValue = unic;
		}

		// Заполнение словаря записи
		if (has_sum == true) {
			for (UInt32 j = 0; j < eleminpos.GetSize(); j++) {
				API_Guid elemGuid = rule.elemts[eleminpos[j]];
				ParamDictValue params = paramToReadelem.Get(elemGuid);
				if (params.ContainsKey(rule.position)) {
					ParamValue param = params.Get(rule.position);
					param.isValid = true;
					summ.val.type = param.val.type;

					// Записываем только изменённые значения
					if (param != summ) {
						param.val = summ.val;
						ParamHelpers::AddParamValue2ParamDictElement(elemGuid, param, paramToWriteelem);
					}
				}
			}
		}
	}
	return;
}