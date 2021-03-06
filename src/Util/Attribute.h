/*
 *
 * Copyright (c) 2018-2019 Doo Yong Kim. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_UTIL_ATTRIBUTE_H_
#define SRC_UTIL_ATTRIBUTE_H_

#include <algorithm>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <functional>
#include <memory>
#include <string>

namespace lucene {
namespace core {
namespace util {

class Attribute {
 public:
  virtual ~Attribute() {}

  template<typename T>
  static std::type_index TypeId() {
    return std::type_index(typeid(T));
  }
};

using AttributeReflector =
  std::function<void(
                     // Attribute class name
                     const std::string&,
                     // Key
                     const std::string&,
                     // Value
                     const std::string&)>;

class AttributeImpl: public Attribute {
 public:
  virtual ~AttributeImpl() { }
  virtual void ReflectWith(AttributeReflector& reflector) = 0;
  virtual void Clear() = 0;
  virtual void End();
  virtual std::vector<std::type_index> Attributes() = 0;
  virtual void ShallowCopyTo(AttributeImpl& attr_impl) = 0;
  virtual AttributeImpl* Clone() = 0;
  virtual AttributeImpl& operator=(const AttributeImpl& other) = 0;
  std::string ReflectAsString(const bool prepend_att_class);
};

using AttributeImplGenerator = std::function<AttributeImpl*(void)>;

class AttributeFactory {
 private:
  static std::unordered_map<std::type_index, AttributeImplGenerator>
         ATTR_IMPL_TABLE;

 public:
  AttributeFactory();
  virtual ~AttributeFactory() {}
  virtual AttributeImpl*
  CreateAttributeInstance(const std::type_index attr_type_id) = 0;

 public:
  static AttributeImplGenerator
  FindAttributeImplGenerator(const std::type_index attr_type_id);

  template<typename ATTR_FACTORY, typename ATTR_IMPL>
  static AttributeFactory* GetStaticImplementation() {
    return new StaticImplementationAttributeFactory<ATTR_FACTORY, ATTR_IMPL>();
  }

  template<typename ATTR, typename ATTR_IMPL_CLASS>
  static void RegisterAttributeImpl() {
    ATTR_IMPL_TABLE[Attribute::TypeId<ATTR>()] = [](){
      return new ATTR_IMPL_CLASS();
    };
  }

 public:
  class DefaultAttributeFactory;

  template<typename ATTR_FACTORY, typename ATTR_IMPL>
  class StaticImplementationAttributeFactory;
};

class AttributeFactory::DefaultAttributeFactory: public AttributeFactory {
 public:
  DefaultAttributeFactory();
  virtual ~DefaultAttributeFactory();
  AttributeImpl*
  CreateAttributeInstance(const std::type_index attr_type_id) override;
};

static AttributeFactory::DefaultAttributeFactory DEFAULT_ATTRIBUTE_FACTORY;

template<typename ATTR_FACTORY, typename ATTR_IMPL>
class AttributeFactory::StaticImplementationAttributeFactory
                                                  : public AttributeFactory {
 private:
  ATTR_FACTORY delegate;
  static std::unordered_set<std::type_index> DEFAULT_ATTR_TYPE_IDS;

 public:
  StaticImplementationAttributeFactory() { }
  virtual ~StaticImplementationAttributeFactory() { }
  AttributeImpl*
  CreateAttributeInstance(const std::type_index attr_type_id) override {
    ATTR_IMPL attr_impl;
    std::vector<std::type_index> attributes = attr_impl.Attributes();
    auto it = std::find(attributes.begin(), attributes.end(), attr_type_id);
    if (it != attributes.end()) {
      return new ATTR_IMPL();
    } else {
      return delegate.CreateAttributeInstance(attr_type_id);
    }
  }
};

template<typename ATTR_FACTORY, typename ATTR_IMPL>
std::unordered_set<std::type_index>
AttributeFactory
::StaticImplementationAttributeFactory<ATTR_FACTORY, ATTR_IMPL>
::DEFAULT_ATTR_TYPE_IDS
= []() {
  std::unordered_set<std::type_index>
  ret(AttributeFactory::ATTR_IMPL_TABLE.size());

  for (const auto& p : AttributeFactory::ATTR_IMPL_TABLE) {
    ret.insert(p.first);
  }
  return ret;
}();

class AttributeSource {
 public:
  class State final {
   public:
    AttributeImpl* attribute;
    State* next;
    bool delete_attribute;

   public:
    explicit State(bool delete_attribute = false);
    State(const State& other);
    State(State&& other);
    ~State();
    State& operator=(const State& other);
    State& operator=(State&& other);
    void CleanAttribute() noexcept;
  };

  class StateHolder {
   private:
    static std::function<void(State**)> SAFE_DELETER;
    std::shared_ptr<State*> state_ptr;

   private:
    void ResetState(State* state);

   public:
    StateHolder();
    void NewState();
    void ClearState();
    State* GetState();
  };

 private:
  StateHolder state_holder;
  // Mapping Attribute's type_id to AttributeImpl instance
  std::unordered_map<std::type_index,
                     std::shared_ptr<AttributeImpl>> attributes;
  // Mapping AttributeImpl's type_id to AttributeImpl instance
  std::unordered_map<std::type_index,
                     std::shared_ptr<AttributeImpl>> attribute_impls;
  AttributeFactory& factory;

 private:
  State* GetCurrentState();

 public:
  AttributeSource();
  AttributeSource(const AttributeSource& other);
  /**
   * AttributeSource consturctor.
   * AttributeSource shares a given factory.
   * So factory's life cycle is not managed by this instance.
   */
  explicit AttributeSource(AttributeFactory& factory);
  AttributeFactory& GetAttributeFactory() const;
  void AddAttributeImpl(AttributeImpl* attr_impl);

  template <typename ATTR>
  std::shared_ptr<ATTR> AddAttribute() {
    std::type_index id = Attribute::TypeId<ATTR>();
    auto attr_it = attributes.find(id);
    if (attr_it == attribute_impls.end()) {
      AddAttributeImpl(factory.CreateAttributeInstance(
                               Attribute::TypeId<ATTR>()));
      attr_it = attributes.find(id);
    }

    return std::dynamic_pointer_cast<ATTR>(attr_it->second);
  }

  bool HasAttributes();

  template<typename ATTR>
  bool HasAttribute() {
    return (attributes.find(Attribute::TypeId<ATTR>()) != attributes.end());
  }

  void ClearAttributes();
  void EndAttributes();
  void RemoveAllAttributes();
  State* CaptureState();
  void RestoreState(State* state);
  std::string ReflectAsString(const bool prepend_att);
  void ReflectWith(AttributeReflector& reflector);
  AttributeSource& operator=(const AttributeSource& other);
  void ShallowCopyTo(AttributeSource& other) const;
};

}  // namespace util
}  // namespace core
}  // namespace lucene

#endif  // SRC_UTIL_ATTRIBUTE_H_
