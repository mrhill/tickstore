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

    def("greet", greet);

    class_<tsTickSenderFinance>("tsTickSender")
        .def(init<const char*, int>())
        .def("send", &tsTickSender::send)
    ;

    class_<tsTick>("tsTick")
        .add_property("type", &tsTick::type)
        .add_property("count", &tsTick::count, &tsTick::setCount)
        .def("time", &tsTick::time)
        //.def("setTime", &tsTick::setTime)
        //.def(str(self))
    ;
}
