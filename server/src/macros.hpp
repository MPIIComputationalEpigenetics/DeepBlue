#define ADD_NAMESPACE(NS,FUNC) NS##::##FUNC

#define NEW_DATASET_ID(DATASET_ID, __MSG) {                                      \
      if (!dba::helpers::get_increment_counter("datasets", DATASET_ID, __MSG) || \
          !dba::helpers::notify_change_occurred("datasets", __MSG))  {           \
        return false;                                                            \
      }                                                                          \
    }

#define BUILD_ID(__COLLECTION, __PREFIX, __ID, __MSG) {                        \
      using ::epidb::dba::helpers::get_increment_counter;                      \
      using ::epidb::dba::helpers::notify_change_occurred;                     \
      using ::epidb::dba::Collections;                                         \
      int _id;                                                                 \
      if (!get_increment_counter(Collections::__COLLECTION(), _id, __MSG) ||   \
          !notify_change_occurred(Collections::__COLLECTION(), __MSG))  {      \
        return false;                                                          \
      }                                                                        \
      __ID = #__PREFIX + utils::integer_to_string(_id);                         \
  }
