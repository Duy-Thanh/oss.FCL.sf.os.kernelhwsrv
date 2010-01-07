// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of the License "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "sl_std.h"

//-----------------------------------------------------------------------------
/**
    Removes trailing dots from aName.
    @return new string descriptor that may have its length adjusted
*/
TPtrC RemoveTrailingDots(const TDesC& aName)
{
    TInt len = aName.Length();
    
    while(len > 0)
    {
        if(aName[len-1] == '.')
            len--;
        else
            break;
    }

    TPtrC ptrNoDots(aName.Ptr(), len);
    return ptrNoDots;
}


TUint32 Log2(TUint32 aVal)
{
    return Log2_inline(aVal);
}


TTime DosTimeToTTime(TInt aDosTime,TInt aDosDate)
//
//	Deciphers the dos time/date entry information and converts to TTime
//
	{
	TInt secMask=0x1F;
	TInt minMask=0x07E0;
	TInt hrMask=0xF800;
	TInt dayMask=0x1F;
	TInt monthMask=0x01E0;
	TInt yearMask=0xFE00;

	TInt secs=(aDosTime&secMask)*2;
	TInt mins=(aDosTime&minMask)>>5;
	TInt hrs=(aDosTime&hrMask)>>11;
	TInt days=(aDosDate&dayMask)-1;
	TMonth months=(TMonth)(((aDosDate&monthMask)>>5)-1);
	TInt years=((aDosDate&yearMask)>>9)+1980;
	
	TDateTime datetime;
	TInt ret=datetime.Set(years,months,days,hrs,mins,secs,0);
	if (ret==KErrNone)
		return(TTime(datetime));
	return(TTime(0));
	}

TInt DosTimeFromTTime(const TTime& aTime)
//
// Converts a TTime to a dos time
//
	{
	TDateTime dateTime=aTime.DateTime();
	TInt dosSecs=dateTime.Second()/2;
	TInt dosMins=dateTime.Minute()<<5;
	TInt dosHrs=dateTime.Hour()<<11;
	return dosSecs|dosMins|dosHrs;
	}

TInt DosDateFromTTime(const TTime& aTime)
//
// Converts a TTime to a dos date
//
	{

	TDateTime dateTime=aTime.DateTime();
	TInt dosDays=dateTime.Day()+1;
	TInt dosMonths=(dateTime.Month()+1)<<5;
	TInt dosYears=(dateTime.Year()-1980)<<9;
	return dosDays|dosMonths|dosYears;
	}

TBuf8<12> DosNameToStdFormat(const TDesC8& aDosName)
//
// Converts xxx.yyy to standard format aaaaaaaayyy
//
	{

	__ASSERT_DEBUG(aDosName.Length()>=0 && aDosName.Length()<=12,Fault(EFatBadDosFormatName));
	TBuf8<12> result;
	Mem::Fill((TUint8*)result.Ptr(),result.MaxSize(),' ');
	TInt dotPos=aDosName.Locate('.');
	if (dotPos==KErrNotFound)
		{
		result=aDosName;
		result.SetLength(11);
		return result;
		}
	result=aDosName.Left(dotPos);
	result.SetLength(11);
	TPtr8 ext(&result[8],3);
	ext=aDosName.Right(aDosName.Length()-dotPos-1);
	return result;
	}

TBuf8<12> DosNameFromStdFormat(const TDesC8& aStdFormatName)
//
// Converts aaaaaaaayyy to dos name format xxx.yyy
//
	{

	__ASSERT_DEBUG(aStdFormatName.Length()==11,Fault(EFatBadStdFormatName));
	TBuf8<12> result;
	TInt nameLen=aStdFormatName.Locate(' ');
	if (nameLen>8 || nameLen==KErrNotFound)
		nameLen=8;
	result=aStdFormatName.Left(nameLen);
	TPtrC8 ext(&aStdFormatName[8],3);
	TInt extLen=ext.Locate(' ');
	if (extLen)
		result.Append(TChar('.'));
	if (extLen==KErrNotFound)
		extLen=3;
	result.Append(ext.Left(extLen));
    if(result.Length() && result[0]==0x05 )
	    {
	    result[0]=0xE5;
	    }
	return result;
	}

TInt NumberOfVFatEntries(TInt aNameLength)
//
// Return the number of VFat entries required to describe a filename of length aNameLength
//
	{
	TInt numberOfEntries=0;
	if (aNameLength%KMaxVFatEntryName)
		aNameLength++;	//	Include a zero terminator
//	If aNameLength is a exact multiple of KMaxVFatEntryName, don't bother
//	with a zero terminator - it just adds an unnecessary directory entry		
	
	numberOfEntries=(1+(aNameLength/KMaxVFatEntryName));	
	
	if (aNameLength%KMaxVFatEntryName)
		numberOfEntries++;
	
	return(numberOfEntries);
	}

//-----------------------------------------------------------------------------
/** 
    Calculate DOS short name checksum
    @param aShortName short name descriptor (must be at least 11 bytes long)
    @return checksum
*/
TUint8 CalculateShortNameCheckSum(const TDesC8& aShortName)
    {

    ASSERT(aShortName.Length() >= KFatDirNameSize);
    const TUint8* pName = aShortName.Ptr();

    const TUint32 w0 = ((const TUint32*)pName)[0];
    const TUint32 w1 = ((const TUint32*)pName)[1];

    TUint32 chkSum = w0 & 0xFF;
    
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ((w0 << 16) >> 24));
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ((w0 << 8)  >> 24));
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ( w0 >> 24));

    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + (w1) & 0xFF);
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ((w1 << 16) >> 24));
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ((w1 << 8)  >> 24));
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + ( w1 >> 24));

    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + pName[8]);
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + pName[9]);
    chkSum = (TUint8)(((chkSum<<7) | (chkSum>>1)) + pName[10]);

    return (TUint8)chkSum;
    }

//-----------------------------------------------------------------------------

const TUint32 K_FFFF = 0xFFFFFFFF; //-- all one bits, beware rigth shifts of signed integers!


RBitVector::RBitVector()
          :iNumBits(0), ipData(NULL), iNumWords(0)
    {
    }


RBitVector::~RBitVector()
    {
    Close();
    }

/**
    Panics.
    @param aPanicCode   a panic code
*/
void RBitVector::Panic(TPanicCode aPanicCode) const
    {
    _LIT(KPanicCat,"RBitVector");
    User::Panic(KPanicCat, aPanicCode);
    }

/** explicitly closes the object and deallocates memory */
void RBitVector::Close()
    {
    iNumBits = 0;
    iNumWords =0;
    User::Free(ipData);
    ipData = NULL;
    }

//-----------------------------------------------------------------------------

/**
    Comparison perator.
    @param  aRhs a vector to compate with.
    @panic ESizeMismatch in the case of different vector sizes
*/
TBool RBitVector::operator==(const RBitVector& aRhs) const
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    __ASSERT_ALWAYS(iNumBits == aRhs.iNumBits, Panic(ESizeMismatch));


    if(!iNumBits)
        return ETrue; //-- comparing 0-lenght arrays

    if(this == &aRhs)
        return ETrue; //-- comparing with itself

    if(iNumWords >= 1)
        {
        const TUint32 cntBytes = (iNumBits >> 5) << 2; //-- bytes to compare
        if(memcompare((const TUint8*)ipData, cntBytes, (const TUint8*)aRhs.ipData, cntBytes))
            return EFalse;
        }

    const TUint32 bitsRest  = iNumBits & 0x1F;
    if(bitsRest)
        {
        const TUint32 mask = K_FFFF >> (32-bitsRest);
        return ( (ipData[iNumWords-1] & mask) == (aRhs.ipData[iNumWords-1] & mask) );
        }
    
    return ETrue;
    }

TBool RBitVector::operator!=(const RBitVector& aRhs) const  
    {
    return ! ((*this) == aRhs);
    } 

//-----------------------------------------------------------------------------

/** The same as Create(), but leaves on error */
void RBitVector::CreateL(TUint32 aNumBits)
    {
    User::LeaveIfError(Create(aNumBits));
    }


/**
    Create the vector with the size of aNumBits bits.
    @return system-wide error codes:
        KErrNoMemory    unable to allocate sufficient amount of memory for the array
        KErrInUse       an attempt to call Create() for non-empty vector. Close it first.
        KErrArgument    invalid aNumBits value == 0
*/
TInt RBitVector::Create(TUint32 aNumBits)
    {

    if(ipData)
        return KErrInUse; //-- array is already in use. Close it first.

    if(!aNumBits)
        return KErrArgument;

    //-- memory is allocated by word (32 bit) quiantities
    const TUint32 numWords = (aNumBits >> 5) + ((aNumBits & 0x1F) > 0 ? 1:0);
    ipData = (TUint32*)User::AllocZ(numWords << 2);

    if(!ipData)
        return KErrNoMemory;

    iNumBits  = aNumBits;
    iNumWords = numWords;

    return KErrNone;
    }


/**
    Fill a bit vector with a given bit value
    @param aVal a bit value
*/
void RBitVector::Fill(TBool aVal)
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    memset(ipData, (aVal ? 0xFF : 0x00), iNumWords << 2);
    }

/** Invert all bits in a bit vector */
void RBitVector::Invert()
{
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    for(TUint32 i=0; i<iNumWords; ++i)
        ipData[i] ^= K_FFFF;
}


/**
    Perform "And" operation between 2 vectors. They shall be the same size.
    @param  aRhs a vector from the right hand side
    @panic ESizeMismatch in the case of different vector sizes
*/
void RBitVector::And(const RBitVector& aRhs)
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    __ASSERT_ALWAYS(iNumBits == aRhs.iNumBits, Panic(ESizeMismatch));
    for(TUint32 i=0; i<iNumWords; ++i)
        {
        ipData[i] &= aRhs.ipData[i];
        }
    }

/**
    Perform "Or" operation between 2 vectors. They shall be the same size.    
    @param  aRhs a vector from the right hand side
    @panic ESizeMismatch in the case of different vector sizes
*/
void RBitVector::Or(const RBitVector& aRhs)
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    __ASSERT_ALWAYS(iNumBits == aRhs.iNumBits, Panic(ESizeMismatch));
    for(TUint32 i=0; i<iNumWords; ++i)
        {
        ipData[i] |= aRhs.ipData[i];
        }
    }

/**
    Perform "Xor" operation between 2 vectors. They shall be the same size.    
    @param  aRhs a vector from the right hand side
    @panic ESizeMismatch in the case of different vector sizes
*/
void RBitVector::Xor(const RBitVector& aRhs)
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    __ASSERT_ALWAYS(iNumBits == aRhs.iNumBits, Panic(ESizeMismatch));
    for(TUint32 i=0; i<iNumWords; ++i)
        {
        ipData[i] ^= aRhs.ipData[i];
        }
    }

//-----------------------------------------------------------------------------
/**
    Fill a range from bit number "aIndexFrom" to "aIndexTo" inclusively with the value of aVal
    
    @param  aIndexFrom  start bit number (inclusive)
    @param  aIndexTo    end bit number (inclusive)
    @param  aVal        the value to be used to fill the range (0s or 1s)
*/
void RBitVector::Fill(TUint32 aIndexFrom, TUint32 aIndexTo, TBool aVal)
    {
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));

    //-- swap indexes if they are not in order
    if(aIndexFrom > aIndexTo)
        {
        const TUint32 tmp = aIndexFrom;
        aIndexFrom = aIndexTo;
        aIndexTo = tmp;
        }

    __ASSERT_ALWAYS((aIndexFrom < iNumBits) && (aIndexTo < iNumBits), Panic(EIndexOutOfRange));

    const TUint32 wordStart = WordNum(aIndexFrom);
    const TUint32 wordTo    = WordNum(aIndexTo);

    if(aVal)
        {//-- filling a range with '1'
        
        TUint32 shift = BitInWord(aIndexFrom);
        const TUint32 mask1 = (K_FFFF >> shift) << shift;

        TUint32 mask2 = K_FFFF;
        shift = 1+BitInWord(aIndexTo);
        if(shift < 32)
            {
            mask2 = ~((mask2 >> shift) << shift);
            }

        if(wordTo == wordStart)
            {//-- a special case, filling is in the same word
            ipData[wordStart] |= (mask1 & mask2);
            }
        else
            {
            ipData[wordStart] |= mask1; 
            ipData[wordTo]    |= mask2;
            
            const TUint32 wholeWordsBetween = wordTo - wordStart - 1; //-- whole words that can be bulk filled

            if(wholeWordsBetween)
                memset(ipData+wordStart+1, 0xFF, wholeWordsBetween << 2);
                            
            }
        }
    else
        {//-- filling a range with '0'
        
        //-- if you need this functionality, remove the panic and uncomment the code below.

        Panic(ENotImplemented);
        
        /*
        TUint32 shift = BitInWord(aIndexFrom);
        const TUint32 mask1 = ~((K_FFFF >> shift) << shift);

        TUint32 mask2 = 0;
        shift = 1+BitInWord(aIndexTo);
        if(shift < 32)
            {
            mask2 = ((K_FFFF >> shift) << shift);
            }

        if(wordTo == wordStart)
            {//-- a special case, filling is in the same word
            ipData[wordStart] &= (mask1 | mask2);
            }
        else
            {
            ipData[wordStart] &= mask1; 
            ipData[wordTo]    &= mask2;
            
            const TUint32 wholeWordsBetween = wordTo - wordStart - 1; //-- whole words that can be bulk filled

            if(wholeWordsBetween)
                memset(ipData+wordStart+1, 0x00, wholeWordsBetween << 2);
                            
            }
        */
        }

    }

//-----------------------------------------------------------------------------

/**
    Search for a specified bit value ('0' or '1') in the vector from the given position.
    @param  aStartPos   zero-based index; from this position the search will start. This position isn't included to the search.
                        On return may contain a new position if the specified bit is found in specified direction.
    @param  aBitVal     zero or non-zero bit to search.
    @param  aDir        Specifies the search direction

    @return ETrue if the specified bit value is found; aStartPos gets updated.
            EFalse otherwise.

*/
TBool RBitVector::Find(TUint32& aStartPos, TBool aBitVal, TFindDirection aDir) const
    {
    __ASSERT_ALWAYS(aStartPos < iNumBits, Panic(EIndexOutOfRange));
    ASSERT(iNumWords && ipData);

    switch(aDir)
        {
        case ERight:    //-- Search from the given position to the right
            return FindToRight(aStartPos, aBitVal);

        case ELeft:     //-- Search from the given position to the left (towards lower index)
            return FindToLeft(aStartPos, aBitVal);

        case ENearestL: //-- Search for the nearest value in both directions starting from left
            return FindNearest(aStartPos, aBitVal, ETrue);

        case ENearestR: //-- Search for the nearest value in both directions starting from right
            return FindNearest(aStartPos, aBitVal, EFalse);

        default:
            Panic(EWrondFindDirection);
            return EFalse;

        };
    
    }

//-----------------------------------------------------------------------------
/**
    Internal method to look for a given bit value in the right direction.
    see TBool RBitVector::Find(...)
*/
TBool RBitVector::FindToRight(TUint32& aStartPos, TBool aBitVal) const
    {
    if(aStartPos >= iNumBits-1)
        return EFalse; //-- no way to the right

    const TUint32 startPos = aStartPos+1;
    const TUint32 fInvert = aBitVal ? 0 : K_FFFF; //-- invert everything if we are looking for '0' bit

    TUint32 wordNum = WordNum(startPos);
    TUint32 val = ipData[wordNum] ^ fInvert;

    if(wordNum == iNumWords-1)
        {//-- process the last word in the array, some higher bits might not belong to the bit vector
        val = MaskLastWord(val);
        }

    const TUint32 shift = BitInWord(startPos);
    val = (val >> shift) << shift; //-- mask unused low bits

    if(val)
        {//-- there are '1' bits in the current word
        goto found;
        }
    else
        {//-- search in higher words
        wordNum++;

        while(iNumWords-wordNum > 1)
            {
            val = ipData[wordNum] ^ fInvert;
            if(val)
                goto found;

            wordNum++;
            }

        if(wordNum == iNumWords-1)
            {//-- process the last word in the array, some higher bith might not belong to the bit vector
            val = ipData[wordNum] ^ fInvert;
            val = MaskLastWord(val);

            if(val)
                goto found;
            }
        }

    return EFalse; //-- haven't found anything

  found:

    val &= (~val+1); //-- select rightmost bit
    aStartPos = (wordNum << 5)+Log2(val);
    return ETrue;
    }


//-----------------------------------------------------------------------------

/**
    Internal method to look for a given bit value in the left direction.
    see TBool RBitVector::Find(...)
*/
TBool RBitVector::FindToLeft(TUint32& aStartPos, TBool aBitVal) const
{
    if(!aStartPos)
        return EFalse; //-- no way to the left
    
    const TUint32 startPos=aStartPos-1;
    const TUint32 fInvert = aBitVal ? 0 : K_FFFF; //-- invert everything if we are looking for '0' bit

    TUint32 wordNum = WordNum(startPos);
    TUint32 val = ipData[wordNum] ^ fInvert;

    const TUint32 shift = 31-(BitInWord(startPos));
    val = (val << shift) >> shift; //-- mask unused high bits

    if(val)
    {//-- there are '1' bits in the current word
        goto found;
    }
    else
    {//-- search in the lower words
        while(wordNum)
        {
            wordNum--;
            val=ipData[wordNum] ^ fInvert;
            if(val)
                goto found;
        }
    }

    return EFalse; //-- nothing found

 found:
    aStartPos = (wordNum << 5)+Log2(val);
    return ETrue;
}

//-----------------------------------------------------------------------------

/**
    Internal method to look for a given bit value in the both directions.
    see TBool RBitVector::Find(...)
*/
TBool RBitVector::FindNearest(TUint32& aStartPos, TBool aBitVal, TBool aToLeft) const
{
    if(iNumBits < 2)
        return EFalse;

    if(aStartPos == 0)
        return FindToRight(aStartPos, aBitVal);

    if(aStartPos == iNumBits-1)
        return FindToLeft(aStartPos, aBitVal);

    
    const TUint32 fInvert = aBitVal ? 0 : K_FFFF; //-- invert everything if we are looking for '0' bit
    
    TUint32 wordNum = WordNum(aStartPos);
    TUint32 l_Idx; //-- index of the word to the left
    TUint32 r_Idx; //-- index of the word to the right
    
    l_Idx = r_Idx = wordNum;

    TBool   noWayLeft  = (wordNum == 0);            //-- if we are in the first word
    TBool   noWayRight = (wordNum == iNumWords-1);  //-- if we are in the last word

    //-- look in the current word first
    TUint32 val = ipData[wordNum] ^ fInvert;
    
    if(noWayRight)
    {   //-- this is the last word in the array, mask unused high bits in the last word
        val = MaskLastWord(val);
    }

    const TUint32 bitPos = aStartPos & 0x1F;
    val &= ~(1<<bitPos); //-- mask the bit at current position
    
    if(val == 0)
    {//-- no '1' bits in the current word
        noWayLeft  = ItrLeft(l_Idx);
        noWayRight = ItrRight(r_Idx);
    }
    else if(bitPos == 0)
    {
        noWayLeft = ItrLeft(l_Idx); //-- move to the previous word
    }
    else if(bitPos == 31)
    {
        noWayRight = ItrRight(r_Idx); //-- move to the next word
    }
    else
    {//-- look in the current word, in both halves to the left and right from the start position
        
        const TUint32 shift1 = 32-bitPos;
        const TUint32 partLo = (val << shift1) >> shift1; //-- towards lower bits

        const TUint32 shift2 = bitPos+1;
        const TUint32 partHi = (val >> shift2) << shift2; //-- towards higher bits 
        

        if(partLo && !partHi) //-- only lower part has '1' bits   
        {
            aStartPos = (wordNum << 5)+Log2(partLo);
            return ETrue;
        }
        else if(!partLo && partHi) //-- only higher part has '1' bits
        {
            aStartPos = (wordNum << 5)+Log2( (partHi & (~partHi+1)) );
            return ETrue;
        }
        else if(partLo && partHi) //-- both parts contain '1' bits, select the nearest one
        {
            const TUint32 posL = (wordNum << 5)+Log2(partLo);
            const TUint32 posR = (wordNum << 5)+Log2( (partHi & (~partHi+1)) );
        
            ASSERT(aStartPos > posL);
            ASSERT(posR > aStartPos);
            const TUint32 distL = aStartPos-posL;
            const TUint32 distR = posR-aStartPos;

            if(distL < distR)
            {
                aStartPos = posL;
                return ETrue;
            }
            else if(distL > distR)
            {
                aStartPos = posR;
                return ETrue;
            }
            else
            {//-- distL == distR, take into account search priority
                aStartPos = aToLeft ? posL : posR;
                return ETrue;
            }
        }
        else //-- (!partLo && !partHi), nothing in the current word
        {
            ASSERT(0);
        }

    }// if(bitPos > 0 && bitPos < 31)

    //-- now we are processing separate words from both sides of the search position
    for(;;)
    { 
        TUint32 wL = ipData[l_Idx] ^ fInvert;
        TUint32 wR = ipData[r_Idx] ^ fInvert;
        if(r_Idx == iNumWords-1)
        {   //-- this is the last word in the array, mask unused high bits in the last word
            wR = MaskLastWord(wR);
        }

        if(wL && !wR)
        {
            aStartPos = (l_Idx << 5)+Log2(wL);
            return ETrue;
        }
        else if(!wL && wR)
        {
            aStartPos = (r_Idx << 5)+Log2( (wR & (~wR+1)) );
            return ETrue;
        }
        else if(wL && wR)
        {
            const TUint32 posL = (l_Idx << 5)+Log2(wL);
            const TUint32 posR = (r_Idx << 5)+Log2( (wR & (~wR+1)) );
        
            ASSERT(aStartPos > posL);
            ASSERT(posR > aStartPos);
            const TUint32 distL = aStartPos-posL;
            const TUint32 distR = posR-aStartPos;

            if(distL < distR)
            {
                aStartPos = posL;
                return ETrue;
            }
            else if(distL > distR)
            {
                aStartPos = posR;
                return ETrue;
            }
            else
            {//-- distL == distR, take into account search priority
                aStartPos = aToLeft ? posL : posR;
                return ETrue;
            }

        }//else if(wL && wR)


        if(noWayLeft)
        {
            aStartPos = r_Idx << 5;
            return FindToRight(aStartPos, aBitVal);
        }
        else
        {
            noWayLeft  = ItrLeft(l_Idx);
        }

        if(noWayRight)
        {
            aStartPos = l_Idx << 5;
            return FindToLeft(aStartPos, aBitVal);
        }
        else
        {    
            noWayRight = ItrRight(r_Idx);
        }

   }//for(;;)

    //return EFalse;
}

//-----------------------------------------------------------------------------
/**
    Find out if two vectors are different.

    @param  aRhs        vector to compare with
    @param  aDiffIndex  if there is a differene, here will be the number of the first different bit
    @return ETrue if vectors differ, EFalse, if they are identical.
*/
TBool RBitVector::Diff(const RBitVector& aRhs, TUint32& aDiffIndex) const
{
    __ASSERT_ALWAYS(ipData, Panic(ENotInitialised));
    __ASSERT_ALWAYS(iNumBits == aRhs.iNumBits, Panic(ESizeMismatch));
    ASSERT(iNumWords > 0);

    TUint32 diffWord=0;
    TUint32 wordNum=0;

    //-- compare all but the last word in the array
    for(wordNum=0; wordNum < iNumWords-1; ++wordNum)
    {
        diffWord = ipData[wordNum] ^ aRhs.ipData[wordNum];
        if(diffWord)
            break;  //-- found difference
    }

    //-- process the last word in the array
    if(!diffWord)
    {
        diffWord = MaskLastWord(ipData[wordNum]) ^ MaskLastWord(aRhs.ipData[wordNum]);
    }

    if(!diffWord)
        return EFalse; //-- vectors are the same

    //-- calculate the position of the bit that different.
    diffWord &= (~diffWord+1); //-- select rightmost bit
    aDiffIndex = (wordNum << 5)+Log2(diffWord);
    
    return ETrue;
}
//-----------------------------------------------------------------------------

/**
    Iterate to the left (towards lower index) in the array of words ipData

    @param  aIdx index within ipData array to be decremented; if it's possible to move left, it will be decreased
    @return ETrue if there is no way left i.e. aIdx is 0. EFalse otherwise and aIdx decreased.
*/
TBool RBitVector::ItrLeft(TUint32& aIdx) const
{
    if(aIdx == 0)
        return ETrue;
    else
    {
        aIdx--;
        return EFalse;
    }
}


/**
    Iterate to the right (towards higher index) in the array of words ipData

    @param  aIdx index within ipData array to be incremented; if it's possible to move right, it will be increased
    @return ETrue if there is no way right i.e. aIdx corresponds to the last word. EFalse otherwise and aIdx increased.
*/
TBool RBitVector::ItrRight(TUint32& aIdx) const
{
    if(aIdx < iNumWords-1)
    {
        aIdx++;
        return EFalse;
    }
    else
        return ETrue;
}







