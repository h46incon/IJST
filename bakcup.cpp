//
// Created by h46incon on 2017/9/20.
//
#if IJST_JSON_LIB == "JSON_CPP"
	#include "json/json.h"
	typedef Json::Value StoreType;
	#define IJSTI_STORE_LIST 		Json::Value(Json::arrayValue)
	#define IJSTI_STORE_MAP 		Json::Value(Json::objectValue)
	#define IJSTI_STORE_NULL		Json::Value(Json::nullValue)
	#define IJSTI_STORE_MOVE(dest, src)			dest.swap(src)
#elif IJST_JSON_LIB == "RAPID_JSON"

class SerializerDispatcher {
private:
	typedef IJMapType<FType::_E, SerializerInterface*> InterfaceMap;
public:
	SerializerInterface *Get(FType::_E ftype)
	{
		InterfaceMap::iterator itera = mapInterface.find(ftype);
		if (itera == mapInterface.end()) {
			throw std::runtime_error("Unimplement ftype: " + ftype);
		}
		return itera->second;
	}

private:
	template <FType::_E ftype> friend class SerializerRegister;
	InterfaceMap mapInterface;
};

typedef Singleton<SerializerDispatcher> SerializerDispatcherS;

template<class ftype, unsigned int fdecorate>
struct FVarType {
	typedef void Type;
};
#define GetVarType(ftype, fdesc) FVarType<(ftype), (desc) & FDesc::_MaskDecorate>::Type


template<class _T>
struct FVarType<_T, 0> {
	typedef typename FSerializer<_T>::VarType Type;
};

template<class _T>
struct FVarType<_T, FDesc::Repeat> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename std::vector<ElemType> Type;
};

template<class _T>
struct FVarType<_T, FDesc::Repeat_GT1> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename std::vector<ElemType> Type;
};

template<class _T>
struct FVarType<_T, FDesc::Map> {
	typedef typename FSerializer<_T>::VarType ElemType;
	typedef typename IJMapType<std::string, ElemType> Type;
};

template <FType::_E ftype>
class SerializerRegister{
public:
	SerializerRegister()
	{
		std::cout << "SerializerRegister: " << ftype << std::endl;
		SerializerDispatcherS::GetInstance()->mapInterface[ftype] = FSerializerS(ftype)::GetInstance();
	}
};

SerializerRegister<FType::Int> _reg_Int;
SerializerRegister<FType::String> _reg_String;

	int SSS(const std::string& fieldName, SerializerInterface::SerializeReq& req, SerializerInterface* serializerInterface)
	{
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(fieldName.c_str(), fieldName.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		// Try add field when need
		StoreType* pNewElem = IJSTI_NULL;
		bool hasAllocMember = false;
		{
			rapidjson::Value::MemberIterator itMember = req.buffer.FindMember(fieldNameVal);
			if (itMember == req.buffer.MemberEnd()) {
				// Add member, field name is not need copy
				req.buffer.AddMember(
						rapidjson::Value().SetString(fieldNameRef),
						rapidjson::Value(rapidjson::kNullType).Move(),
						req.allocator
				);
				// Why rapidjson AddMember function do not return newly create member
				pNewElem = &req.buffer[fieldNameVal];
				hasAllocMember = true;
			}
			else {
				pNewElem = &(itMember->value);
			}
		}

		SerializerInterface::SerializeReq elemSerializeReq(
				*pNewElem, req.allocator, req.field, req.pushAllField, req.tryInPlace);

		SerializerInterface::SerializeResp elemSerializeResp;
		int ret = serializerInterface->Serialize(elemSerializeReq, elemSerializeResp);
		if (ret != 0) {
			if (hasAllocMember) {
				req.buffer.RemoveMember(fieldNameVal);
			}
			return ret;
		}
		assert(&(req.buffer[fieldNameVal]) == &elemSerializeReq.buffer);
		return 0;
	}
