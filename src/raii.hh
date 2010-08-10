#ifndef USOCK_RAII_HH
#define USOCK_RAII_HH


namespace usock
{

    template<typename T>
    struct raii_array
    {
        raii_array(T* array) :
            m_array(array)
        {  }
        ~raii_array() {
            delete [] m_array;
        }

    private:
        T* m_array;
    };

}
#endif // USOCK_RAII_HH
