#include <boost/python.hpp>
#include "tsTickSender.h"
#include "tsTickFinance.h"

static tsTickFactoryFinance tickFactoryFinance;

struct tsTickSenderFinance : public tsTickSender
{
	tsTickSenderFinance() : tsTickSender(tickFactoryFinance, "localhost", 2227) {}
	tsTickSenderFinance(const char* host, int port = 2227) : tsTickSender(tickFactoryFinance, host, port) {}
};

char const* greet()
{
   return "hello, world";
}

BOOST_PYTHON_MODULE(pyts)
{
    using namespace boost::python;
	using self_ns::str;

    def("greet", greet);

    class_<tsObjID>("tsObjID")
        .def(init<bbU32, bbU64>())
        .add_property("exchangeID", &tsObjID::exchangeID, &tsObjID::setExchangeID)
        .add_property("symbolID", &tsObjID::symbolID, &tsObjID::setSymbolID)
        .def(self < self)
        .def(str(self))
    ;

	void (tsTick::*setTimeU64)(bbU64) = &tsTick::setTime;
    class_<tsTick>("tsTick")
        .add_property("objID", make_function( &tsTick::objID, return_value_policy<copy_const_reference>() ), &tsTick::setObjID)
        .add_property("type", &tsTick::type)
        .add_property("count", &tsTick::count, &tsTick::setCount)
        .add_property("time", &tsTick::time, setTimeU64)
        //.def("setTime", &tsTick::setTime)
        .def(str(self))
    ;

    class_<tsTickSenderFinance>("tsTickSender")
        .def(init<const char*, int>())
        .def("send", &tsTickSender::send)
    ;
}
