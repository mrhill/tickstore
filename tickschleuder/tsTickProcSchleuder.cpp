#include "tsTickProcSchleuder.h"
#include "tsTickFinance.h"

tsMonSymbol* tsTickProcSchleuder::GetMon(const tsObjID& objID)
{
    tsMonSymbol* pMon;
    ObjIDMonMap::iterator it = mObjID2Mon.find(objID);

    if (it == mObjID2Mon.end())
        mObjID2Mon[objID] = pMon = new tsMonSymbol;
    else
        pMon = it->second;
        
    return pMon;
}

void tsTickProcSchleuder::Proc(const tsTick& tick)
{
    bool setPrice  = false;
    bool setVolume = false;

    double price = 0;
    bbU64 volume = 0;

    switch(tick.type())
    {
    case tsTickType_Price:
        {
            const tsTickPrice& tickPrice = static_cast<const tsTickPrice&>(tick);
            price = tickPrice.price();
            setPrice = true;
        }
        break;

    case tsTickType_Volume:
        {
            const tsTickVolume& tickVolume = static_cast<const tsTickVolume&>(tick);
            volume = tickVolume.volume();
            setVolume = true;
        }
        break;

    case tsTickType_PriceVolume:
        {
            const tsTickPriceVolume& tickPriceVolume = static_cast<const tsTickPriceVolume&>(tick);
            price = tickPriceVolume.price();
            setPrice = true;
            volume = tickPriceVolume.volume();
            setVolume = true;
        }
        break;

    default:
        return;
    }

    tsMonSymbol* pMon = GetMon(tick.objID());
    if (!pMon)
        return;

    if (setPrice)
    {
        pMon->setPrice(price);
     
        if (!pMon->highValid() || price > pMon->high())
            pMon->setHigh(price);
        
        if (!pMon->lowValid() || price < pMon->low())
            pMon->setLow(price);

        if (!pMon->openValid())
            pMon->setOpen(price);
    }

    if (setVolume)
    {
        pMon->setVolume(volume);
        pMon->addVolume(volume);
    }
}

