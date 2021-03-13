/*
 * Copyright 2020 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "io_realm_internal_OsSet.h"

#include <realm/object-store/set.hpp>
#include <realm/object-store/shared_realm.hpp>

#include "java_accessor.hpp"
#include "java_object_accessor.hpp"
#include "java_exception_def.hpp"
#include "jni_util/java_exception_thrower.hpp"
#include "util.hpp"

using namespace realm;
using namespace realm::util;
using namespace realm::object_store;
using namespace realm::_impl;

void finalize_set(jlong ptr) {
    delete reinterpret_cast<object_store::Set*>(ptr);
}

JNIEXPORT jlong JNICALL Java_io_realm_internal_OsSet_nativeGetFinalizerPtr(JNIEnv*, jclass) {
    return reinterpret_cast<jlong>(&finalize_set);
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_OsSet_nativeCreate(JNIEnv* env, jclass, jlong shared_realm_ptr,
                                          jlong obj_ptr, jlong column_key) {
    try {
        auto obj = *reinterpret_cast<realm::Obj*>(obj_ptr);
        auto shared_realm = *reinterpret_cast<SharedRealm*>(shared_realm_ptr);
        auto set_ptr = new object_store::Set(shared_realm, obj, ColKey(column_key));
        return reinterpret_cast<jlong>(set_ptr);
    }
    CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_OsSet_nativeIsValid(JNIEnv* env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        return set.is_valid();
    }
    CATCH_STD()
    return false;
}

JNIEXPORT jobject JNICALL
Java_io_realm_internal_OsSet_nativeGetValueAtIndex(JNIEnv* env, jclass, jlong set_ptr, jint position) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        const Mixed& value = set.get_any(position);
        if (value.is_null()) {
            return nullptr;
        } else {
            const DataType& type = value.get_type();
            switch (type) {
                case DataType::Type::Int:
                    return JavaClassGlobalDef::new_long(env, value.get_int());
                case DataType::Type::Double:
                    return JavaClassGlobalDef::new_double(env, value.get_double());
                case DataType::Type::Bool:
                    return JavaClassGlobalDef::new_boolean(env, value.get_bool());
                case DataType::Type::String:
                    return to_jstring(env, value.get_string());
                case DataType::Type::Binary:
                    return JavaClassGlobalDef::new_byte_array(env, value.get_binary());
                case DataType::Type::Float:
                    return JavaClassGlobalDef::new_float(env, value.get_float());
                case DataType::Type::UUID:
                    return JavaClassGlobalDef::new_uuid(env, value.get_uuid());
                case DataType::Type::ObjectId:
                    return JavaClassGlobalDef::new_object_id(env, value.get_object_id());
                case DataType::Type::Timestamp:
                    return JavaClassGlobalDef::new_date(env, value.get_timestamp());
                case DataType::Type::Decimal:
                    return JavaClassGlobalDef::new_decimal128(env, value.get_decimal());
                default:
                    throw std::logic_error("'getValue' method only suitable for int, double, boolean, String, byte[], float, UUID, Decimal128 and ObjectId.");
            }
        }
    }
    CATCH_STD()
    return nullptr;
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_OsSet_nativeSize(JNIEnv* env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        return set.size();
    }
    CATCH_STD()
    return 0;
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_OsSet_nativeContainsNull(JNIEnv *env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        size_t found = set.find_any(Mixed());
        return found != npos;       // npos represents "not found"
    }
    CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_OsSet_nativeContainsString(JNIEnv* env, jclass, jlong set_ptr,
                                                  jstring j_value) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        JStringAccessor value(env, j_value);
        size_t found = set.find_any(Mixed(StringData(value)));
        return found != npos;       // npos represents "not found"
    }
    CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}

JNIEXPORT void JNICALL
Java_io_realm_internal_OsSet_nativeAddNull(JNIEnv* env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        JavaAccessorContext context(env);
        set.insert(context, Any());
    }
    CATCH_STD()
}

JNIEXPORT void JNICALL
Java_io_realm_internal_OsSet_nativeAddString(JNIEnv* env, jclass, jlong set_ptr, jstring j_value) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        JStringAccessor value(env, j_value);
        JavaAccessorContext context(env);
        set.insert(context, Any(value));
    }
    CATCH_STD()
}

JNIEXPORT jlongArray JNICALL
Java_io_realm_internal_OsSet_nativeRemoveNull(JNIEnv* env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        const std::pair<size_t, bool>& remove_pair = set.remove_any(Mixed());
        jlong ret[2];
        ret[0] = remove_pair.first;     // index
        ret[1] = remove_pair.second;    // found (or not)
        jlongArray ret_array = env->NewLongArray(2);
        env->SetLongArrayRegion(ret_array, 0, 2, ret);
        return ret_array;
    }
    CATCH_STD()
    return nullptr;
}

JNIEXPORT jlongArray JNICALL
Java_io_realm_internal_OsSet_nativeRemoveString(JNIEnv* env, jclass, jlong set_ptr, jstring j_value) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        JStringAccessor value(env, j_value);
        const std::pair<size_t, bool>& remove_pair = set.remove_any(Mixed(StringData(value)));
        jlong ret[2];
        ret[0] = remove_pair.first;     // index
        ret[1] = remove_pair.second;    // found (or not)
        jlongArray ret_array = env->NewLongArray(2);
        env->SetLongArrayRegion(ret_array, 0, 2, ret);
        return ret_array;
    }
    CATCH_STD()
    return nullptr;
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_OsSet_nativeIsSubSetOf(JNIEnv* env, jclass, jlong set_ptr,
                                              jlong other_set_ptr) {
    try {
        auto &other_set = *reinterpret_cast<realm::object_store::Set *>(other_set_ptr);
        auto &set = *reinterpret_cast<realm::object_store::Set *>(set_ptr);
        return other_set.is_subset_of(set);
    }
    CATCH_STD()
    return false;
}

JNIEXPORT jboolean JNICALL
Java_io_realm_internal_OsSet_nativeContainsAllString(JNIEnv* env, jclass, jlong set_ptr,
                                                     jobjectArray j_other_set_values) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        JObjectArrayAccessor<JStringAccessor, jstring> values(env, j_other_set_values);
        jsize values_size = values.size();
        for (int i = 0; i < values_size; i++) {
            JStringAccessor value = values[i];
            size_t found;
            if (value.is_null()) {
                found = set.find_any(Mixed());
            } else {
                found = set.find_any(Mixed(StringData(value)));
            }
            if (found == npos) {    // npos represents "not found"
                return false;
            }
        }
        return true;
    }
    CATCH_STD()
    return false;
}

JNIEXPORT void JNICALL
Java_io_realm_internal_OsSet_nativeClear(JNIEnv* env, jclass, jlong set_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        set.remove_all();
    }
    CATCH_STD()
}

JNIEXPORT jlong JNICALL
Java_io_realm_internal_OsSet_nativeFreeze(JNIEnv* env, jclass, jlong set_ptr,
                                          jlong frozen_realm_ptr) {
    try {
        auto& set = *reinterpret_cast<realm::object_store::Set*>(set_ptr);
        auto& shared_realm_ptr = *reinterpret_cast<std::shared_ptr<Realm>*>(frozen_realm_ptr);
        const object_store::Set& frozen_set = set.freeze(shared_realm_ptr);
        auto* frozen_set_ptr = new object_store::Set(frozen_set);
        return reinterpret_cast<jlong>(frozen_set_ptr);
    }
    CATCH_STD()
    return reinterpret_cast<jlong>(nullptr);
}