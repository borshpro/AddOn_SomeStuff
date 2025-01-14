﻿//------------ kuvbur 2022 ------------
#include	<map>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Summ.hpp"
#include	"Sync.hpp"
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
	long time_start = clock();
	GS::UniString funcname = "Summation";
	GS::Int32 nPhase = 1;
#ifdef AC_27
	bool showPercent = true;
	ACAPI_ProcessWindow_InitProcessWindow(&funcname, &nPhase);
#else
	ACAPI_Interface(APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true, syncSettings, true);
	if (guidArray.IsEmpty()) return NoError;
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	bool flag_write = true;
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		GS::UniString subtitle = GS::UniString::Printf("Reading data from %d elements", guidArray.GetSize());; short i = 1;
#ifdef AC_27
		Int32 maxval = 2;
		ACAPI_ProcessWindow_SetNextProcessPhase(&subtitle, &maxval, &showPercent);
#else
		ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
		ParamDictElement paramToWriteelem;
		if (!GetSumValuesOfElements(guidArray, paramToWriteelem)) {
			flag_write = false;
			msg_rep("SumSelected", "No data to write", NoError, APINULLGuid);
#ifdef AC_27
			ACAPI_ProcessWindow_CloseProcessWindow();
#else
			ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
			return NoError;
		}
		subtitle = GS::UniString::Printf("Writing data to %d elements", paramToWriteelem.GetSize()); i = 2;
#ifdef AC_27
		ACAPI_ProcessWindow_SetNextProcessPhase(&subtitle, &maxval, &showPercent);
#else
		ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
		ParamHelpers::ElementsWrite(paramToWriteelem);
		long time_end = clock();
		GS::UniString time = GS::UniString::Printf(" %d s", (time_end - time_start) / 1000);
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d ", guidArray.GetSize()) + GS::UniString::Printf("wrtite to - %d", paramToWriteelem.GetSize()) + time;
		msg_rep("SumSelected", intString, NoError, APINULLGuid);
#ifdef AC_27
		ACAPI_ProcessWindow_CloseProcessWindow();
#else
		ACAPI_Interface(APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
		return NoError;
		});
	if (flag_write) SyncArray(syncSettings, guidArray);
	return NoError;
}

bool GetSumValuesOfElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem) {
	if (guidArray.IsEmpty()) return false;
	SumRules rules;
	ParamDictElement paramToReadelem;
	GS::UniString subtitle = GS::UniString::Printf("Reading data from %d elements", guidArray.GetSize());

	// Получаем список правил суммирования
	bool hasSum = false;
	for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
#ifdef AC_27
		bool showPercent = true;
		Int32 maxval = guidArray.GetSize();
		if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase(&subtitle, &maxval, &showPercent);
#else
		if (i % 10 == 0) ACAPI_Interface(APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#ifdef AC_27
		if (ACAPI_ProcessWindow_IsProcessCanceled()) return false;
#else
		if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) return false;
#endif
		ParamDictValue propertyParams;
		ParamDictValue paramToRead;
		ParamHelpers::AllPropertyDefinitionToParamDict(propertyParams, guidArray[i]);
		if (!propertyParams.IsEmpty()) {
			if (Sum_GetElement(guidArray[i], propertyParams, paramToRead, rules)) {
				ParamHelpers::Read(guidArray[i], paramToRead, propertyParams);
				ParamHelpers::AddParamDictValue2ParamDictElement(guidArray[i], paramToRead, paramToReadelem);
				hasSum = true;
			}
		}
	}
	if (!hasSum) return false;

	// Суммируем, заполняе словарь для записи
	for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
		const SumRule& rule = *cIt->value;
		if (!rule.elemts.IsEmpty()) Sum_OneRule(rule, paramToReadelem, paramToWriteelem);
	}
	return !paramToWriteelem.IsEmpty();
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
bool Sum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue& paramToRead, SumRules& rules) {
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
				if (!paramtype.position.IsEmpty()) ParamHelpers::AddParamValue2ParamDict(elemGuid, propertyParams.Get(paramtype.position), paramToRead);
				if (!paramtype.value.IsEmpty()) ParamHelpers::AddParamValue2ParamDict(elemGuid, propertyParams.Get(paramtype.value), paramToRead);
				if (!paramtype.criteria.IsEmpty()) ParamHelpers::AddParamValue2ParamDict(elemGuid, propertyParams.Get(paramtype.criteria), paramToRead);
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
	paramName.ReplaceAll("\\/", "/");
	GS::Array<GS::UniString>	partstring;
	int nparam = StringSplt(paramName.ToLowerCase(), ";", partstring);
	if (nparam == 0) return false;
	if (propertyParams.ContainsKey("{@" + partstring[0] + "}")) {
		paramtype.value = "{@" + partstring[0] + "}";
	}
	else {
		return false;
	}

	// Ищём определение свойства-критерия
	if (nparam > 1) {
		if (propertyParams.ContainsKey("{@" + partstring[1] + "}")) {
			paramtype.criteria = "{@" + partstring[1] + "}";
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

void Sum_OneRule(const SumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem) {
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
						summ.val.doubleValue = summ.val.doubleValue + param.val.doubleValue;
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
					if (rule.sum_type != TextSum) {
						summ.val.stringformat = param.val.stringformat;
						summ.val.uniStringValue = ParamHelpers::ToString(summ);
					}

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