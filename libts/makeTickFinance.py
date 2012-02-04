#!/usr/bin/python
from string import Template
import sys

if len(sys.argv) < 3:
    print "Syntax: %s <header.h.templ> <impl.cpp.templ>" % sys.argv[0]
    exit(1)

tickTypes = [
    {'name': 'Price', 'scheme':
        """double price
           bbU32  opt""" },

    {'name': 'Volume', 'scheme':
        """bbU64  volume
           bbU32  opt""" },

    {'name': 'PriceVolume', 'scheme':
        """double price
           bbU64  volume
           bbU32  opt""" },

    #{'name': 'Bid',         'scheme':'f'},
    #{'name': 'Ask',         'scheme':'f'},
    #{'name': 'BidAsk',      'scheme':'f'},
]

typeMap = {
    'bbU8'  : [1, 0, '%u',  'B'],
    'bbU16' : [2, 0, '%u',  'B'],
    'bbU32' : [4, 0, '%u',  'L'],
    'bbU64' : [8, 0, '%I64u', 'Q'],
    'float' : [4, 0, '%g',  'f'],
    'double': [8, 0, '%lg', 'd'],
}

def getAttrList(str):
    ret = []
    for i in str.split("\n"):
        ret.append(i.split(None, 1))
    return ret

tickClassTempl = """
struct tsTick$name : tsTick
{
$members
    tsTick$name(const tsObjID& objID$initParams) :
$initializers        tsTick(objID, tsTickType_$name)
    {
    }

$accessors

    static const int tailSize = $tailSize;
    void serializeTail(char* pBuf) const;
    void unserializeTail(const char* pBuf);
    std::string strTail() const;
};
"""

tickClassImplTempl = """
void tsTick${name}::serializeTail(char* pBuf) const
{
}

void tsTick${name}::unserializeTail(const char* pBuf)
{
}

std::string tsTick${name}::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT("${attrFmt}")${attrVar});
    return std::string(str.GetPtr());
}

"""

tickTypeDefs = ''
tickClassDefs = ''

factoryTailSize = ''
factorySerializeTail = ''
factoryUnserializeTail = ''
factoryStrTail = ''
tickClassImpl = ''

for tickType in tickTypes:
    name = tickType['name']
    scheme = getAttrList(tickType['scheme'])

    # Generate .h for this tick type

    tickTypeDefs += "    tsTickType_" + name + ",\n"

    classDict = dict(name=name, initParams='', members='', initializers='', accessors='')
    tailSize = 0
    for (attrType, attrName) in scheme:
        attrMemberName = 'm'+attrName.capitalize()
        classDict['members']      += "    %s %s;\n" % (attrType, attrMemberName)
        classDict['initParams']   += ", %s %s" % (attrType, attrName)
        classDict['initializers'] += "        %s(%s),\n" % (attrMemberName, attrName)

        classDict['accessors']    += """
    inline void set%s(%s %s) { %s = %s; }
    inline %s %s() const { return %s; }""" % (
            attrName.capitalize(), attrType, attrName, attrMemberName, attrName, 
            attrType, attrName, attrMemberName)

        tailSize += typeMap[attrType][0]

    classDict['tailSize'] = tailSize
    tickClassDefs += Template(tickClassTempl).substitute(classDict)

    # Generate .cpp for this tick type

    factoryTailSize += "    case tsTickType_%s: return tsTick%s::tailSize;\n" % (name, name)
    factorySerializeTail += "    case tsTickType_%s: static_cast<const tsTick%s*>(pTick)->serializeTail(pBuf); break;\n" % (name, name)
    factoryUnserializeTail += "    case tsTickType_%s: static_cast<tsTick%s*>(pTick)->unserializeTail(pBuf); break;\n" % (name, name)
    factoryStrTail += "    case tsTickType_%s: return static_cast<const tsTick%s*>(pTick)->strTail();\n" % (name, name)

    classImplDict = dict(name=name, attrFmt='', attrVar='')
    for (attrType, attrName) in scheme:
        attrMemberName = 'm'+attrName.capitalize()
        classImplDict['attrFmt'] += ",%s=%s" % (typeMap[attrType][2], attrName)
        classImplDict['attrVar'] += ", %s" % (attrMemberName)
    tickClassImpl += Template(tickClassImplTempl).substitute(classImplDict)
    

templ = Template(open(sys.argv[1]).read())
open(sys.argv[1].replace(".templ",""), "w").write(
    templ.substitute(dict(tickTypeDefs=tickTypeDefs, tickClassDefs=tickClassDefs)))

templ = Template(open(sys.argv[2]).read())
open(sys.argv[2].replace(".templ",""), "w").write(
    templ.substitute(dict(  factoryTailSize=factoryTailSize, 
                            factorySerializeTail=factorySerializeTail, 
                            factoryUnserializeTail=factoryUnserializeTail, 
                            factoryStrTail=factoryStrTail, 
                            tickClassImpl=tickClassImpl)))
