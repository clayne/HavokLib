/*	Havok Format Library
	Copyright(C) 2016-2019 Lukas Cone

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#include "hkNewHeader.h"
#include "datas/binreader.hpp"
#include "datas/macroLoop.hpp"
#include "datas/masterprinter.hpp"
#include <map>

typedef std::map<unsigned int, hkVirtualClass*(*)()> hkClassMapper;
extern hkClassMapper hkClassStorage;

constexpr uint _ToFourCC(const char *input)
{
	return reinterpret_cast<const uint&>(input[2]);
}

#define classFourCC(iname) _ToFourCC(#iname)

int chSDKVRead(BinReader * rd, hkChunk *, hkxNewHeader *root)
{
	if (!root)
		return 1;

	rd->ReadBuffer(root->contentsVersionStripped, 4);
	rd->Skip(4);

	return 0;
}

int chDATARead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;

	const int dataSize = holder->Size();
	root->dataBuffer = static_cast<char*>(malloc(dataSize));
	rd->ReadBuffer(root->dataBuffer, dataSize);

	return 0;
}

int chTSTRRead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;


	const int bufferSize = holder->Size();
	char *&buffer = root->classNamesBuffer;
	buffer = static_cast<char*>(malloc(bufferSize));
	rd->ReadBuffer(buffer, bufferSize);

	int numStrings = 0;

	for (int t = 0; t < bufferSize; t++)
		if (!buffer[t])
			numStrings++;

	_clVec &vec = root->classNames;
	vec.reserve(++numStrings);	
	
	char *lastPtr = buffer;

	for (int t = 0; t < bufferSize; t++)
	{
		if (t && !buffer[t - 1] && !buffer[t])
			break;

		if (!buffer[t])
		{
			vec.push_back(lastPtr);
			lastPtr = buffer + t + 1;
		}
	}
	
	return 0;
}

int chFSTRRead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;

	
	const int bufferSize = holder->Size();
	char *&buffer = root->memberNamesBuffer;
	buffer = static_cast<char*>(malloc(bufferSize));
	rd->ReadBuffer(buffer, bufferSize);

	int numStrings = 0;

	for (int t = 0; t < bufferSize; t++)
		if (!buffer[t])
			numStrings++;

	_clVec &vec = root->memberNames;
	vec.reserve(++numStrings);

	char *lastPtr = buffer;

	for (int t = 0; t < bufferSize; t++)
	{
		if (t && !buffer[t - 1] && !buffer[t])
			break;

		if (!buffer[t])
		{
			vec.push_back(lastPtr);
			lastPtr = buffer + t + 1;
		}
	}

	return 0;
}

ES_INLINE int ReadCompressedInt(BinReader *rd)
{
	uchar tempChar;
	int resultInt = 0;

	rd->Read(tempChar);

	const bool flag1 = (tempChar & 0x80) == 0x80;
	const bool flag2 = (tempChar & 0xC0) == 0xC0;
	const bool flag3 = (tempChar & 0xE0) == 0xE0;

	if (flag3)
	{
		rd->Read(resultInt);
		resultInt |= (tempChar & 0xf) << 4;
	}
	else if (flag2)
	{
		printerror("[Havok] Unhandled int compression : 0xC0!")
	}
	else if (flag1)
	{
		rd->Read(resultInt, 1);
		resultInt |= (static_cast<int>(tempChar) & 0xf) << 8;
	}
	else
		resultInt = tempChar;

	return resultInt;
}

int chTNAMRead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;

	const size_t savepos = rd->Tell();
	const int numClasses = ReadCompressedInt(rd) - 1;

	root->weldedClassNames.resize(numClasses);

	for (auto &c : root->weldedClassNames)
	{
		const int classNameIndex = ReadCompressedInt(rd);
		c.className = root->classNames[classNameIndex];
		
		uchar numTemplateArgs;
		rd->Read(numTemplateArgs);
		c.templateArguments.resize(numTemplateArgs);

		for (auto &t : c.templateArguments)
		{
			int argNameIndex = ReadCompressedInt(rd);
			t.argName = root->classNames[argNameIndex];

			argNameIndex = ReadCompressedInt(rd);
			t.argType = &root->weldedClassNames[argNameIndex-1];
		}		
	}

	const size_t diff = holder->Size() - (rd->Tell() - savepos);

	if (diff < 0)
	{
		printwarning("[Havok] TNAM chunk read too much data, possible incorrect class map, proceeding to next chunk.");
	}

	rd->Skip(diff);

	return 0;
}

int chITEMRead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;

	const int numFixups = holder->Size() / sizeof(classEntryFixup);

	rd->ReadContainer(root->classEntries, numFixups);

	return 0;
}

ES_INLINE std::string _hkGenerateClassnameNew(hkxNewHeader *hdr, std::string className)
{
	return className + "_t<" + className + hdr->contentsVersionStripped + "_t<" + (ES_X64 ? "hkRealPointer" : "hkFakePointer") + ">>";
}

int chPTCHRead(BinReader * rd, hkChunk *holder, hkxNewHeader *root)
{
	if (!holder || !root)
		return 1;

	const size_t endPos = rd->Tell() + holder->Size();

	while (rd->Tell() < endPos)
	{
		int classNameIndex;
		rd->Read(classNameIndex);
		
		const char *toclassname = root->weldedClassNames[classNameIndex-1].className;

		bool isHKArray = !strcmp(toclassname, "hkArray");

		int numPointers;
		rd->Read(numPointers);

		for (int t = 0; t < numPointers; t++)
		{
			int cPointer;
			rd->Read(cPointer);
			
			uintptr_t *retarget = reinterpret_cast<uintptr_t*>(root->dataBuffer + cPointer);
			const classEntryFixup &xfix = root->classEntries[*retarget];
			
			*retarget = reinterpret_cast<uintptr_t>(xfix.tag.hash + root->dataBuffer);

			if (isHKArray)
				*(retarget + 1) = xfix.count;

		}
	}
	
	for (auto &f : root->classEntries)
	{
		const int clsID = f.Size() - 1;

		if (clsID < 0)
			continue;

		const char *clName = root->weldedClassNames[clsID].className;
		std::string compiledClassname = _hkGenerateClassnameNew(root, clName);
		const JenHash _chash = JenkinsHash(compiledClassname.c_str(), static_cast<int>(compiledClassname.size()));

		if (hkClassStorage.count(_chash))
		{
			hkVirtualClass *cls = hkClassStorage[_chash]();
			cls->SetDataPointer(root->dataBuffer + f.tag.hash);
			cls->namePtr = clName;
			cls->masterBuffer = root->dataBuffer;
			cls->header = root;

			root->virtualClasses.push_back(cls);
			cls->Process();
		}
	}

	root->classEntries.~vector();

	return 0;
}

int ReadBlankChunk(BinReader * rd, hkChunk *holder, hkxNewHeader *)
{
	if (!holder)
		return 1;

	rd->Skip(holder->Size());
	return 0;
}

int ReadBlankChunkNoSkip(BinReader *, hkChunk *, hkxNewHeader *) { return 0; }

#define chunkRegister(value) {_ToFourCC(#value), value},
#define chunkRegisterBlank(value) {_ToFourCC(#value), ReadBlankChunk},
#define chunkRegisterBlankNoskip(value) {_ToFourCC(#value), ReadBlankChunkNoSkip},

static const std::map<const uint, int(*)(BinReader *, hkChunk *, hkxNewHeader *)> hkChunkRegistry = 
{
	StaticFor(chunkRegister, chSDKVRead, chDATARead, chTSTRRead, chFSTRRead, chTNAMRead, chITEMRead, chPTCHRead)
	StaticFor(chunkRegisterBlank, chTAG0, chTPTR, chTPAD, chTHSH)
	StaticFor(chunkRegisterBlankNoskip, chTYPE, chINDX)
};

#define hkRegisterClass(classname) hkChunkRegistry[classname::HASH] = &hkCreateChunk<classname>;


int hkChunk::Read(BinReader * rd, hkxNewHeader * root)
{
	rd->Read(*this);
	Reorder();

	if (!hkChunkRegistry.count(tag.hash))
	{
		printwarning("[Havok] Unhandled tag chunk: ", << tag.fourCC[0] << tag.fourCC[1] << tag.fourCC[2] << tag.fourCC[3]);
		rd->Skip(Size());
		return 0;
	}

	if (hkChunkRegistry.at(tag.hash)(rd, this, root))
	{
		printerror("[Havok] Error reading chunk: ", << tag.fourCC[0] << tag.fourCC[1] << tag.fourCC[2] << tag.fourCC[3]);
		return 1;
	}	

	return 0;
}

int hkxNewHeader::GetVersion()
{
	return atoi(contentsVersionStripped);
}

int hkxNewHeader::Load(BinReader *rd)
{
	rd->Read(static_cast<hkChunk&>(*this));
	Reorder();

	if (tag != classFourCC(chTAG0))
	{
		printerror("[Havok] Invalid packfile.");
		return 1;
	}

	const int fileSize = Size();

	while (rd->Tell() < fileSize)
	{
		hkChunk curChunk;

		if (curChunk.Read(rd, this))
			return 1;
	}

	return 0;
}

hkxNewHeader::~hkxNewHeader()
{
	if (dataBuffer)
		free(dataBuffer);
	if (classNamesBuffer)
		free(classNamesBuffer);
	if (memberNamesBuffer)
		free(memberNamesBuffer);

	for (auto &v : virtualClasses)
		delete v;
}

void hkxNewHeader::DumpClassNames(std::ostream &str)
{
	for (auto &cl : weldedClassNames)
	{
		str << cl.className;

		if (cl.templateArguments.size())
		{
			str << '<';

			for (auto &t : cl.templateArguments)
			{
				str << "typename " << t.argName << " = " << t.argType->className << ", ";
			}

			str.seekp(-2, SEEK_CUR);
			str << '>';
		}

		str << '\n';
	}
}

std::vector<hkVirtualClass*> hkxNewHeader::GetClasses(const char * hkClassName)
{
	std::vector<hkVirtualClass*> buffa;

	std::string hkfullclass = _hkGenerateClassnameNew(this, hkClassName);

	JenHash hash = JenkinsHash(hkfullclass.c_str(), static_cast<int>(hkfullclass.size()));

	for (auto &c : virtualClasses)
		if (c->hash == hash)
			buffa.push_back(c);

	return buffa;
}

const hkVirtualClass * hkxNewHeader::GetClass(const void * ptr)
{
	for (auto &c : virtualClasses)
		if (c->GetPointer() == ptr)
			return c;

	return nullptr;
}