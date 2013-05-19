#include <string>
#include <iostream>
#include <boost/python.hpp>
#include "tsTime.h"
#include "tsTickSender.h"
#include "tsTickFinance.h"

using namespace boost::python;

static tsTickFactoryFinance tickFactoryFinance;

struct tsTickSenderFinance : public tsTickSender
{
    static std::string getScriptName()
    {
        object main = import("__main__");
        object main_file(main.attr("__file__"));
        std::string s = extract<std::string>(main_file);

        size_t pos = s.rfind("/");
        if (pos != std::string::npos)
            s.erase(0, pos+1);

        if (s.find_first_of(".py", s.size()-3) != std::string::npos)
            s.erase(s.size()-3);

        return s;
    }

    tsTickSenderFinance()
      : tsTickSender(tickFactoryFinance, getScriptName().c_str(), "localhost", 2227)
    {
    }

    tsTickSenderFinance(const char* host, int port = 2227)
      : tsTickSender(tickFactoryFinance, getScriptName().c_str(), host, port)
    {}

    tsTickSenderFinance(const char* queueName, const char* host, int port = 2227)
      : tsTickSender(tickFactoryFinance, queueName, host, port)
    {}
};

bbU64 symFromStr(char const* pStr)
{
    bbU64 sym = 0;
    bbUINT i = 0;
    while(char c = *pStr++) {
        sym |= (bbU64)c << i; i+=8;
    }
    return sym;
}

BOOST_PYTHON_MODULE(pyts)
{
    using self_ns::str;

    def("symFromStr", symFromStr);

    class_<tsTime>("tsTime")
        .def(init<int,int,int, int,int,int, int>())
        .def(str(self))
    ;

    class_<tsObjID>("tsObjID")
        .def(init<bbU64, bbU64>())
        .add_property("feedID", &tsObjID::feedID, &tsObjID::setFeedID)
        .add_property("symbolID", &tsObjID::symbolID, &tsObjID::setSymbolID)
        .def(self < self)
        .def(str(self))
    ;

    void (tsTick::*setTimeU64)(bbU64) = &tsTick::setTime;
    void (tsTick::*setTimeTS)(const tsTime&) = &tsTick::setTime;
    class_<tsTick>("tsTick")
        .add_property("objID", make_function( &tsTick::objID, return_value_policy<copy_const_reference>() ), &tsTick::setObjID)
        .add_property("type", &tsTick::type)
        .add_property("count", &tsTick::count, &tsTick::setCount)
        .add_property("time", &tsTick::time, setTimeU64)
        .def("setTime", setTimeTS)
        .def(str(self))
    ;

    class_<tsTickDiag, bases<tsTick> >("tsTickDiag")
        .def(init<const tsObjID&>())
        .add_property("sendTime", &tsTickDiag::sendTime, &tsTickDiag::setSendTime)
        .add_property("receiveTime", &tsTickDiag::receiveTime, &tsTickDiag::setReceiveTime)
        .add_property("storeTime", &tsTickDiag::storeTime, &tsTickDiag::setStoreTime)
    ;

    #include "tsTickFinance.cxx"

    class_<tsTickSenderFinance>("tsTickSender")
        .def(init<const char*, const char*, int>())
        .def("send", &tsTickSender::send)
    ;
}

