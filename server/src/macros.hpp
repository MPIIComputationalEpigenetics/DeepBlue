#define ADD_NAMESPACE(NS,FUNC) NS##::##FUNC

#define NEW_DATASET_ID(DATASET_ID, __MSG) {                                    \
  if (!dba::helpers::get_increment_counter("datasets", DATASET_ID, __MSG) ||   \
    !dba::helpers::notify_change_occurred("datasets", __MSG))  {               \
      return false;                                                            \
    }                                                                          \
}

#define BUILD_ID(__COLLECTION, __PREFIX, __ID, __MSG) {                        \
  using ::epidb::dba::helpers::get_increment_counter;                          \
  using ::epidb::dba::helpers::notify_change_occurred;                         \
  using ::epidb::dba::Collections;                                             \
  int _id;                                                                     \
  if (!get_increment_counter(Collections::__COLLECTION(), _id, __MSG) ||       \
    !notify_change_occurred(Collections::__COLLECTION(), __MSG))  {            \
      return false;                                                            \
  }                                                                            \
  __ID = #__PREFIX + utils::integer_to_string(_id);                            \
}


#define GET_EXPRESSION_TYPE(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)         \
  using ::epidb::datatypes::ExpressionManager;                                 \
  const auto *em = ExpressionManager::INSTANCE();                              \
  if (!em->is_expression_type(__EXPRESSION_TYPE_NAME)) {                       \
    std::string _msg;                                                          \
    _msg = Error::m(ERR_INVALID_EXPRESSION_TYPE, __EXPRESSION_TYPE_NAME);      \
    result.add_error(_msg);                                                    \
    return false;                                                              \
  }                                                                            \
  const datatypes::ExpressionTypePtr& __EXPRESSION_TYPE =                      \
       em->get_manager(expression_type_name);                                  \

