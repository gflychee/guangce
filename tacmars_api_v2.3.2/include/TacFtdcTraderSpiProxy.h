#pragma once
#include "TacFtdcTraderApi.h"
#include <utility>

class CTacFtdcTraderSpiProxy{
private:
    static constexpr const size_t vtableCounterBase = __COUNTER__ + 1;
    //__COUNTER__是宏计数，每出现一次值+1
#undef DEFINE_SPI
#define DEFINE_SPI(function_name,...) \
    public:\
    template<\
        typename ... Args\
        /*,typename = std::void_t<\
            decltype(\
                std::declval<void(*)(__VA_ARGS__)>()\
                (std::declval<Args&&>()...)\
            )\
        >*/\
    >\
    void function_name(Args&&...args)\
    {\
        return reinterpret_cast<void(*)(CTacFtdcTraderSpi*,##__VA_ARGS__)>\
            (m_vtable[__COUNTER__ - vtableCounterBase])(\
                m_spi\
                ,std::forward<Args>(args)...\
            );\
    }
#include "TacFtdcTraderSpiInterface.h"

public:
    CTacFtdcTraderSpiProxy(CTacFtdcTraderSpi* spi)
    {
        *this = spi;
    }

    CTacFtdcTraderSpiProxy& operator=(CTacFtdcTraderSpi* spi){
        if(!spi){
            return *this;
        }
        m_spi = spi;
        memcpy(m_vtable,*reinterpret_cast<void**>(m_spi),sizeof(m_vtable));
        return *this;
    }

    CTacFtdcTraderSpiProxy* operator->(){
        return this;
    }

    operator CTacFtdcTraderSpi*() const
    {
        return m_spi;
    }
private:
    CTacFtdcTraderSpi* m_spi;
    void* m_vtable[__COUNTER__ - vtableCounterBase];
};