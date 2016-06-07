/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This generated file is licensed to you under the terms described in the
 * rpcdds_LICENSE file included in this rpcdds distribution.
 *
 *************************************************************************
 * 
 * @file CalculatorExtension.h
 * This header file contains the declaration of user exceptions in Calculator.idl
 *
 * This file was generated by the tool rpcddsgen.
 */

#ifndef _CalculatorEXTENSION_H_
#define _CalculatorEXTENSION_H_

#include <rpcdds/exceptions/UserException.h>
#include "CalculatorSupport.h"


#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif
#else
#define eProsima_user_DllExport
#endif

namespace Calculator
{


}

class RPCUSERDllExport CalculatorExt 
{
    public:
        
                    virtual DDS_Long addition(/*in*/ DDS_Long value1, /*in*/ DDS_Long value2) = 0;
                
                    virtual DDS_Long subtraction(/*in*/ DDS_Long value1, /*in*/ DDS_Long value2) = 0;
                
};

#endif // _CalculatorEXTENSION_H_