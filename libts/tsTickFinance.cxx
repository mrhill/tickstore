
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

    class_<tsTickBid, bases<tsTick> >("tsTickBid")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,bbU64,bbU32>())
        .add_property("price", &tsTickBid::price, &tsTickBid::setPrice)
        .add_property("volume", &tsTickBid::volume, &tsTickBid::setVolume)
        .add_property("opt", &tsTickBid::opt, &tsTickBid::setOpt)
    ;

    class_<tsTickAsk, bases<tsTick> >("tsTickAsk")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,bbU64,bbU32>())
        .add_property("price", &tsTickAsk::price, &tsTickAsk::setPrice)
        .add_property("volume", &tsTickAsk::volume, &tsTickAsk::setVolume)
        .add_property("opt", &tsTickAsk::opt, &tsTickAsk::setOpt)
    ;

    class_<tsTickBidAsk, bases<tsTick> >("tsTickBidAsk")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,double,bbU32>())
        .add_property("priceBid", &tsTickBidAsk::priceBid, &tsTickBidAsk::setPricebid)
        .add_property("priceAsk", &tsTickBidAsk::priceAsk, &tsTickBidAsk::setPriceask)
        .add_property("opt", &tsTickBidAsk::opt, &tsTickBidAsk::setOpt)
    ;
