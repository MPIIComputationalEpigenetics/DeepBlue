//http://www.codeproject.com/Articles/647856/Performance-Improvement-with-the-StringBuilde

#include <numeric>


template <typename chr>
class StringBuilder {
    typedef std::basic_string<chr> string_t;
    typedef std::deque<string_t> container_t;
    typedef typename string_t::size_type size_type;
    container_t m_Data;
    size_type m_totalSize;

    StringBuilder(const StringBuilder &);
    StringBuilder & operator = (const StringBuilder &);

public:
    StringBuilder(const string_t &src) {
        if (!src.empty()) {
            m_Data.push_back(src);
        }
        m_totalSize = src.size();
    }
    StringBuilder() {
        m_totalSize = 0;
    }

    void append(const string_t &src) {
        m_Data.push_back(src);
        m_totalSize += src.size();
    }

    void append(string_t&& src) {
        m_Data.push_back(src);
        m_totalSize += src.size();
    }

    void tab() {
        static std::string tab("\t");
        m_Data.push_back(tab);
        m_totalSize += 1;
    }

    void endLine() {
        static std::string new_line("\n");
        m_Data.push_back(new_line);
        m_totalSize += 2;
    }

    string_t ToString() const {
        string_t result;
        result.reserve(m_totalSize + 1);
        for (auto iter = m_Data.begin(); iter != m_Data.end(); ++iter) {
            result += *iter;
        }
        return result;
    }

}; // class StringBuilder
