
    class_<tsTickPrice, bases<tsTick> >("tsTickPrice")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,bbU32>())
        .add_property("price", &tsTickPrice::price, &tsTickPrice::setPrice)
        .add_property("opt", &tsTickPrice::opt, &tsTickPrice::setOpt)
    ;

    class_<tsTickVolume, bases<tsTick> >("tsTickVolume")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,bbU64,bbU32>())
        .add_property("volume", &tsTickVolume::volume, &tsTickVolume::setVolume)
        .add_property("opt", &tsTickVolume::opt, &tsTickVolume::setOpt)
    ;

    class_<tsTickPriceVolume, bases<tsTick> >("tsTickPriceVolume")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,bbU64,bbU32>())
        .add_property("price", &tsTickPriceVolume::price, &tsTickPriceVolume::setPrice)
        .add_property("volume", &tsTickPriceVolume::volume, &tsTickPriceVolume::setVolume)
        .add_property("opt", &tsTickPriceVolume::opt, &tsTickPriceVolume::setOpt)
    ;
