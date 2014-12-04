//http://www.codeproject.com/Articles/647856/Performance-Improvement-with-the-StringBuilde

#include <numeric>


template <typename chr>
class StringBuilder {
    typedef std::basic_string<chr> string_t;
    // Tried also vector and list. Deque wan, albeit by a narrow margin.
    typedef std::deque<string_t> container_t;
    // Reuse the size type in the string.
    typedef typename string_t::size_type size_type;
    container_t m_Data;
    size_type m_totalSize;

    // No copy constructor, no assignement.
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

    void endLine() {
        static chr lineFeed[] { 10, 0 };
        m_Data.push_back(lineFeed);
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
