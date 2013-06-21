
    class_<tsTickAuthReply, bases<tsTick> >("tsTickAuthReply")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,bbU64,bbU32>())
        .add_property("UID", &tsTickAuthReply::UID, &tsTickAuthReply::setUID)
        .add_property("success", &tsTickAuthReply::success, &tsTickAuthReply::setSuccess)
    ;

    class_<tsTickSubscribe, bases<tsTick> >("tsTickSubscribe")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,bbU64>())
        .add_property("feedID", &tsTickSubscribe::feedID, &tsTickSubscribe::setFeedID)
    ;

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
        .add_property("priceBid", &tsTickBidAsk::priceBid, &tsTickBidAsk::setPriceBid)
        .add_property("priceAsk", &tsTickBidAsk::priceAsk, &tsTickBidAsk::setPriceAsk)
        .add_property("opt", &tsTickBidAsk::opt, &tsTickBidAsk::setOpt)
    ;

    class_<tsTickRecap, bases<tsTick> >("tsTickRecap")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,double,double,double,bbU64,bbU64,bbU32>())
        .add_property("open", &tsTickRecap::open, &tsTickRecap::setOpen)
        .add_property("high", &tsTickRecap::high, &tsTickRecap::setHigh)
        .add_property("low", &tsTickRecap::low, &tsTickRecap::setLow)
        .add_property("close", &tsTickRecap::close, &tsTickRecap::setClose)
        .add_property("volume", &tsTickRecap::volume, &tsTickRecap::setVolume)
        .add_property("openInt", &tsTickRecap::openInt, &tsTickRecap::setOpenInt)
        .add_property("opt", &tsTickRecap::opt, &tsTickRecap::setOpt)
    ;

    class_<tsTickS32, bases<tsTick> >("tsTickS32")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,bbS32,bbU32>())
        .add_property("count", &tsTickS32::count, &tsTickS32::setCount)
        .add_property("opt", &tsTickS32::opt, &tsTickS32::setOpt)
    ;

    class_<tsTickF64, bases<tsTick> >("tsTickF64")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&,double,bbU32>())
        .add_property("value", &tsTickF64::value, &tsTickF64::setValue)
        .add_property("opt", &tsTickF64::opt, &tsTickF64::setOpt)
    ;
