#include <typeinfo>
#include <sstream>
#include <stdexcept>
#include <Util/Attribute.h>
#include <Analysis/AttributeImpl.h>

using namespace lucene::core::util;
using namespace lucene::core::analysis::tokenattributes;

/**
 * AttributeImpl
 */
void AttributeImpl::End() {
  Clear();
}

std::string AttributeImpl::ReflectAsString(const bool prepend_att_class) {
  std::stringstream buf;
  AttributeReflector f = [&buf, &prepend_att_class](const std::string& att_class, const std::string& key, const std::string& value){
    if(buf.tellp() > 0) {
      buf << ',';
    }

    if(prepend_att_class) {
      buf << att_class << '#';
    }

    buf << key << '=' << (value.empty() ? "null" : value);
  };

  ReflectWith(f);

  return buf.str();
}

/**
 * AttributeFactory
 */
std::unordered_map<type_id, AttributeImplGenerator>
AttributeFactory::ATTR_IMPL_TABLE = {
    {Attribute::TypeId<BytesTermAttribute>(), [](){ return new BytesTermAttributeImpl(); }},
    {Attribute::TypeId<CharTermAttribute>(), [](){ return new CharTermAttributeImpl(); }},
    {Attribute::TypeId<FlagsAttribute>(), [](){ return new FlagsAttributeImpl(); }},
    {Attribute::TypeId<KeywordAttribute>(), [](){ return new KeywordAttributeImpl(); }},
    {Attribute::TypeId<OffsetAttribute>(), [](){ return new OffsetAttributeImpl(); }},
    {Attribute::TypeId<PayloadAttribute>(), [](){ return new PayloadAttributeImpl(); }},
    {Attribute::TypeId<PositionIncrementAttribute>(), [](){ return new PositionIncrementAttributeImpl(); }},
    {Attribute::TypeId<PositionLengthAttribute>(), [](){ return new PositionLengthAttributeImpl(); }},
    {Attribute::TypeId<TermFrequencyAttribute>(), [](){ return new TermFrequencyAttributeImpl(); }},
    {Attribute::TypeId<TypeAttribute>(), [](){ return new TypeAttributeImpl(); }}
 };

AttributeFactory::AttributeFactory() {
}

AttributeImplGenerator AttributeFactory::FindAttributeImplGenerator(const type_id attr_type_id) {
  auto it = AttributeFactory::ATTR_IMPL_TABLE.find(attr_type_id);

  if(it == AttributeFactory::ATTR_IMPL_TABLE.end()) {
    throw std::runtime_error("Attribute " + std::to_string(attr_type_id) + " implmentation was not found");
  }

  return it->second;
}

/**
 * DefaultAttributeFactory
 */
AttributeFactory::DefaultAttributeFactory DEFAULT_ATTRIBUTE_FACTORY();

AttributeFactory::DefaultAttributeFactory::DefaultAttributeFactory() { }

AttributeFactory::DefaultAttributeFactory::~DefaultAttributeFactory() { }

AttributeImpl* AttributeFactory::DefaultAttributeFactory::CreateAttributeInstance(type_id attr_type_id) {
  auto generator = AttributeFactory::FindAttributeImplGenerator(attr_type_id);
  return generator();
}

/**
 * AttributeSource
 */
AttributeSource::AttributeSource()
  : AttributeSource(DEFAULT_ATTRIBUTE_FACTORY) {
}

AttributeSource::AttributeSource(const AttributeSource& other)
  : state_holder(other.state_holder),
    attributes(other.attributes),
    attribute_impls(other.attribute_impls),
    factory(other.factory) {
}

AttributeSource::AttributeSource(AttributeFactory& factory)
  : state_holder(),
    attributes(),
    attribute_impls(),
    factory(factory) {
}

AttributeFactory& AttributeSource::GetAttributeFactory() const {
  return factory;
}

void AttributeSource::AddAttributeImpl(AttributeImpl* attr_impl) {
  std::vector<type_id> attr_type_ids = attr_impl->Attributes();
  std::shared_ptr<AttributeImpl> attr_impl_shptr(attr_impl);

  for(size_t attr_type_id : attr_type_ids) {
    attributes[attr_type_id] = attr_impl_shptr;
  }

  attribute_impls[typeid(*attr_impl).hash_code()] = attr_impl_shptr;
  // Invalidate state to force recomputation in CaptureState()
  state_holder.ClearState();
}

bool AttributeSource::HasAttributes()  {
  return !attributes.empty();
}

AttributeSource::State* AttributeSource::GetCurrentState() {
  AttributeSource::State* current_state = state_holder.GetState();
  if(current_state != nullptr || !HasAttributes()) {
    return current_state;
  }

  state_holder.NewState();
  AttributeSource::State* head = state_holder.GetState();
  AttributeSource::State* c = head;
  auto it = attribute_impls.begin();
  c->attribute = it->second.get();
  it++;

  while(it != attribute_impls.end()) {
    c->next = new AttributeSource::State();
    c = c->next;
    c->attribute = it->second.get();
    it++;
  }

  return head;
}

void AttributeSource::ClearAttributes() {
  for(AttributeSource::State* state = GetCurrentState() ; state != nullptr ; state = state->next) {
    state->attribute->Clear();
  }
}

void AttributeSource::EndAttributes() {
  for(AttributeSource::State* state = GetCurrentState() ; state != nullptr ; state = state->next) {
    state->attribute->End();
  }
}

void AttributeSource::RemoveAllAttributes() {
  attributes.clear();
  attribute_impls.clear();
}

AttributeSource::State* AttributeSource::CaptureState() {
  AttributeSource::State* curr_state = GetCurrentState();
  if(curr_state == nullptr) {
    return nullptr;
  } else {
    return new AttributeSource::State(*curr_state);
  }
}

void AttributeSource::RestoreState(AttributeSource::State* state) {
  if(state == nullptr || state->attribute == nullptr) return;

  AttributeSource::State* current = state;
  do {
    AttributeImpl* source_attr = current->attribute;
    type_id attr_impl_type_id = typeid(*source_attr).hash_code();

    auto it = attribute_impls.find(attr_impl_type_id);
    if(it == attribute_impls.end()) {
      throw std::invalid_argument("AttributeSource::State contains AttributeImpl of type " + std::to_string(attr_impl_type_id) + " that is not in this AttributeSource");
    }

    AttributeImpl* target_attr = it->second.get();
    *target_attr = *source_attr;

    current = current->next;
  } while(current != nullptr);
}

std::string AttributeSource::ReflectAsString(const bool prepend_att) {
  // TODO Implement this.
  return "NOOP";
}

void AttributeSource::ReflectWith(AttributeReflector& reflector)  {
  // TODO Implement this
}

AttributeSource& AttributeSource::operator=(const AttributeSource& other) {
  attributes = other.attributes;
  attribute_impls = other.attribute_impls;
  factory = other.factory;
}


/**
 *  AttributeSource::State
 */
AttributeSource::State::State(bool delete_attribute/*=false*/)
  : attribute(nullptr),
    next(nullptr),
    delete_attribute(delete_attribute) {
}

AttributeSource::State::State(const AttributeSource::State& other)
  : attribute(other.attribute == nullptr ? nullptr : other.attribute->Clone()),
    next(nullptr),
    delete_attribute(true) {
  AttributeSource::State* curr = this;
  for(AttributeSource::State* from = other.next ; from != nullptr ; from = from->next) {
    curr->next = new AttributeSource::State(true);
    curr->next->attribute = from->attribute->Clone();
    curr = curr->next;
  }
}

AttributeSource::State::State(AttributeSource::State&& other)
  : attribute(other.attribute),
    next(other.next),
    delete_attribute(other.delete_attribute) {
  other.delete_attribute = false;
  other.next = nullptr;
}

AttributeSource::State::~State() {
  CleanAttribute();
  if(next != nullptr) {
    next->~State();
  }
}

AttributeSource::State& AttributeSource::State::operator=(const AttributeSource::State& other) {
  this->~State();

  attribute = other.attribute->Clone();
  delete_attribute = true;
  AttributeSource::State* curr = this;

  for(AttributeSource::State* from = other.next ; from != nullptr ; from = from->next) {
    curr->next = new AttributeSource::State(true);
    curr->next->attribute = from->attribute->Clone();
    curr = curr->next;
  }
}

AttributeSource::State& AttributeSource::State::operator=(AttributeSource::State&& other) {
  this->~State();

  attribute = other.attribute;
  next = other.next;

  other.delete_attribute = false;
  other.next = nullptr;
}

void AttributeSource::State::CleanAttribute() noexcept {
  if(delete_attribute && attribute) {
    delete attribute;
    attribute = nullptr;
  }
}

/**
 * AttributeSource::StateHolder
 */
std::function<void(AttributeSource::State**)> AttributeSource::StateHolder::SAFE_DELETER = [](State** state_ptr){
  if(*state_ptr != nullptr) {
    delete *state_ptr;
  }

  delete state_ptr;
};

AttributeSource::StateHolder::StateHolder()
  : state_ptr(new State*(nullptr), AttributeSource::StateHolder::SAFE_DELETER) {
}

void AttributeSource::StateHolder::ResetState(State* state) {
  if(*state_ptr != nullptr) {
    delete *state_ptr;
  }

  *state_ptr = state;
}

AttributeSource::State* AttributeSource::StateHolder::GetState() {
  return *state_ptr;
}

void AttributeSource::StateHolder::NewState() {
  ResetState(new AttributeSource::State());
}

void AttributeSource::StateHolder::ClearState() {
  ResetState(nullptr);
}
