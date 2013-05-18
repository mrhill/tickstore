#!/usr/bin/python
from string import Template

typeMap = {
    'bbU8'  : [1, 0, '%u'],
    'bbU16' : [2, 0, '%u'],
    'bbU32' : [4, 0, '%u'],
    'bbS32' : [4, 0, '%d'],
    'bbU64' : [8, 0, '%"bbI64"u'],
    'bbS64' : [8, 0, '%"bbI64"d'],
    'float' : [4, 0, '%g'],
    'double': [8, 0, '%lg'],
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
    tsTick$name() :
$initializers0    tsTick(tsTickType_$name) {}

    tsTick$name(const tsObjID& objID) :
$initializers0    tsTick(objID, tsTickType_$name) {}

    tsTick$name(const tsObjID& objID$initParams) :
$initializers    tsTick(objID, tsTickType_$name) {}
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
${serializeTailUnion}${serializeTail}}

void tsTick${name}::unserializeTail(const char* pBuf)
{
${serializeTailUnion}${unserializeHead}}

std::string tsTick${name}::strTail() const
{
    bbStrBuf str;
    str.Printf(bbT("${attrFmt}")${attrVar});
    return std::string(str.GetPtr());
}

"""

tickPyClassTempl = """
    class_<tsTick${name}, bases<tsTick> >("tsTick${name}")
        .def(init<const tsObjID&>())
        .def(init<const tsObjID&${initParams}>())
$pyAccessors    ;
"""

def makeTicks(tickTypes, headerTempl, cppTempl):

	if not headerTempl.endswith("h.templ") or not cppTempl.endswith("cpp.templ"):
		print "headerTempl and cppTempl filename must end with [.h|.cpp].templ"
		return False
	tickTypeDefs = ''
	tickClassDefs = ''

	factoryTailSize = ''
	factorySerializeTail = ''
	factoryUnserializeTail = ''
	factoryStrTail = ''
	tickClassImpl = ''
	tickPyClassImpl = ''

	for tickType in tickTypes:
		name = tickType['name']
		scheme = getAttrList(tickType['scheme'])

		# Generate .h for this tick type

		tickTypeDefs += "    tsTickType_" + name + ",\n"

		classDict = dict(name=name, initParams='', members='', initializers0='', initializers='', accessors='')
		tailSize = 0
		for (attrType, attrName) in scheme:
		    attrMemberName = 'm'+attrName[0].upper()+attrName[1:]
		    classDict['members']      += "    %s %s;\n" % (attrType, attrMemberName)
		    classDict['initParams']   += ", %s %s" % (attrType, attrName)
		    classDict['initializers'] += "        %s(%s),\n" % (attrMemberName, attrName)
		    classDict['initializers0']+= "        %s(0),\n" % (attrMemberName)

		    classDict['accessors']    += """
    inline void set%s(%s %s) { %s = %s; }
    inline %s %s() const { return %s; }""" % (
		        attrName[0].upper()+attrName[1:], attrType, attrName, attrMemberName, attrName,
		        attrType, attrName, attrMemberName)

		    tailSize += typeMap[attrType][0]

		classDict['tailSize'] = tailSize
		tickClassDefs += Template(tickClassTempl).substitute(classDict)

		# Generate .cpp for this tick type

		factoryTailSize += "    case tsTickType_%s: return tsTick%s::tailSize;\n" % (name, name)
		factorySerializeTail += "    case tsTickType_%s: static_cast<const tsTick%s*>(pTick)->serializeTail(pBuf); break;\n" % (name, name)
		factoryUnserializeTail += "    case tsTickType_%s: static_cast<tsTick%s*>(pTick)->unserializeTail(pBuf); break;\n" % (name, name)
		factoryStrTail += "    case tsTickType_%s: return static_cast<const tsTick%s*>(pTick)->strTail();\n" % (name, name)

		classImplDict = dict(name=name, attrFmt='', attrVar='', serializeTail='', serializeTailUnion='', unserializeHead='')
		for (attrType, attrName) in scheme:
		    attrMemberName = 'm'+attrName[0].upper()+attrName[1:]
		    classImplDict['attrFmt'] += ",%s=%s" % (attrName, typeMap[attrType][2])
		    classImplDict['attrVar'] += ", %s" % (attrMemberName)

		    if attrType == 'double' or attrType == 'float':
		        classImplDict['serializeTailUnion'] = "    union { bbU32 u32; float f32; bbU64 u64; double f64; };\n\n"

		    if attrType == 'bbU8':
		        classImplDict['serializeTail']   += "    *pBuf++ = %s;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    %s = *pBuf++\n" % (attrMemberName)
		    if attrType == 'bbU16':
		        classImplDict['serializeTail']   += "    bbST16LE(pBuf, %s); pBuf+=2;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    %s = bbLD16LE(pBuf); pBuf+=2;\n" % (attrMemberName)
		    if attrType == 'bbU32':
		        classImplDict['serializeTail']   += "    bbST32LE(pBuf, %s); pBuf+=4;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    %s = bbLD32LE(pBuf); pBuf+=4;\n" % (attrMemberName)
		    if attrType == 'bbU64':
		        classImplDict['serializeTail']   += "    bbST64LE(pBuf, %s); pBuf+=8;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    %s = bbLD64LE(pBuf); pBuf+=8;\n" % (attrMemberName)
		    elif attrType == 'float':
		        classImplDict['serializeTail']   += "    f32 = %s; bbST32LE(pBuf, u32); pBuf+=4;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    u32 = bbLD32LE(pBuf); pBuf+=4; %s = f32;\n" % (attrMemberName)
		    elif attrType == 'double':
		        classImplDict['serializeTail']   += "    f64 = %s; bbST64LE(pBuf, u64); pBuf+=8;\n" % (attrMemberName)
		        classImplDict['unserializeHead'] += "    u64 = bbLD64LE(pBuf); pBuf+=8; %s = f64;\n" % (attrMemberName)

		tickClassImpl += Template(tickClassImplTempl).substitute(classImplDict)

		# Generate .cxx python wrapper for this tick type

		pyClassImplDict = dict(name=name, pyAccessors='', initParams='')
		for (attrType, attrName) in scheme:
		    attrGet = attrName
		    attrSet = 'set'+attrName[0].upper()+attrName[1:]
		    pyClassImplDict['pyAccessors'] += '        .add_property("%s", &tsTick%s::%s, &tsTick%s::%s)\n' % (attrName, name, attrGet, name, attrSet)
		    pyClassImplDict['initParams'] += ",%s" % (attrType)

		tickPyClassImpl += Template(tickPyClassTempl).substitute(pyClassImplDict)

	templ = Template(open(headerTempl).read())
	open(headerTempl.replace(".templ",""), "w").write(
		templ.substitute(dict(tickTypeDefs=tickTypeDefs, tickClassDefs=tickClassDefs)))

	templ = Template(open(cppTempl).read())
	open(cppTempl.replace(".templ",""), "w").write(
		templ.substitute(dict(  factoryTailSize=factoryTailSize, 
		                        factorySerializeTail=factorySerializeTail, 
		                        factoryUnserializeTail=factoryUnserializeTail, 
		                        factoryStrTail=factoryStrTail, 
		                        tickClassImpl=tickClassImpl)))

	open(cppTempl.replace("cpp.templ","cxx"), "w").write(tickPyClassImpl)

	return True