//
// Created by h46incon on 2017/9/19.
//

#ifndef _IJST_HPP_INCLUDE_
#define _IJST_HPP_INCLUDE_

#include "std_layout_wrapper.h"
#include "detail/detail.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <cassert>		// assert
#include <cstddef>		// NULL, size_t, offsetof
#include <algorithm>	// std::sort, std::lower_bound
#include <vector>
#include <string>
#include <sstream>

/**	========================================================================================
 *				Public Interface
 */

/**
 * IJST_ENABLE_TO_JSON_OBJECT
 * 	By default, ijst can only serialize the structure to string.
 * 	If users need to serialize the structure to a rapidjson::Value, specify this flag to 1.
 */
#ifndef IJST_ENABLE_TO_JSON_OBJECT
	#define IJST_ENABLE_TO_JSON_OBJECT 			0
#endif

/**
 * IJST_ENABLE_FROM_JSON_OBJECT
 * 	By default, ijst can only deserialize the structure from string.
 * 	If users need to deserialize the structure from a rapidjson::Value, specify this flag to 1.
 */
#ifndef IJST_ENABLE_FROM_JSON_OBJECT
	#define IJST_ENABLE_FROM_JSON_OBJECT 		0
#endif

/**
 * IJST_AUTO_META_INIT
 *	if define IJST_AUTO_META_INIT before include this header, the meta class information will init before main.
 *	That's will make it thread-safe to init meta class information before C++11.
 *	The feature is enable default before C++11. So set the value to 0 to force disable it.
 */
#ifndef IJST_AUTO_META_INIT
	#if __cplusplus < 201103L
		#define IJST_AUTO_META_INIT		1
	#else
		#define IJST_AUTO_META_INIT		0
	#endif
#endif

/**
 * IJST_ASSERT
 *	By default, ijst uses assert() for errors that indicate a bug.
 *	User can override it by defining IJST_ASSERT(x) macro.
 */
#ifndef IJST_ASSERT
	#define IJST_ASSERT(x) assert(x)
#endif

/**
 * IJST_USE_SL_WRAPPER
 *	By default, ijst uses std::vector, std::map, and std::string when defining struct.
 *	User can specify IJST_USE_SL_WRAPPER to 1 to use ijst::Vector, ijst::Map, ijst::SLWrapper<std::string>.
 *	This option will make the behaviour more standards-compliant, but will bring a little inconvenience when using.
 */
#ifndef IJST_USE_SL_WRAPPER
	#define IJST_USE_SL_WRAPPER	0
#endif

/**
 * IJST_OFFSETOF
 *	By default, the offset of each field is computed by "((Struct*)0)->field" when IJST_USE_SL_WRAPPER is false,
 *	and macro "offsetof()" when IJST_USL_SL_WRAPPER is true
 */
#ifndef IJST_OFFSETOF
	#if IJST_USE_SL_WRAPPER
		#define IJST_OFFSETOF(_T, member)	offsetof(_T, member)
	#else
		#define IJST_OFFSETOF(_T, member)	((size_t)&(((_T*)0)->member))
	#endif
#endif



#define IJST_OUT

//! Declare a ijst struct.
#define IJST_DEFINE_STRUCT(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, F, __VA_ARGS__)
//! Declare a ijst struct with getter.
#define IJST_DEFINE_STRUCT_WITH_GETTER(...) \
    IJSTI_DEFINE_STRUCT_IMPL(IJSTI_PP_NFIELD(__VA_ARGS__), false, T, __VA_ARGS__)
//! Declare a ijst struct which represent a value instead of members insides a object
#define IJST_DEFINE_VALUE(stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, F, stName, (type, fName, "JSON_ITSELF", desc))
//! Declare a ijst struct which represent a value instead of members insides a object with getter
#define IJST_DEFINE_VALUE_WITH_GETTER(stName, type, fName, desc)	\
    IJSTI_DEFINE_STRUCT_IMPL(1, true, T, stName, (type, fName, "JSON_ITSELF", desc))

//! Get status of field in obj.
#define IJST_GET_STATUS(obj, field)				obj._.GetStatus(& ((obj).field))
//! Mark status of field in obj to FStatus::kValid.
#define IJST_MARK_VALID(obj, field)				obj._.MarkValid(& ((obj).field))
//! Mark status of field in obj to FStatus::kNull.
#define IJST_MARK_NULL(obj, field)				obj._.MarkNull(& ((obj).field))
//! Mark status of field in obj to FStatus::kMissing.
#define IJST_MARK_MISSING(obj, field)			obj._.MarkMissing(& ((obj).field))
//! Set field in obj to val and mark it valid.
#define IJST_SET(obj, field, val)				obj._.Set((obj).field, (val))
//! Set field in obj to val and mark it valid. Type of field and val must be same.
#define IJST_SET_STRICT(obj, field, val)		obj._.SetStrict((obj).field, (val))

//! IJST_NULL.
#define IJST_NULL				IJSTI_NULL

//! Wrappers.
#if IJST_USE_SL_WRAPPER
	#define IJST_CONT_VAL(_v)			(_v).Val()
#else
	#define IJST_CONT_VAL(_v)			(_v)
#endif

namespace ijst {
typedef rapidjson::Value JsonValue;
typedef rapidjson::MemoryPoolAllocator<> 	JsonAllocator;

//! Field description.
struct FDesc {
	typedef unsigned int Mode;
	static const Mode _MaskDesc 		= 0x000000FF;
	static const Mode Optional 			= 0x00000001;
	static const Mode Nullable 			= 0x00000002;
	static const Mode ElemNotEmpty 		= 0x00000004;
	// FDesc of Element (i.e. Nullable, ElemNotEmpty) inside container is hard to represent, has not plan to implement it
};

//! Field status.
struct FStatus {
public:
	enum _E {
		kNotAField,
		kMissing,
		kNull,
		kParseFailed,
		kValid,
	};
};
typedef FStatus::_E EFStatus;

//! Serialization options about fields.
//! Options can be combined by bitwise OR operator (|).
struct FPush {
	typedef unsigned int Mode;
	//! does not set any option
	static const Mode kZero								= 0x00000000;
	//! set if need serialize all field, otherwise will serialize only valid field
	static const Mode kPushAllFields					= 0x00000001;
	//! set if need serialize unknown field
	static const Mode kPushUnknown						= 0x00000002;
};

//! Behaviour when meeting unknown member in json object.
struct UnknownMode {
public:
	enum _E {
		kKeep,		// keep unknown fields
		kIgnore,	// ignore unknown fields
		kError		// error when unknown fields occurs
	};
};
typedef UnknownMode::_E EUnknownMode;

//! Error code.
struct Err {
	static const int kSucc 							= 0x00000000;
	static const int kDeserializeValueTypeError 	= 0x00001001;
	static const int kDeserializeSomeFiledsInvalid 	= 0x00001002;
	static const int kDeserializeParseFaild 		= 0x00001003;
	static const int kDeserializeElemEmpty 			= 0x00001004;
	static const int kDeserializeSomeUnknownMember	= 0x00001005;
	static const int kInnerError 					= 0x00002001;
	static const int kWriteFailed					= 0x00003001;
};

#define IJSTI_OPTIONAL_BASE_DEFINE(_T)						\
	public:													\
		explicit Optional(_T* _pVal) : m_pVal(_pVal) {}		\
		_T* Ptr() const { return m_pVal; }					\
	private:												\
		_T* const m_pVal;

/**
 * Optional helper for getter chaining
 * @tparam _T 	type
 *
 * @note	The specialized template for container is declared in "types_container.h",
 * 			which implements operator []
 */
template <typename _T>
class Optional
{
	typedef _T ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	_T* operator->() const
	{
		static _T empty(false);
		if (m_pVal == IJST_NULL) {
			return &empty;
		}
		else {
			return m_pVal;
		}
	}
};

/**	========================================================================================
 *				Inner Interface
 */
namespace detail {
	typedef rapidjson::Writer<rapidjson::StringBuffer> JsonWriter;

	//! return Err::kWriteFailed if action return false
	#define IJSTI_RET_WHEN_WRITE_FAILD(action) 					\
		do { if(!(action)) return Err::kWriteFailed; } while (false)
	//! return if action return non-0
	#define IJSTI_RET_WHEN_NOT_ZERO(action) 					\
		do { int ret = (action); if(ret != 0) return (ret); } while (false)

	#if IJST_AUTO_META_INIT
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)			::ijst::detail::Singleton< _T>::InitInstanceBeforeMain();
	#else
		#define IJSTI_TRY_INIT_META_BEFORE_MAIN(_T)
	#endif

	class SerializerInterface {
	public:
		virtual ~SerializerInterface() { }

		struct SerializeReq {
			// Serialize option about fields
			FPush::Mode fPushMode;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			JsonWriter& writer;

			SerializeReq(JsonWriter& _writer, const void *_pField, FPush::Mode _fPushMode)
					: fPushMode(_fPushMode)
					, pField(_pField)
					, writer(_writer)
			{ }
		};

		virtual int Serialize(const SerializeReq &req) = 0;

#if IJST_ENABLE_TO_JSON_OBJECT
		struct ToJsonReq {
			// ToJson option about fields
			FPush::Mode fPushMode;

			// Pointer of field to serialize.
			// The actual type of field should be decide in the derived class
			const void* pField;

			// true if move context in pField to avoid copy when possible
			bool canMoveSrc;

			// Output buffer info. The instance should serialize in this object and use allocator
			IJST_OUT JsonValue& buffer;
			JsonAllocator& allocator;

			ToJsonReq(JsonValue &_buffer, JsonAllocator &_allocator,
					  const void *_pField, bool _canMoveSrc,
					  FPush::Mode _fPushMode)
					: fPushMode(_fPushMode)
					  , pField(_pField)
					  , canMoveSrc(_canMoveSrc)
					  , buffer(_buffer)
					  , allocator(_allocator)
			{ }
		};

		virtual int ToJson(const ToJsonReq &req)= 0;

		virtual int SetAllocator(void *pField, JsonAllocator &allocator)
		{ (void) pField; (void) allocator; return 0; }
#endif // #if IJST_ENABLE_TO_JSON_OBJECT

	struct FromJsonReq {
		// Pointer of deserialize output.
		// The instance should deserialize in this object
		// The actual type of field should be decide in the derived class
		void* pFieldBuffer;

		// The input stream and allocator
		JsonValue& stream;
		JsonAllocator& allocator;

		// true if move context in stream to avoid copy when possible
		bool canMoveSrc;
		EUnknownMode unknownMode;

		FromJsonReq(JsonValue &_stream, JsonAllocator &_allocator,
					EUnknownMode _unknownMode, bool _canMoveSrc,
					void *_pField)
				: pFieldBuffer(_pField)
				  , stream(_stream)
				  , allocator(_allocator)
				  , canMoveSrc(_canMoveSrc)
				  , unknownMode(_unknownMode)
		{ }
	};

	struct FromJsonResp {
		EFStatus fStatus;
		size_t fieldCount;
		DeserializeErrDoc errDoc;

		explicit FromJsonResp(JsonAllocator* _pAllocator) :
				fStatus(FStatus::kMissing),
				fieldCount(0),
				errDoc(_pAllocator)
		{ }

	};

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp)= 0;

	};

	/**
	 * Template interface of serialization class
	 * This template is unimplemented, and will throw a compile error when use it.
	 * @tparam _T class
	 */
	template<class _T>
	class FSerializer : public SerializerInterface {
	public:
		#if __cplusplus >= 201103L
		static_assert(!std::is_same<_T, _T>::value,
					  "This base template should not be instantiated. (Maybe use wrong param when define ijst struct)");
		#endif

		typedef void VarType;

		virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE = 0;

#if IJST_ENABLE_TO_JSON_OBJECT
		virtual int ToJson(const ToJsonReq &req) IJSTI_OVERRIDE = 0;
		virtual int SetAllocator(void *pField, JsonAllocator &allocator) IJSTI_OVERRIDE;
#endif

		virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE = 0;
	};

	#define IJSTI_FSERIALIZER_INS(_T) ::ijst::detail::Singleton< ::ijst::detail::FSerializer< _T> >::GetInstance()

	/**	========================================================================================
	 *				Private
	 */

	struct MetaField { // NOLINT
		int index;
		FDesc::Mode desc;
		std::size_t offset;
		SerializerInterface *serializerInterface;
		std::string name;
	};

	class MetaClass {
	public:
		MetaClass() : accessorOffset(0), mapInited(false) { }

		void InitBegin(const std::string& _tag, std::size_t _fieldCount, std::size_t _accessorOffset)
		{
			tag = _tag;
			accessorOffset = _accessorOffset;
			metaFields.reserve(_fieldCount);
		}

		void PushMetaField(const std::string &name, std::size_t offset,
						   FDesc::Mode desc, SerializerInterface *serializerInterface)
		{
			MetaField metaField;
			metaField.name = name;
			metaField.offset = offset;
			metaField.desc = desc;
			metaField.serializerInterface = serializerInterface;
			metaFields.push_back(IJSTI_MOVE(metaField));
		}

		void InitEnd()
		{
			// assert MetaClass's map has not inited before
			assert(!mapInited);
			SortMetaFieldsByOffset();

			m_nameMap.reserve(metaFields.size());
			m_offsets.reserve(metaFields.size());

			for (size_t i = 0; i < metaFields.size(); ++i) {
				MetaField *ptrMetaField = &(metaFields[i]);
				ptrMetaField->index = static_cast<int>(i);

				m_nameMap.push_back(NameMap(&(ptrMetaField->name), ptrMetaField));
				m_offsets.push_back(ptrMetaField->offset);

				// Assert field offset is sorted and not exist before
				assert(i == 0 || m_offsets[i]  > m_offsets[i-1]);
			}

			std::sort(m_nameMap.begin(), m_nameMap.end());
			IJST_ASSERT(CheckNameMapSortedAndUnique());
			mapInited = true;
		}

		int FindIndex(size_t offset) const
		{
			std::vector<size_t>::const_iterator it =
					std::lower_bound(m_offsets.begin(), m_offsets.end(), offset);
			if (it != m_offsets.end() && *it == offset) {
				return static_cast<int>(it - m_offsets.begin());
			}
			else {
				return -1;
			}
		}

		const MetaField* FindByName(const std::string &name) const
		{
			std::vector<NameMap>::const_iterator it =
					std::lower_bound(m_nameMap.begin(), m_nameMap.end(), name, NameMapComp);
			if (it != m_nameMap.end() && (*it->pName) == name) {
				return it->metaField;
			}
			else {
				return IJST_NULL;
			}
		}

		std::vector<MetaField> metaFields;
		std::string tag;
		std::size_t accessorOffset;

	private:
		struct NameMap {
			NameMap(const std::string* _pName, const MetaField* _metaField)
					: pName(_pName), metaField(_metaField) {}

			bool operator<(const NameMap &r) const
			{ return (*pName) < (*r.pName); }

			const std::string* pName;
			const MetaField* metaField;
		};

		static bool NameMapComp(const NameMap &l, const std::string &name)
		{
			return (*l.pName) < name;
		}

		void SortMetaFieldsByOffset()
		{
			// metaFields is already sorted in most case, use insertion sort
			const size_t n = metaFields.size();
			for (size_t i = 1; i < n; i++) {
				for (size_t j = i; j > 0 && metaFields[j - 1].offset > metaFields[j].offset; j--) {
					detail::Swap(metaFields[j], metaFields[j - 1]);
				}
			}
		}

		bool CheckNameMapSortedAndUnique() const
		{
			for (size_t i = 1; i < m_nameMap.size(); ++i) {
				if (!((*m_nameMap[i - 1].pName) < (*m_nameMap[i].pName))) {
					return false;		// Maybe the field's json name is duplicate
				}
			}
			return true;
		}

		std::vector<NameMap> m_nameMap;
		std::vector<size_t> m_offsets;

		bool mapInited;
	};

	/**
	 * Reflection info.
	 * Push meta class info of _T in specialized constructor MetaInfo<_T>().
	 * @tparam _T: class. Concept require _T::InitMetaInfo(MetaInfo*)
	 */
	template<class _T>
	class MetaInfo {
	public:
		MetaClass metaClass;

	private:
		friend class Singleton<MetaInfo<_T> >;

		MetaInfo()
		{
			_T::InitMetaInfo(this);
		}
	};

}	// namespace detail

/**
 * Struct Accessor.
 * User can access and modify fields, serialize and deserialize of a structure.
 */
class Accessor {
public:
	//region constructors
	explicit Accessor(const detail::MetaClass *pMetaClass, bool isParentVal, bool isValid) :
			m_pMetaClass(pMetaClass), m_isValid(isValid), m_isParentVal(isParentVal)
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->metaFields.size() == 1);
		m_pFieldStatus = new FieldStatusType(m_pMetaClass->metaFields.size(), FStatus::kMissing);
		m_pUnknown = new rapidjson::Value(rapidjson::kObjectType);
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		InitOuterPtr();
	}

	Accessor(const Accessor &rhs)
	{
		assert(this != &rhs);

		m_isValid = rhs.m_isValid;
		m_isParentVal = rhs.m_isParentVal;

		m_pFieldStatus = new FieldStatusType(*rhs.m_pFieldStatus);
		m_pUnknown = new rapidjson::Value(rapidjson::kObjectType);
		m_pOwnDoc = new rapidjson::Document();
		m_pAllocator = &m_pOwnDoc->GetAllocator();

		m_pMetaClass = rhs.m_pMetaClass;
		InitOuterPtr();

		m_pUnknown->CopyFrom(*rhs.m_pUnknown, *m_pAllocator, true);
	}

	#if __cplusplus >= 201103L
	Accessor(Accessor &&rhs) IJSTI_NOEXCEPT
	{
		m_pUnknown = IJST_NULL;
		m_pOwnDoc = IJST_NULL;
		m_pFieldStatus = IJST_NULL;
		Steal(rhs);
	}
	#endif

	Accessor &operator=(Accessor rhs)
	{
		Steal(rhs);
		return *this;
	}

	~Accessor() IJSTI_NOEXCEPT
	{
		delete m_pUnknown;
		m_pUnknown = IJST_NULL;
		delete m_pOwnDoc;
		m_pOwnDoc = IJST_NULL;
		delete m_pFieldStatus;
		m_pFieldStatus = IJST_NULL;
	}
	//endregion

	//! Steal other Accessor object.
	void Steal(Accessor &rhs)
	{
		if (this == &rhs) {
			return;
		}

		// Handler resource
		delete m_pUnknown;
		m_pUnknown = rhs.m_pUnknown;
		rhs.m_pUnknown = IJST_NULL;

		delete m_pOwnDoc;
		m_pOwnDoc = rhs.m_pOwnDoc;
		rhs.m_pOwnDoc = IJST_NULL;

		delete m_pFieldStatus;
		m_pFieldStatus = rhs.m_pFieldStatus;
		rhs.m_pFieldStatus = IJST_NULL;

		// other simple field
		m_pMetaClass = rhs.m_pMetaClass;
		rhs.m_pMetaClass = IJST_NULL;
		m_pAllocator = rhs.m_pAllocator;
		rhs.m_pAllocator = IJST_NULL;

		m_isValid = rhs.m_isValid;
		m_isParentVal = rhs.m_isParentVal;

		InitOuterPtr();
	}

	inline bool IsValid() const { return m_isValid; }
	inline bool IsParentVal() const { return m_isParentVal; }

	/*
	 * Field accessor.
	 */
	//! Check if pField is a filed in this object.
	inline bool HasField(const void *pField) const
	{
		size_t offset = GetFieldOffset(pField);
		return (m_pMetaClass->FindIndex(offset) != -1);
	}

	//! Set field to val and mark it valid.
	template<typename _T1, typename _T2>
	inline void Set(_T1 &field, const _T2 &value)
	{
		MarkValid(&field);
		field = value;
	}

	//! Set field to val and mark it valid. The type of field and value must be same.
	template<typename _T>
	inline void SetStrict(_T &field, const _T &value)
	{
		Set(field, value);
	}

	//! Mark status of field to FStatus::kValid.
	inline void MarkValid(const void* pField) { MarkFieldStatus(pField, FStatus::kValid); }
	//! Mark status of field to FStatus::kNull.
	inline void MarkNull(const void* pField) { MarkFieldStatus(pField, FStatus::kNull); }
	//! Mark status of field to FStatus::kMissing.
	inline void MarkMissing(const void* pField) { MarkFieldStatus(pField, FStatus::kMissing); }

	//! Get status of field.
	inline EFStatus GetStatus(const void *pField) const
	{
		const size_t offset = GetFieldOffset(pField);
		const int index = m_pMetaClass->FindIndex(offset);
		return index == -1 ? FStatus::kNotAField : (*m_pFieldStatus)[index];
	}

	//! Get unknwon fields
	inline JsonValue &GetUnknown() { return *m_pUnknown; }
	inline const JsonValue &GetUnknown() const { return *m_pUnknown; }

	//! Get allocator used in object.
	//! The inner allocator is own allocator when init, but may change to other allocator
	//! when calling SetMembersAllocator() or Deserialize().
	inline JsonAllocator &GetAllocator() { return *m_pAllocator; }
	inline const JsonAllocator &GetAllocator() const { return *m_pAllocator; }

	//! Get own allocator that used to manager resource.
	//! User could use the returned value to check if this object use outer allocator.
	inline JsonAllocator &GetOwnAllocator() { return m_pOwnDoc->GetAllocator(); }
	inline const JsonAllocator &GetOwnAllocator() const { return m_pOwnDoc->GetAllocator(); }

	/**
	 * Serialize the structure to string.
	 * @param strOutput 		The output of result
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 */
	int Serialize(IJST_OUT std::string &strOutput, FPush::Mode fieldPushMode = FPush::kPushAllFields | FPush::kPushUnknown)  const
	{
		rapidjson::StringBuffer buffer;
		detail::JsonWriter writer(buffer);
		IJSTI_RET_WHEN_NOT_ZERO(DoSerialize(writer, fieldPushMode));

		strOutput = std::string(buffer.GetString(), buffer.GetLength());
		return 0;
	}

	/**
	 * Deserialize from C-style string.
	 * @param cstrInput			Input C string
	 * @param length			Length of string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The input string can contain '\0'
	 */
	int Deserialize(const char *cstrInput, std::size_t length, EUnknownMode unknownMode = UnknownMode::kKeep,
					std::string *pErrMsgOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces soon in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.Parse(cstrInput, length);
		if (doc.HasParseError())
		{
			if (pErrMsgOut != IJST_NULL)
			{
				*pErrMsgOut = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kDeserializeParseFaild;
		}

		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, unknownMode, pErrMsgOut);
	}

	/**
	 * Deserialize from std::string.
	 * @param strInput			Input string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput, EUnknownMode unknownMode = UnknownMode::kKeep,
						   std::string *pErrMsgOut = IJST_NULL)
	{
		return Deserialize(strInput.c_str(), strInput.length(), unknownMode, pErrMsgOut);
	}

	/**
	 * Deserialize from std::string.
	 * @param strInput			Input string
	 * @param errMsgOut			Error message output
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @return					Error code
	 */
	inline int Deserialize(const std::string &strInput, IJST_OUT std::string& errMsgOut, EUnknownMode unknownMode = UnknownMode::kKeep)
	{
		return Deserialize(strInput.c_str(), strInput.length(), unknownMode, &errMsgOut);
	}

	/**
	 * Deserialize insitu from str.
	 * @param cstrInput			Input C string
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note The context in str may be changed after deserialize
	 * @note Make sure the lifecycle of str is longer than this object
	 */
	int DeserializeInsitu(char *str, EUnknownMode unknownMode = UnknownMode::kKeep,
						  std::string *pErrMsgOut = IJST_NULL)
	{
		// The new object will call FromJson() interfaces in most situation
		// So clear own allocator will not bring much benefice
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		rapidjson::Document doc(m_pAllocator);
		doc.ParseInsitu(str);
		if (doc.HasParseError())
		{
			if (pErrMsgOut != IJST_NULL)
			{
				*pErrMsgOut = IJSTI_MOVE(std::string(
						rapidjson::GetParseError_En(doc.GetParseError())
				));
			}
			return Err::kDeserializeParseFaild;
		}
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, doc, unknownMode, pErrMsgOut);
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	/**
	 * Serialize the structure to a JsonValue object.
	 * @param output 			The output of result
	 * @param allocator	 		Allocator when adding members to output
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note If using inner allocator, user should carefully handler the structure's life cycle
	 */
	inline int ToJson(IJST_OUT JsonValue &output, JsonAllocator &allocator,
					  FPush::Mode fieldPushMode = FPush::kPushUnknown | FPush::kPushAllFields) const
	{
		return Accessor::template DoToJson<false, const Accessor>
				(*this, fieldPushMode, output, allocator);
	}

	/**
	 * Serialize the structure to a JsonValue object.
	 * @note The object may be invalid after serialization
	 * @param output 			The output of result
	 * @param allocator	 		Allocator when adding members to output.
	 * @param fieldPushMode 	Serialization options about fields, options can be combined by bitwise OR operator (|)
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note If using inner allocator, user should carefully handler the structure's life cycle
	 */
	inline int MoveToJson(IJST_OUT JsonValue &output, JsonAllocator &allocator,
					  FPush::Mode fieldPushMode = FPush::kPushUnknown | FPush::kPushAllFields)
	{
		return Accessor::template DoToJson<true, Accessor>
				(*this, fieldPushMode, output, allocator);
	}

	/**
	 * Init allocator of members to self's allocator.
	 * @return 					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 */
	int InitMembersAllocator()
	{
		return SetMembersAllocator(*m_pAllocator);
	}

	/**
	 * Set Inner allocator of object and members.
	 * @param allocator			New allocator
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_TO_JSON_OBJECT to 1 to enable this method
	 * @note This method will NOT clear pervious allocator
	 */
	int SetMembersAllocator(JsonAllocator &allocator)
	{
		if (m_pAllocator != &allocator) {
			// copy buffer when need
			JsonValue temp;
			temp = *m_pUnknown;
			// The life cycle of const string in temp should be same as this object
			m_pUnknown->CopyFrom(temp, allocator, false);
		}

		m_pAllocator = &allocator;

		// Set allocator in members
		for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
			 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
		{
			void *pField = GetFieldByOffset(itMetaField->offset);
			IJSTI_RET_WHEN_NOT_ZERO(
					itMetaField->serializerInterface->SetAllocator(pField, allocator)
			);
		}
		return 0;
	}
#endif

#if IJST_ENABLE_FROM_JSON_OBJECT
	/**
	 * Deserialize from json object.
	 * @param stream			Input json object
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 */
	inline int FromJson(const JsonValue &stream, EUnknownMode unknownMode = UnknownMode::kKeep,
						std::string *pErrMsgOut = IJST_NULL)
	{
		return DoFromJsonWrap<const JsonValue>(&Accessor::DoFromJson, stream, unknownMode, pErrMsgOut);
	}

	/**
	 * Deserialize from json document. The source object may be stolen after deserialize.
	 * Because the accessor need manager the input allocator, but the Allocator class has no Swap() interface,
	 * so use document object instead.
	 * @param srcDocStolen		Input document object
	 * @param unknownMode		Behaviour when meet unknown member in json
	 * @param pErrMsgOut		Error message output. Null if do not need error message
	 * @return					Error code
	 *
	 * @note Need to set IJST_ENABLE_FROM_JSON_OBJECT to 1 to enable this method
	 * @note The source document may be changed after deserialize
	 * @note Make sure srcDocStolen use own allocator, or use allocator in this object
	 */
	inline int MoveFromJson(rapidjson::Document &srcDocStolen, EUnknownMode unknownMode = UnknownMode::kKeep,
							std::string *pErrMsgOut = IJST_NULL)
	{
		// Store document to manager allocator
		m_pOwnDoc->Swap(srcDocStolen);
		m_pAllocator = &m_pOwnDoc->GetAllocator();
		return DoFromJsonWrap<JsonValue>(&Accessor::DoMoveFromJson, *m_pOwnDoc, unknownMode, pErrMsgOut);
	}
#endif

private:
	// #region Implement SerializeInterface
	template <class _T> friend class detail::FSerializer;
	typedef detail::SerializerInterface::SerializeReq SerializeReq;
	inline int ISerialize(const SerializeReq &req) const
	{
		assert(req.pField == this);
		return DoSerialize(req.writer, req.fPushMode);
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	typedef detail::SerializerInterface::ToJsonReq ToJsonReq;

	inline int IToJson(const ToJsonReq &req)
	{
		assert(req.pField == this);
		if (req.canMoveSrc) {
			return Accessor::template DoToJson<true, Accessor>
					(*this, req.fPushMode, req.buffer, req.allocator);
		}
		else {
			return Accessor::template DoToJson<false, const Accessor>
					(*this, req.fPushMode, req.buffer, req.allocator);
		}
	}

	inline int ISetAllocator(void* pField, JsonAllocator& allocator)
	{
		(void) pField;
		assert(pField == this);
		return SetMembersAllocator(allocator);
	}
#endif

	typedef detail::SerializerInterface::FromJsonReq FromJsonReq;
	typedef detail::SerializerInterface::FromJsonResp FromJsonResp;

	inline int IFromJson(const FromJsonReq &req, IJST_OUT FromJsonResp& resp)
	{
		assert(req.pFieldBuffer == this);

		m_pAllocator = &req.allocator;

		if (req.canMoveSrc) {
			return DoMoveFromJson(req.stream, req.unknownMode, resp.errDoc, resp.fieldCount);
		}
		else {
			return DoFromJson(req.stream, req.unknownMode, resp.errDoc, resp.fieldCount);
		}
	}

	// #endregion

	//! Serialize to string using SAX API
	int DoSerialize(detail::JsonWriter &writer, FPush::Mode fPushMode) const
	{
		if (m_isParentVal) {
			return DoSerializeFields(writer, fPushMode);
			// Unknown will be ignored
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.StartObject());

		// Write fields
		IJSTI_RET_WHEN_NOT_ZERO(DoSerializeFields(writer, fPushMode));

		// Write buffer if need
		if (isBitSet(fPushMode, FPush::kPushUnknown))
		{
			assert(m_pUnknown->IsObject());
			for (rapidjson::Value::ConstMemberIterator itMember = m_pUnknown->MemberBegin();
				 itMember != m_pUnknown->MemberEnd(); ++itMember)
			{
				// Write key
				const JsonValue& key = itMember->name;
				IJSTI_RET_WHEN_WRITE_FAILD(
						writer.Key(key.GetString(), key.GetStringLength()) );
				// Write value
				IJSTI_RET_WHEN_WRITE_FAILD(
						itMember->value.Accept(writer) );
			}
		}

		IJSTI_RET_WHEN_WRITE_FAILD(writer.EndObject());
		return 0;
	}

	int DoSerializeFields(detail::JsonWriter &writer, FPush::Mode fPushMode) const
	{
		IJST_ASSERT(!m_isParentVal || m_pMetaClass->metaFields.size() == 1);
		for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
			 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
		{
			// Check field state
			const EFStatus fstatus = (*m_pFieldStatus)[itMetaField->index];
			switch (fstatus) {
				case FStatus::kValid:
				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (fstatus != FStatus::kValid && !isBitSet(fPushMode, FPush::kPushAllFields)) {
						continue;
					}

					const void *pFieldValue = GetFieldByOffset(itMetaField->offset);
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->name.c_str(), itMetaField->name.length()) );
					}
					// write value
					SerializeReq req(writer, pFieldValue, fPushMode);
					IJSTI_RET_WHEN_NOT_ZERO(
							itMetaField->serializerInterface->Serialize(req));
				}
					break;

				case FStatus::kNull:
				{
					if (!m_isParentVal) {
						// write key
						IJSTI_RET_WHEN_WRITE_FAILD(
								writer.Key(itMetaField->name.c_str(), itMetaField->name.length()) );
					}
					// write value
					IJSTI_RET_WHEN_WRITE_FAILD(writer.Null());
				}
					break;

				case FStatus::kNotAField:
				default:
					// Error occurs
					assert(false);
					return Err::kInnerError;
			}
		}
		return 0;
	}

#if IJST_ENABLE_TO_JSON_OBJECT
	int DoAllFieldsToJson(FPush::Mode fPushMode, bool canMoveSrc,
						  IJST_OUT JsonValue &buffer, JsonAllocator &allocator) const
	{
		// Serialize fields to buffer

		if (!m_isParentVal)
		{
			// Init buffer to object and reserve space
			buffer.SetObject();

			do {
				const size_t unknwonSize = isBitSet(fPushMode, FPush::kPushUnknown) ? m_pUnknown->MemberCount() : 0;
				const size_t maxSize = m_pMetaClass->metaFields.size() + unknwonSize;
				if (buffer.MemberCapacity() >= maxSize) {
					break;
				}

				size_t fieldSize;
				if (isBitSet(fPushMode, FPush::kPushAllFields)) {
					fieldSize = maxSize;
				}
				else {
					fieldSize = unknwonSize;
					for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
						 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
					{
						const EFStatus fstatus = (*m_pFieldStatus)[itMetaField->index];
						if (fstatus == FStatus::kValid || fstatus == FStatus::kNull) {
							++fieldSize;
						}
					}

				}
				buffer.MemberReserve(fieldSize, allocator);
			} while (false);
		}

		#ifndef NDEBUG
		// assert that buffer will not reallocate memory during serialization
		const rapidjson::SizeType oldCapcity = m_isParentVal ? 0 : buffer.MemberCapacity();
		#endif

		for (std::vector<detail::MetaField>::const_iterator itMetaField = m_pMetaClass->metaFields.begin();
			 itMetaField != m_pMetaClass->metaFields.end(); ++itMetaField)
		{
			// Check field state
			const EFStatus fstatus = (*m_pFieldStatus)[itMetaField->index];
			switch (fstatus) {
				case FStatus::kValid:
				case FStatus::kMissing:
				case FStatus::kParseFailed:
				{
					if (fstatus != FStatus::kValid && !isBitSet(fPushMode, FPush::kPushAllFields)) {
						continue;
					}
					IJSTI_RET_WHEN_NOT_ZERO(
							DoFieldToJson(itMetaField, canMoveSrc, fPushMode, buffer, allocator) );
				}
					break;

				case FStatus::kNull:
				{
					IJSTI_RET_WHEN_NOT_ZERO(
							DoNullFieldToJson(itMetaField, buffer, allocator) );
				}
					break;


				case FStatus::kNotAField:
				default:
					// Error occurs
					assert(false);
					return Err::kInnerError;
			}

		}

		assert(m_isParentVal || buffer.MemberCapacity() == oldCapcity);
		// Unknown fields will be add in AppendUnknownToBuffer
		return 0;
	}

	template <bool kCanMoveSrc, class _TAccessor>
	static int DoToJson(_TAccessor &accessor, FPush::Mode fPushMode, IJST_OUT JsonValue &buffer,
						JsonAllocator &allocator)
	{
		const Accessor& rThis = accessor;
		IJSTI_RET_WHEN_NOT_ZERO(
				rThis.DoAllFieldsToJson(fPushMode, kCanMoveSrc, buffer, allocator) );
		if (isBitSet(fPushMode, FPush::kPushUnknown)) {
			Accessor::template AppendUnknownToBuffer<kCanMoveSrc, _TAccessor>(accessor, buffer, allocator);
		}
		return 0;
	}

	int DoFieldToJson(std::vector<detail::MetaField>::const_iterator itMetaField,
					  bool canMoveSrc, unsigned fPushMode,
					  JsonValue &buffer, JsonAllocator &allocator) const
	{
		// Init
		const void *pFieldValue = GetFieldByOffset(itMetaField->offset);

		if (m_isParentVal) {
			// Set buffer itself to json value
			ToJsonReq elemSerializeReq(buffer, allocator, pFieldValue, canMoveSrc, fPushMode);
			return itMetaField->serializerInterface->ToJson(elemSerializeReq);
		}
		// Add a member to buffer

		// Serialize field
		JsonValue elemOutput;
		ToJsonReq elemSerializeReq(elemOutput, allocator, pFieldValue, canMoveSrc, fPushMode);
		IJSTI_RET_WHEN_NOT_ZERO(
				itMetaField->serializerInterface->ToJson(elemSerializeReq));

		// Add member, copy field name because the fieldName store in Meta info maybe release when dynamical
		// library unload, and the memory pool should be fast to copy field name.
		// Do not check existing key, that's a feature
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		buffer.AddMember(
				rapidjson::Value().SetString(fieldNameRef, allocator),
				elemOutput,
				allocator
		);
		return 0;
	}

	int DoNullFieldToJson(std::vector<detail::MetaField>::const_iterator itMetaField,
						  JsonValue &buffer, JsonAllocator &allocator) const
	{
		if (m_isParentVal) {
			buffer.SetNull();
			return 0;
		}

		// Init
		rapidjson::GenericStringRef<char> fieldNameRef =
				rapidjson::StringRef(itMetaField->name.c_str(), itMetaField->name.length());
		rapidjson::Value fieldNameVal;
		fieldNameVal.SetString(fieldNameRef);

		// Add member, field name is not need deep copy
		// Do not check if there is a existing key, that's a feature
		buffer.AddMember(
				rapidjson::Value().SetString(fieldNameRef),
				rapidjson::Value().SetNull(),
				allocator
		);
		return 0;
	}

	template<bool kCanMoveSrc, typename _TAccessor>
	static inline void AppendUnknownToBuffer(_TAccessor &accessor, JsonValue &buffer, JsonAllocator &allocator);
#endif // #if IJST_ENABLE_TO_JSON_OBJECT

	template<typename TJsonValue, typename Func>
	int DoFromJsonWrap(Func func, TJsonValue &stream, EUnknownMode unknownMode, std::string* pErrMsgOut)
	{
		size_t fieldCount;
		JsonAllocator* allocator = pErrMsgOut == IJSTI_NULL ? IJSTI_NULL : new JsonAllocator();
		detail::MemoryGuarder<JsonAllocator> ptrGuarder(allocator);
		detail::DeserializeErrDoc errDoc(allocator);
		int ret = (this->*func)(stream, unknownMode, errDoc, fieldCount);
		if (ret != 0 && pErrMsgOut != IJSTI_NULL) {
			assert(allocator != IJSTI_NULL);
			rapidjson::StringBuffer sb;
			detail::JsonWriter writer(sb);
			errDoc.errMsg.Accept(writer);
			(*pErrMsgOut) = std::string(sb.GetString(), sb.GetLength());
		}
		return ret;
	}

	/**
	 * Deserialize move from json object
	 * @note Make sure the lifecycle of allocator of the stream is longer than this object
	 */
	int DoMoveFromJson(JsonValue &stream, EUnknownMode unknownMode,
					   detail::DeserializeErrDoc& errDoc, IJST_OUT size_t &fieldCountOut)
	{
		if (m_isParentVal) {
			// Set field by stream itself
			assert(m_pMetaClass->metaFields.size() == 1);
			return DoFieldFromJson(
					&m_pMetaClass->metaFields[0], stream, unknownMode, /*canMoveSrc=*/true, errDoc);
		}

		// Set fields by members of stream
		if (!stream.IsObject()) {
			errDoc.TypeMismatch("object", stream);
			return Err::kDeserializeValueTypeError;
		}

		fieldCountOut = 0;
		// For each member
		rapidjson::Value::MemberIterator itNextRemain = stream.MemberBegin();
		for (rapidjson::Value::MemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{

			// Get related field info
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			const detail::MetaField *pMetaField = m_pMetaClass->FindByName(fieldName);

			if (pMetaField == IJST_NULL) {
				// Not a field in struct
				switch (unknownMode) {
					case UnknownMode::kKeep:
						// Move unknown fields to the front of array first
						// TODO: This is relay on the implementation details of rapidjson's object storage (array), how to check?
						if (itNextRemain != itMember) {
							itNextRemain->name.SetNull().Swap(itMember->name);
							itNextRemain->value.SetNull().Swap(itMember->value);
						}
						++itNextRemain;
						break;
					case UnknownMode::kError:
						errDoc.ErrorInObject("UnknownMember", fieldName);
						return Err::kDeserializeSomeUnknownMember;
					case UnknownMode::kIgnore:
						break;
					default:
						assert(false);
				}
				continue;
			}

			// Move member out of object
			JsonValue memberStream(rapidjson::kNullType);
			memberStream.Swap(itMember->value);

			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, unknownMode, /*canMoveSrc=*/true, errDoc) );
			++fieldCountOut;
		}

		// Clean deserialized
		if (stream.MemberCount() != 0) {
			stream.EraseMember(itNextRemain, stream.MemberEnd());
		}
		if (unknownMode == UnknownMode::kKeep) {
			m_pUnknown->SetNull().Swap(stream);
		}
		else {
			m_pUnknown->SetObject();
		}

		int ret = CheckFieldState(errDoc);
		return ret;
	}

	//! Deserialize from stream
	int DoFromJson(const JsonValue &stream, EUnknownMode unknownMode,
				   detail::DeserializeErrDoc& errDoc, IJST_OUT size_t &fieldCountOut)
	{
		if (m_isParentVal) {
			// Serialize field by stream itself
			assert(m_pMetaClass->metaFields.size() == 1);
			return DoFieldFromJson(
					&m_pMetaClass->metaFields[0], const_cast<JsonValue &>(stream), unknownMode, /*canMoveSrc=*/true, errDoc);
		}

		// Serialize fields by members of stream
		if (!stream.IsObject()) {
			errDoc.TypeMismatch("object", stream);
			return Err::kDeserializeValueTypeError;
		}

		m_pUnknown->SetObject();
		fieldCountOut = 0;
		// For each member
		for (rapidjson::Value::ConstMemberIterator itMember = stream.MemberBegin();
			 itMember != stream.MemberEnd(); ++itMember)
		{
			// Get related field info
			const std::string fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			const detail::MetaField *pMetaField = m_pMetaClass->FindByName(fieldName);

			if (pMetaField == IJST_NULL)
			{
				// Not a field in struct
				switch (unknownMode) {
					case UnknownMode::kKeep:
						m_pUnknown->AddMember(
								rapidjson::Value().SetString(fieldName.c_str(), fieldName.length(), *m_pAllocator),
								rapidjson::Value().CopyFrom(itMember->value, *m_pAllocator, true),
								*m_pAllocator
						);
						break;
					case UnknownMode::kIgnore:
						break;
					case UnknownMode::kError:
						errDoc.ErrorInObject("UnknownMember", fieldName);
						return Err::kDeserializeSomeUnknownMember;
					default:
						assert(false);
				}
				continue;
			}

			JsonValue& memberStream = const_cast<JsonValue&>(itMember->value);
			IJSTI_RET_WHEN_NOT_ZERO(
					DoFieldFromJson(pMetaField, memberStream, unknownMode, /*canMoveSrc=*/false, errDoc) );
			++fieldCountOut;
		}

		int ret = CheckFieldState(errDoc);
		return ret;
	}


	int DoFieldFromJson(const detail::MetaField *metaField,
						JsonValue &stream, EUnknownMode unknownMode, bool canMoveSrc, detail::DeserializeErrDoc &errDoc)
	{
		// Check nullable
		if (isBitSet(metaField->desc, FDesc::Nullable)
			&& stream.IsNull())
		{
			(*m_pFieldStatus)[metaField->index] = FStatus::kNull;
		}
		else
		{
			void *pField = GetFieldByOffset(metaField->offset);
			FromJsonReq elemReq(stream, *m_pAllocator, unknownMode, canMoveSrc, pField);
			FromJsonResp elemResp(errDoc.pAllocator);
			int ret = metaField->serializerInterface->FromJson(elemReq, elemResp);
			// Check return
			if (ret != 0)
			{
				(*m_pFieldStatus)[metaField->index] = FStatus::kParseFailed;
				errDoc.ErrorInObject("ErrInObject", metaField->name, &elemResp.errDoc);
				return ret;
			}
			// Check elem size
			if (isBitSet(metaField->desc, FDesc::ElemNotEmpty)
				&& elemResp.fieldCount == 0)
			{
				errDoc.ErrorInObject("ElemIsEmpty", metaField->name);
				return Err::kDeserializeElemEmpty;
			}
			// succ
			(*m_pFieldStatus)[metaField->index] = FStatus::kValid;
		}
		return 0;
	}

	inline void InitOuterPtr()
	{
		m_pOuter = reinterpret_cast<const unsigned char *>(this - m_pMetaClass->accessorOffset);
	}

	void MarkFieldStatus(const void* field, EFStatus fStatus)
	{
		const std::size_t offset = GetFieldOffset(field);
		const int index = m_pMetaClass->FindIndex(offset);
		IJST_ASSERT(index >= 0 && (unsigned int)index < m_pFieldStatus->size());
		(*m_pFieldStatus)[index] = fStatus;
	}

	int CheckFieldState(detail::DeserializeErrDoc& errDoc) const
	{
		// Check all required field status
		std::stringstream invalidNameOss;
		bool hasErr = false;

		detail::DeserializeErrDoc missingFields(errDoc.pAllocator);
		missingFields.errMsg.SetArray();

		for (std::vector<detail::MetaField>::const_iterator itField = m_pMetaClass->metaFields.begin();
			 itField != m_pMetaClass->metaFields.end(); ++itField)
		{
			if (isBitSet(itField->desc, FDesc::Optional))
			{
				// Optional
				continue;
			}

			const EFStatus fStatus = (*m_pFieldStatus)[itField->index];
			if (fStatus == FStatus::kValid
				|| fStatus == FStatus::kNull)
			{
				// Correct
				continue;
			}

			// Has error
			hasErr = true;
			missingFields.PushMemberName(itField->name);
		}
		if (hasErr)
		{
			errDoc.MissingMember(missingFields);
			return Err::kDeserializeSomeFiledsInvalid;
		}
		return 0;
	}

	inline std::size_t GetFieldOffset(const void *ptr) const
	{
		const unsigned char *filed_ptr = static_cast<const unsigned char *>(ptr);
		return filed_ptr - m_pOuter;
	}

	inline void *GetFieldByOffset(std::size_t offset) const
	{
		return (void *) (m_pOuter + offset);
	}

	static inline bool isBitSet(unsigned int val, unsigned int bit)
	{
		return (val & bit) != 0;
	}

	// Note: Use pointers to make class Accessor be a standard-layout type struct
	typedef std::vector<EFStatus> FieldStatusType;
	FieldStatusType* m_pFieldStatus;
	const detail::MetaClass* m_pMetaClass;

	rapidjson::Value* m_pUnknown;
	// Should use document instead of Allocator because document can swap allocator
	rapidjson::Document* m_pOwnDoc;

	JsonAllocator* m_pAllocator;
	const unsigned char *m_pOuter;

	bool m_isValid;
	bool m_isParentVal;
	//</editor-fold>
};

#if IJST_ENABLE_TO_JSON_OBJECT
//! copy version of Accessor::AppendInnerToBuffer
template <>
inline void Accessor::template AppendUnknownToBuffer<false, const Accessor>(
		const Accessor &accessor, JsonValue &buffer, JsonAllocator &allocator)
{
	const Accessor& rThis = accessor;
	// Copy
	for (rapidjson::Value::ConstMemberIterator itMember = rThis.m_pUnknown->MemberBegin();
		 itMember != rThis.m_pUnknown->MemberEnd(); ++itMember)
	{
		rapidjson::Value name;
		rapidjson::Value val;
		name.CopyFrom(itMember->name, allocator, true);
		val.CopyFrom(itMember->value, allocator, true);
		buffer.AddMember(name, val, allocator);
	}
}

//! move version of Accessor::AppendInnerToBuffer
template <>
inline void Accessor::template AppendUnknownToBuffer<true, Accessor>(
		Accessor& accessor, JsonValue&buffer, JsonAllocator& allocator)
{
	Accessor& rThis = accessor;
	// append inner data to buffer
	if (&allocator == rThis.m_pAllocator) {
		// Move
		for (rapidjson::Value::MemberIterator itMember = rThis.m_pUnknown->MemberBegin();
			 itMember != rThis.m_pUnknown->MemberEnd(); ++itMember)
		{
			buffer.AddMember(itMember->name, itMember->value, allocator);
			// both itMember->name, itMember->value are null now
		}
	}
	else
	{
		// Copy
		Accessor::template AppendUnknownToBuffer<false, const Accessor>(accessor, buffer, allocator);
		// The object will be release after serialize in most suitation, so do not need clear own allocator
	}

	rThis.m_pUnknown->SetObject();
}
#endif

//! IJSTI_DEFINE_STRUCT_IMPL
//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
//! @param N			fields size
//! @param isRawVal		is struct a raw value: true/false
//! @param needGetter	need Get* function: T/F
//! @param stName		struct name
//! @param ...			fields define: [(fType, fName, sName, desc)]*
#ifdef _MSC_VER
	//! @params	N, isRawVal, needGetter, stName, ...
	#define IJSTI_DEFINE_STRUCT_IMPL(N, ...) \
		IJSTI_PP_CONCAT(IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__), )
#else
	//! Wrapper of IJST_DEFINE_STRUCT_IMPL_*
	//! @param needGetter: must be T or F
	#define IJSTI_DEFINE_STRUCT_IMPL(N, isRawVal, needGetter, ...) \
		IJSTI_PP_CONCAT(IJSTI_DEFINE_STRUCT_IMPL_, N)(isRawVal, needGetter, __VA_ARGS__)
#endif

//! IJSTI_DEFINE_GETTER_T
//! Define getter of fields
#ifdef _MSC_VER
	#define IJSTI_DEFINE_GETTER_T(N, ...)	\
		IJSTI_PP_CONCAT(IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N)(__VA_ARGS__), )
#else
	#define IJSTI_DEFINE_GETTER_T(N, ...)	\
		IJSTI_PP_CONCAT(IJSTI_DEFINE_GETTER_IMPL_, N)(__VA_ARGS__)
#endif
	#define IJSTI_DEFINE_GETTER_F(N, ...)	// empty

	#define IJSTI_IDL_FTYPE(fType, fName, sName, desc)		fType
	#define IJSTI_IDL_FNAME(fType, fName, sName, desc)		fName
	#define IJSTI_IDL_SNAME(fType, fName, sName, desc)		sName
	#define IJSTI_IDL_DESC(fType, fName, sName, desc)		desc

	#define IJSTI_DEFINE_FIELD(fType, fName, ... )												\
			::ijst::detail::FSerializer< fType>::VarType fName;

	#define IJSTI_FIELD_TYPEDEF_START()															\
			struct _TypeDef {

	#define IJSTI_FIELD_TYPEDEF(fType, fName, ...) 												\
				typedef ::ijst::detail::FSerializer< fType>::VarType fName;

	#define IJSTI_FIELD_TYPEDEF_END()															\
			};

	#define IJSTI_FIELD_GETTER(fType, fName, ... )												\
			::ijst::Optional<const _TypeDef::fName> IJSTI_PP_CONCAT(Get, fName)() const 		\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<const _TypeDef::fName>(IJST_NULL); }				\
				return ::ijst::Optional<const _TypeDef::fName>(&fName);							\
			}																					\
			::ijst::Optional<_TypeDef::fName> IJSTI_PP_CONCAT(Get, fName)()						\
			{																					\
				if (!this->_.IsValid() || this->_.GetStatus(&fName) != ijst::FStatus::kValid)	\
					{ return ::ijst::Optional<_TypeDef::fName>(IJST_NULL); }					\
				return ::ijst::Optional<_TypeDef::fName>(&fName);								\
			}

	#define IJSTI_METAINFO_DEFINE_START(stName, N)												\
			typedef ::ijst::detail::MetaInfo<stName> MetaInfoT;									\
			typedef ::ijst::detail::Singleton<MetaInfoT> MetaInfoS;								\
			friend MetaInfoT;																	\
			static void InitMetaInfo(MetaInfoT* metaInfo)										\
			{																					\
				IJSTI_TRY_INIT_META_BEFORE_MAIN(MetaInfoT);										\
				/*Do not call MetaInfoS::GetInstance() int this function */			 			\
				metaInfo->metaClass.InitBegin(#stName, N, IJST_OFFSETOF(stName, _));			\

	#define IJSTI_METAINFO_ADD(stName, fDef)  								\
			metaInfo->metaClass.PushMetaField(								\
				IJSTI_IDL_SNAME fDef, 										\
				IJST_OFFSETOF(stName, IJSTI_IDL_FNAME fDef),				\
				IJSTI_IDL_DESC fDef, 										\
				IJSTI_FSERIALIZER_INS(IJSTI_IDL_FTYPE fDef)					\
			);

	#define IJSTI_METAINFO_DEFINE_END()															\
				metaInfo->metaClass.InitEnd();													\
			}

}	// namespace ijst

#include "detail/ijst_repeat_def.inc"

#endif //_IJST_HPP_INCLUDE_