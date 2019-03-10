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

#pragma once
#include "HavokApi.hpp"
#include "hkObjectBase.h"
#include "datas/disabler.hpp"

template<class C> struct hkaSkeleton_t : hkaSkeleton
{
	C *Data;
	hkClassConstructor(hkaSkeleton_t<C>);

	const int GetNumBones() const { return Data->GetNumBones(); }
	const char *GetBoneName(int id) const { return Data->GetBoneName(masterBuffer, id); }
	const hkQTransform *GetBoneTM(int id) const { return Data->GetBoneTM(masterBuffer, id); }
	const short GetBoneParentID(int id) const { return Data->GetBoneParentID(masterBuffer, id); }

	const char *GetSkeletonName() const { return Data->GetSkeletonName(masterBuffer); };
	const int GetNumFloatSlots() const { return Data->GetNumFloatSlots(); };
	const char *GetFloatSlot(int id) const { return Data->GetFloatSlot(masterBuffer, id); };

	const int GetNumLocalFrames() const { return Data->GetNumLocalFrames(); };
	const hkLocalFrameOnBone GetLocalFrame(int id) const { return Data->GetLocalFrame(masterBuffer, id); };

	const int GetNumPartitions() const { return Data->GetNumPartitions(); };
	const hkaPartition GetPartition(int id) const { return Data->GetPartition(masterBuffer, id); };
	const int GetNumReferenceFloats() const { return Data->GetNumReferenceFloats(); };
	const float GetReferenceFloat(int id) const { return Data->GetReferenceFloat(masterBuffer, id); }

	void SwapEndian() { Data->SwapEndian(masterBuffer); }
};

template<template<class C>class _ipointer>struct hkLocalFrameOnBone_t
{
	_ipointer<hkLocalFrame> localFrame;
	int boneIndex;
};

template<template<class C>class _ipointer>struct hkLocalFrameOnBone2_t
{
	_ipointer<hkLocalFrame> localFrame;
	short boneIndex;
};


template<template<class C>class _ipointer>struct hkaBone_t
{
	_ipointer<char> name;
	int lockTranslation;
};

template<template<class C>class _ipointer> struct hkaPartition_t
{
	_ipointer<char> Name;
	short startBoneIndex,
		numBones;
};

template<
	template<class C>class _ipointer,
	template<template<class C>class __ipointer> class _parent
>struct hkaSkeleton_t_shared : _parent<_ipointer>
{
	enablePtrPair(bones) GetNumBones() const { return numBones; }
	enablehkArray(bones) GetNumBones() const { return bones.count; }
	enablePtrPairRef(bones) GetNumBones() { return numBones; }
	enablehkArrayRef(bones) GetNumBones() { return bones.count; }

	enablePtrPair(floatSlots) GetNumFloatSlots() const { return numFloatSlots; }
	enablehkArray(floatSlots) GetNumFloatSlots() const { return floatSlots.count; }
	enablePtrPairRef(floatSlots) GetNumFloatSlots() { return numFloatSlots; }
	enablehkArrayRef(floatSlots) GetNumFloatSlots() { return floatSlots.count; }

	enablePtrPair(localFrames) GetNumLocalFrames() const { return numLocalFrames; }
	enablehkArray(localFrames) GetNumLocalFrames() const { return localFrames.count; }
	enablePtrPairRef(localFrames) GetNumLocalFrames() { return numLocalFrames; }
	enablehkArrayRef(localFrames) GetNumLocalFrames() { return localFrames.count; }
	disabledFunction(noLocalFrames, const int) ES_FORCEINLINE GetNumLocalFrames() const { return 0; }

	enablePtrPairArg(bones, const char*) GetBoneName(char *masterBuffer, int id) const { return bones.GetData(masterBuffer)[id].GetData(masterBuffer)->name.GetData(masterBuffer); }
	enablehkArrayArg(bones, const char*) GetBoneName(char *masterBuffer, int id) const { return bones.GetData(masterBuffer)[id].name.GetData(masterBuffer); }

	ES_FORCEINLINE const hkQTransform *GetBoneTM(char *masterBuffer, int id) const { return &tranforms.GetData(masterBuffer)[id]; }
	ES_FORCEINLINE const short GetBoneParentID(char *masterBuffer, int id) const { return parentIndicies.GetData(masterBuffer)[id]; }
	ES_FORCEINLINE const char *GetSkeletonName(char *masterBuffer) const { return Name.GetData(masterBuffer); };
	ES_FORCEINLINE const char *GetFloatSlot(char *masterBuffer, int id) const { return floatSlots.GetData(masterBuffer)[id].GetData(masterBuffer); };
	
	enabledFunction(noLocalFrames, const hkLocalFrameOnBone)ES_FORCEINLINE GetLocalFrame(char *masterBuffer, int id) const { return hkLocalFrameOnBone{ localFrames.GetData(masterBuffer)[id].localFrame.GetData(masterBuffer) ,localFrames.GetData(masterBuffer)[id].boneIndex }; };
	disabledFunction(noLocalFrames, const hkLocalFrameOnBone)ES_FORCEINLINE GetLocalFrame(char *masterBuffer, int id) const { return hkLocalFrameOnBone{}; };

	ADD_DISABLERS(_parent<_ipointer>, noPartitions, noReferenceFloats, noLocalFrames);

	disabledFunction(noPartitions, const int) ES_FORCEINLINE GetNumPartitions() const { return 0; }
	enabledFunction(noPartitions, const int) ES_FORCEINLINE GetNumPartitions() const { return GetNumPartitions(); }
	enabledFunction(noPartitions, int&) ES_FORCEINLINE GetNumPartitions() { return partitions.count; }

	disabledFunction(noReferenceFloats, const int) ES_FORCEINLINE GetNumReferenceFloats() const { return 0; }
	enabledFunction(noReferenceFloats, const int) ES_FORCEINLINE GetNumReferenceFloats() const { return referenceFloats.count; }

	disabledFunction(noPartitions, const hkaPartition) ES_FORCEINLINE GetPartition(char *masterBuffer, int id) const { return hkaPartition{}; }
	enabledFunction(noPartitions, const hkaPartition) ES_FORCEINLINE GetPartition(char *masterBuffer, int id) const { return hkaPartition{ partitions.GetData(masterBuffer)[id].Name.GetData(masterBuffer), partitions.GetData(masterBuffer)[id].startBoneIndex, partitions.GetData(masterBuffer)[id].numBones }; }

	disabledFunction(noReferenceFloats, const float) ES_FORCEINLINE GetReferenceFloat(char *masterBuffer, int id) const { return 0.0f; }
	enabledFunction(noReferenceFloats, const float) ES_FORCEINLINE GetReferenceFloat(char *masterBuffer, int id) const { return referenceFloats.GetData(masterBuffer)[id]; }


	disabledFunction(noLocalFrames, void) ES_FORCEINLINE SwapLocalFrames(char *masterBuffer){}
	enabledFunction(noLocalFrames, void) ES_FORCEINLINE SwapLocalFrames(char *masterBuffer)
	{
		FByteswapper(GetNumLocalFrames());
		const int numLocalFrames = GetNumLocalFrames();

		for (int i = 0; i < numLocalFrames; i++)
			FByteswapper(localFrames.GetData(masterBuffer)[i].boneIndex);
	}
	disabledFunction(noReferenceFloats, void) ES_FORCEINLINE SwapRefFloats(char *masterBuffer) {}
	enabledFunction(noReferenceFloats, void) ES_FORCEINLINE SwapRefFloats(char *masterBuffer)
	{
		FByteswapper(GetNumFloatSlots());
		const int numRefFloats = GetNumFloatSlots();

		for (int i = 0; i < numRefFloats; i++)
			FByteswapper(referenceFloats.GetData(masterBuffer)[i]);
	}
	disabledFunction(noPartitions, void) ES_FORCEINLINE SwapPartitions(char *masterBuffer) {}
	enabledFunction(noPartitions, void) ES_FORCEINLINE SwapPartitions(char *masterBuffer)
	{
		FByteswapper(GetNumPartitions());
		const int numPartitions = GetNumPartitions();

		for (int i = 0; i < numPartitions; i++)
		{
			FByteswapper(partitions.GetData(masterBuffer)[i].startBoneIndex);
			FByteswapper(partitions.GetData(masterBuffer)[i].numBones);
		}
	}

	ES_FORCEINLINE void SwapEndian(char *masterBuffer)
	{
		FByteswapper(GetNumBones());	
		const int numBones = GetNumBones();

		for (int i = 0; i < numBones; i++)
			FByteswapper(parentIndicies.GetData(masterBuffer)[i]);

		for (int i = 0; i < numBones; i++)
		{
			tranforms.GetData(masterBuffer)[i].position.SwapEndian();
			tranforms.GetData(masterBuffer)[i].rotation.SwapEndian();
			tranforms.GetData(masterBuffer)[i].scale.SwapEndian();
		}
		
		SwapLocalFrames(masterBuffer);
		SwapRefFloats(masterBuffer);
		SwapPartitions(masterBuffer);
	}
};

template<template<class C>class _ipointer>struct hkaSkeleton550_t_sharedData
{
	_ipointer<char> Name;
	_ipointer<short> parentIndicies;
	int numParentIndicies;
	_ipointer<_ipointer<hkaBone_t<_ipointer>>> bones;
	int numBones;
	_ipointer<hkQTransform> tranforms;
	int numTranforms;
	_ipointer<_ipointer<char>> floatSlots;
	int numFloatSlots;

	void noPartitions();
	void noReferenceFloats();
	void noLocalFrames();
};

template<template<class C>class _ipointer>struct hkaSkeleton660_t_sharedData
{
	_ipointer<char> Name;
	_ipointer<short> parentIndicies;
	int numParentIndicies;
	_ipointer<_ipointer<hkaBone_t<_ipointer>>> bones;
	int numBones;
	_ipointer<hkQTransform> tranforms;
	int numTranforms;
	_ipointer<_ipointer<char>> floatSlots;
	int numFloatSlots;
	_ipointer<hkLocalFrameOnBone_t<_ipointer>> localFrames;
	int numLocalFrames;

	void noPartitions();
	void noReferenceFloats();
};

template<template<class C>class _ipointer>using hkaSkeleton550_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton550_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton550_rp_t : hkaSkeleton550_t<_ipointer> {};

template<template<class C>class _ipointer>using hkaSkeleton660_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton660_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton660_rp_t : hkaSkeleton660_t<_ipointer> {};

template<template<class C>class _ipointer>struct hkaSkeleton710_t_sharedData : hkReferenceObject<_ipointer>
{
	_ipointer<char> Name;
	hkArray<short, _ipointer> parentIndicies;
	hkArray<hkaBone_t<_ipointer>, _ipointer> bones;
	hkArray<hkQTransform, _ipointer> tranforms;
	hkArray<_ipointer<char>, _ipointer> floatSlots;
	hkArray<hkLocalFrameOnBone_t<_ipointer>, _ipointer> localFrames;

	void noPartitions();
	void noReferenceFloats();
};

template<template<class C>class _ipointer>using hkaSkeleton710_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton710_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton710_rp_t : hkaSkeleton710_t<_ipointer> {};

template<template<class C>class _ipointer>struct hkaSkeleton2010_t_sharedData : hkReferenceObject<_ipointer>
{
	_ipointer<char> Name;
	hkArray<short, _ipointer> parentIndicies;
	hkArray<hkaBone_t<_ipointer>, _ipointer> bones;
	hkArray<hkQTransform, _ipointer> tranforms;
	hkArray<float, _ipointer> referenceFloats;
	hkArray<_ipointer<char>, _ipointer> floatSlots;
	hkArray<hkLocalFrameOnBone_t<_ipointer>, _ipointer> localFrames;

	void noPartitions();
};

template<template<class C>class _ipointer>using hkaSkeleton2010_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton2010_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton2010_rp_t : hkaSkeleton2010_t<_ipointer> {};

template<template<class C>class _ipointer>struct hkaSkeleton2011_t_sharedData : hkReferenceObject<_ipointer>
{
	_ipointer<char> Name;
	hkArray<short, _ipointer> parentIndicies;
	hkArray<hkaBone_t<_ipointer>, _ipointer> bones;
	hkArray<hkQTransform, _ipointer> tranforms;
	hkArray<float, _ipointer> referenceFloats;
	hkArray<_ipointer<char>, _ipointer> floatSlots;
	hkArray<hkLocalFrameOnBone2_t<_ipointer>, _ipointer> localFrames;

	void noPartitions();
};

template<template<class C>class _ipointer>using hkaSkeleton2011_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton2011_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton2011_rp_t : hkaSkeleton2011_t<_ipointer> {};

template<template<class C>class _ipointer>struct hkaSkeleton2012_t_sharedData : hkReferenceObject<_ipointer>
{
	_ipointer<char> Name;
	hkArray<short, _ipointer> parentIndicies;
	hkArray<hkaBone_t<_ipointer>, _ipointer> bones;
	hkArray<hkQTransform, _ipointer> tranforms;
	hkArray<float, _ipointer> referenceFloats;
	hkArray<_ipointer<char>, _ipointer> floatSlots;
	hkArray<hkLocalFrameOnBone2_t<_ipointer>, _ipointer> localFrames;
	hkArray<hkaPartition_t<_ipointer>, _ipointer> partitions;

};

template<template<class C>class _ipointer>using hkaSkeleton2012_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton2012_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton2012_rp_t : hkaSkeleton2012_t<_ipointer> {};
template<template<class C>class _ipointer>struct hkaSkeleton2013_t : hkaSkeleton2012_t<_ipointer> {};
template<template<class C>class _ipointer>struct hkaSkeleton2013_rp_t : hkaSkeleton2012_t<_ipointer> {};
template<template<class C>class _ipointer>struct hkaSkeleton2014_t : hkaSkeleton2012_t<_ipointer> {};
template<template<class C>class _ipointer>struct hkaSkeleton2014_rp_t : hkaSkeleton2012_t<_ipointer> {};

template<template<class C>class _ipointer>struct hkaSkeleton2016_t_sharedData : hkReferenceObject2016<_ipointer>
{
	_ipointer<char> Name;
	hkArray<short, _ipointer> parentIndicies;
	hkArray<hkaBone_t<_ipointer>, _ipointer> bones;
	hkArray<hkQTransform, _ipointer> tranforms;
	hkArray<float, _ipointer> referenceFloats;
	hkArray<_ipointer<char>, _ipointer> floatSlots;
	hkArray<hkLocalFrameOnBone2_t<_ipointer>, _ipointer> localFrames;
	hkArray<hkaPartition_t<_ipointer>, _ipointer> partitions;

};


template<template<class C>class _ipointer>using hkaSkeleton2016_t = hkaSkeleton_t_shared<_ipointer, hkaSkeleton2016_t_sharedData>;
template<template<class C>class _ipointer>struct hkaSkeleton2016_rp_t : hkaSkeleton2016_t<_ipointer> {};