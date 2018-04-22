/**************************************************************************************************
 *		Serialization implementation of Container types
 *		Created by h46incon on 2017/12/26.
 **************************************************************************************************/

#ifndef IJST_TYPES_CONTAINER_HPP_INCLUDE_
#define IJST_TYPES_CONTAINER_HPP_INCLUDE_

#include "ijst.h"
#include <map>
#include <list>
#include <deque>

//! @brief Declare a vector<T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TVEC(T)	::std::vector< T >
//! @brief Declare a deque<T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TDEQUE(T)	::std::deque< T >
//! @brief Declare a vector<T> field.
//! @ingroup IJST_MACRO_API
#define IJST_TLIST(T)	::std::list< T >
//! @brief Declare a map<string, T> field of json object
//! @ingroup IJST_MACRO_API
#define IJST_TMAP(T)	IJST_TYPE(::std::map< ::std::basic_string<_ijst_Ch>, T >)
//! @brief Declare a vector of members of json object
//! @ingroup IJST_MACRO_API
#define IJST_TOBJ(T)	IJST_TYPE(::std::vector< ::ijst::T_Member< T, _ijst_Ch> >)
//! @brief Declare a object field which T is a ijst struct type.
//! @ingroup IJST_MACRO_API
#define IJST_TST(T)		T

namespace ijst {

/**
 * @brief Memeber in json object
 *
 * @tparam T		value type
 * @tparam CharType	character type of string
 */
template<typename T, typename CharType = char>
struct T_Member {
	typedef T ValType;
	typedef CharType Ch;
	std::basic_string<Ch> name;
	T value;

	T_Member(): name(), value() {}
	T_Member(const std::basic_string<Ch>& _name, const T& _value): name(_name), value(_value) {}
#if __cplusplus >= 201103L
	T_Member(std::basic_string<Ch>&& _name, T&& _value): name(_name), value(_value) {}
#endif
};

/**
 * Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam TElem		map value type
 * @tparam CharType		character type of map key
 */
template <typename TElem, typename CharType>
class Optional <std::map<std::basic_string<CharType>, TElem> > {
	typedef std::map<std::basic_string<CharType>, TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(elemInstance) if key is found, Optional(null) else
	 */
	Optional<TElem> operator[](const std::basic_string<CharType>& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<TElem>(IJST_NULL);
		}
		typename ValType::iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<TElem>(IJST_NULL);
		}
		else {
			return Optional<TElem>(&it->second);
		}
	}
};

/**
 * const version Specialization for map type of Optional template.
 * This specialization add operator[] (string key) for getter chaining.
 *
 * @tparam TElem		map value type
 * @tparam CharType		character type of map key
 */
template <typename TElem, typename CharType>
class Optional <const std::map<std::basic_string<CharType>, TElem> >
{
	typedef const std::map<std::basic_string<CharType>, TElem> ValType;
	IJSTI_OPTIONAL_BASE_DEFINE(ValType)
public:
	/**
	 * Get element by key
	 *
	 * @param key 	key
	 * @return 		Optional(const elemInstance) if key is found, Optional(null) else
	 */
	Optional<const TElem> operator[](const std::basic_string<CharType>& key) const
	{
		if (m_pVal == IJST_NULL) {
			return Optional<const TElem>(IJST_NULL);
		}
		typename ValType::const_iterator it = m_pVal->find(key);
		if (it == m_pVal->end()){
			return Optional<const TElem>(IJST_NULL);
		}
		else {
			return Optional<const TElem>(&it->second);
		}
	}
};

/**
 * Specialization for vector or deque type of Optional template.
 * This specialization add operator[] (size_type i) for getter chaining.
 *
 * @tparam TElem	Element type
 */
#define IJSTI_OPTIONAL_ARRAY_DEFINE(is_const, Container)													\
	template<typename TElem>																				\
	class Optional<is_const Container<TElem> >																\
	{ 																										\
		typedef is_const Container<TElem> ValType;															\
		IJSTI_OPTIONAL_BASE_DEFINE(ValType)																	\
	public:																									\
		/** return Optional(elemeInstance) if i is valid, Optional(null) else. */							\
		Optional<is_const TElem> operator[](typename Container<TElem>::size_type i) const					\
		{																									\
			if (m_pVal == IJST_NULL || m_pVal->size() <= i) {												\
				return Optional<is_const TElem>(IJST_NULL);												\
			}																								\
			return Optional<is_const TElem>(&(*m_pVal)[i]);												\
		}																									\
	};

IJSTI_OPTIONAL_ARRAY_DEFINE(, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::vector)
IJSTI_OPTIONAL_ARRAY_DEFINE(, std::deque)
IJSTI_OPTIONAL_ARRAY_DEFINE(const, std::deque)

}	// namespace ijst


namespace ijst {
namespace detail {

template<typename ElemType, typename VarType, typename Encoding>
class ContainerSerializer : public SerializerInterface<Encoding> {
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType*>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartArray());

		for (typename VarType::const_iterator itera = field.begin(); itera != field.end(); ++itera) {
			SerializeReq elemReq(req.writer, &(*itera), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndArray(static_cast<rapidjson::SizeType>(field.size())) );

		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsArray()), "array");

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// Alloc buffer
		field.resize(req.stream.Size());
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);

		size_t i = 0;
		typename VarType::iterator itField = field.begin();
		rapidjson::Value::ValueIterator  itVal = req.stream.Begin();
		for (; itVal != req.stream.End(); ++itVal, ++itField, ++i)
		{
			assert(i < field.size());
			assert(itField != field.end());
			FromJsonReq elemReq(*itVal, req.allocator,
								req.deserFlag, req.canMoveSrc, &*itField, 0);
			FromJsonResp elemResp(resp.errDoc);
			// FromJson
			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.resize(i);
				resp.errDoc.ErrorInArray(static_cast<rapidjson::SizeType>(field.size()));
				return ret;
			}
		}
		assert(i == field.size());

		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ElemType, Encoding);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			intf.ShrinkAllocator(&*itField);
		}
	}
};

#define IJSTI_SERIALIZER_CONTAINER_DEFINE()																		\
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);																	\
	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE												\
	{ return ContainerSerializerSingleton::GetInstance().Serialize(req); }										\
	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE					\
	{ return ContainerSerializerSingleton::GetInstance().FromJson(req, resp); }									\
	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE													\
	{ return ContainerSerializerSingleton::GetInstance().ShrinkAllocator(pField); }

/**
 * Serialization class of Vector types
 * @tparam T class
 */
template<class T, typename Encoding>
class FSerializer<std::vector<T>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::vector<T> VarType;
	typedef Singleton<ContainerSerializer<T, VarType, Encoding> > ContainerSerializerSingleton;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam T class
 */
template<class T, typename Encoding>
class FSerializer<std::deque<T>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::deque<T> VarType;
	typedef Singleton<ContainerSerializer<T, VarType, Encoding> > ContainerSerializerSingleton;

public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Deque types
 * @tparam T class
 */
template<class T, typename Encoding>
class FSerializer<std::list<T>, Encoding> : public SerializerInterface<Encoding> {
	typedef std::list<T> VarType;
	typedef Singleton<ContainerSerializer<T, VarType, Encoding> > ContainerSerializerSingleton;
public:
	IJSTI_SERIALIZER_CONTAINER_DEFINE()
};

/**
 * Serialization class of Map types
 * @tparam T class
 */
template<class T, typename Encoding>
class FSerializer<std::map<std::basic_string<typename Encoding::Ch>, T>, Encoding> : public SerializerInterface<Encoding> {
	typedef typename Encoding::Ch Ch;
	typedef std::map<std::basic_string<Ch>, T> VarType;
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itFieldMember = field.begin(); itFieldMember != field.end(); ++itFieldMember) {
			const std::basic_string<Ch>& key = itFieldMember->first;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itFieldMember->second), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsObject()), "object");

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);

		for (rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
			 itMember != req.stream.MemberEnd(); ++itMember)
		{
			const std::basic_string<Ch> fieldName(itMember->name.GetString(), itMember->name.GetStringLength());
			// New a elem buffer in container first to avoid copy
			typename VarType::value_type buf(fieldName, T());
			std::pair<typename VarType::iterator, bool> insertRet = field.insert(IJSTI_MOVE(buf));
			// Check duplicate
			if (!insertRet.second) {
				resp.errDoc.ElementMapKeyDuplicated(fieldName);
				return ErrorCode::kDeserializeMapKeyDuplicated;
			}

			// Element FromJson
			T &elemBuffer = insertRet.first->second;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer, 0);
			FromJsonResp elemResp(resp.errDoc);
			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				field.erase(fieldName);
				resp.errDoc.ErrorInMap(fieldName);
				return ret;
			}
		}
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType *>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(T, Encoding);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			intf.ShrinkAllocator(&itField->second);
		}
	}
};

/**
 * Serialization class of Object types
 */
template<class T, typename Encoding>
class FSerializer<std::vector<T_Member<T, typename Encoding::Ch> >, Encoding> : public SerializerInterface<Encoding> {
	typedef typename Encoding::Ch Ch;
	typedef T_Member<T, Ch> MemberType;
	typedef typename MemberType::ValType ValType;
	typedef std::vector<MemberType> VarType;
public:
	IJSTI_PROPAGATE_SINTERFACE_TYPE(Encoding);

	virtual int Serialize(const SerializeReq &req) IJSTI_OVERRIDE
	{
		assert(req.pField != IJST_NULL);
		const VarType& field = *static_cast<const VarType *>(req.pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);

		IJSTI_RET_WHEN_WRITE_FAILD(req.writer.StartObject());

		for (typename VarType::const_iterator itMember = field.begin(); itMember != field.end(); ++itMember) {
			const std::basic_string<Ch>& key = itMember->name;
			IJSTI_RET_WHEN_WRITE_FAILD(
					req.writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.size())) );

			SerializeReq elemReq(req.writer, &(itMember->value), req.serFlag);
			IJSTI_RET_WHEN_NOT_ZERO(intf.Serialize(elemReq));
		}

		IJSTI_RET_WHEN_WRITE_FAILD(
				req.writer.EndObject(static_cast<rapidjson::SizeType>(field.size())) );
		return 0;
	}

	virtual int FromJson(const FromJsonReq &req, IJST_OUT FromJsonResp &resp) IJSTI_OVERRIDE
	{
		IJSTI_RET_WHEN_TYPE_MISMATCH((req.stream.IsObject()), "object");

		assert(req.pFieldBuffer != IJST_NULL);
		VarType& field = *static_cast<VarType *>(req.pFieldBuffer);
		field.clear();
		// pField->shrink_to_fit();
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);

		// Alloc buffer
		field.resize(req.stream.MemberCount());
		size_t i = 0;
		rapidjson::Value::MemberIterator itMember = req.stream.MemberBegin();
		for (; itMember != req.stream.MemberEnd(); ++itMember, ++i)
		{
			assert(i < field.size());
			MemberType& memberBuf = field[i];
			memberBuf.name = std::basic_string<Ch>(itMember->name.GetString(), itMember->name.GetStringLength());
			ValType &elemBuffer = memberBuf.value;
			FromJsonReq elemReq(itMember->value, req.allocator,
								req.deserFlag, req.canMoveSrc, &elemBuffer, 0);
			FromJsonResp elemResp(resp.errDoc);

			int ret = intf.FromJson(elemReq, elemResp);
			if (ret != 0)
			{
				resp.errDoc.ErrorInMap(memberBuf.name);
				field.resize(i);
				return ret;
			}
		}
		assert(i == field.size());
		IJSTI_RET_WHEN_VALUE_IS_DEFAULT((field.empty()));
		return 0;
	}

	virtual void ShrinkAllocator(void* pField) IJSTI_OVERRIDE
	{
		VarType& field = *static_cast<VarType*>(pField);
		SerializerInterface<Encoding>& intf = IJSTI_FSERIALIZER_INS(ValType, Encoding);
		for (typename VarType::iterator itField = field.begin(); itField != field.end(); ++itField)
		{
			intf.ShrinkAllocator(&itField->value);
		}
	}
};

}	// namespace detail
}	// namespace ijst

#endif //IJST_TYPES_CONTAINER_HPP_INCLUDE_
