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

#define GET_EXPRESSION_MANAGER_INSTANCE(__EM_INSTANCE)                         \
  using ::epidb::datatypes::ExpressionManager;                                 \
  const auto *__EM_INSTANCE = ExpressionManager::INSTANCE();

#define GET_EXPRESSION_MANAGER(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)      \
  const datatypes::ExpressionTypePtr& __EXPRESSION_TYPE =                      \
       em->get_manager(__EXPRESSION_TYPE_NAME);

#define GET_EXPRESSION_TYPE(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)         \
  GET_EXPRESSION_MANAGER_INSTANCE(em)                                          \
  if (!em->is_expression_type(__EXPRESSION_TYPE_NAME)) {                       \
    std::string _msg;                                                          \
    _msg = Error::m(ERR_INVALID_EXPRESSION_TYPE, __EXPRESSION_TYPE_NAME);      \
    result.add_error(_msg);                                                    \
    return false;                                                              \
  }                                                                            \
  GET_EXPRESSION_MANAGER(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)

#define GET_EXPRESSION_TYPE_MSG(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)     \
  GET_EXPRESSION_MANAGER_INSTANCE(em)                                          \
  if (!em->is_expression_type(__EXPRESSION_TYPE_NAME)) {                       \
    msg = Error::m(ERR_INVALID_EXPRESSION_TYPE, __EXPRESSION_TYPE_NAME);       \
    return false;                                                              \
  }                                                                            \
  GET_EXPRESSION_MANAGER(__EXPRESSION_TYPE_NAME, __EXPRESSION_TYPE)
