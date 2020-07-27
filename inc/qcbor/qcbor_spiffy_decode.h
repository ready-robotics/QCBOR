/*==============================================================================
qcbor_spiffy_decode.h -- higher-level easier-to-use CBOR decoding.

Copyright (c) 2020, Laurence Lundblade. All rights reserved.

SPDX-License-Identifier: BSD-3-Clause

See BSD-3-Clause license in README.md

Created on 7/23/18
=============================================================================*/



#ifndef qcbor_spiffy_decode_h
#define qcbor_spiffy_decode_h


#include "qcbor/qcbor_decode.h"


#ifdef __cplusplus
extern "C" {
#if 0
} // Keep editor indention formatting happy
#endif
#endif


/**
@file qcbor_spiffy_decode.h

Q C B O R    D e c o d e

 This section just discusses decoding assuming familiarity with the general
 description of this encoder / decoder in section XXX.
 
 Encoded CBOR can be viewed to have a tree structure
 where the lead nodes are non-aggregate types like
 integers and strings and the intermediate nodes are
 either arrays or maps. Fundamentally, all decoding
 is a pre-order traversal of the tree. Calling
 GetNext() repeatedly will perform this.
 
 This pre-order traversal gives natural decoding of
 arrays where the array members are taken
 in order, but does not give natural decoding of
 maps where access by label is usually preferred.
 Using the EnterMap and GetByLabel methods,
 map items can be accessed by label. EnterMap
narrows decoding to a particular map. GetByLabel
 allows decoding the item of a particular label in
 the particular map. This can be used with nested
 maps by calling EnterMapByLabel.
 
 When EnterMap is called, pre-order traversal
 continues to work. There is a cursor that is run
 over the tree with calls to GetNext. This can be
 intermixed with calls to GetByLabel. The pre-order
 traversal is limited just to the map entered. Attempts
 to GetNext beyond the end of the map will give
 the HIT END error.
 
  There is also EnterArray to decode arrays. It will
 narrow the traversal to the extent of the array
 entered.
 
 GetByLabel supports duplicate label detection
 and will result in an error if the map has
 duplicate labels.
 
 GetByLabel is implemented by performing the
 pre-order traversal of the map to find the labeled
 item everytime it is called. It doesn't build up
 a hash table, a binary search tree or some other
 efficiently searchable structure internally. For simple
 trees this is fine and for high-speed CPUs this is
 fine, but for complex trees on slow CPUs,
 it may have performance issues (these have
 not be quantified yet). One way ease this is
 to use GetItems which allows decoding of
 a list of items expected in an map in one
 traveral.
 
 Like encoding, decoding maintains an
 internal error state. Once a call to the
 decoder returns an error, this error state
 is entered and subsequent decoder calls
 do nothing. This allows for prettier and cleaner
 decoding code. The only error check needed
 is in the Finish call. 
 
 An easy and clean way to use this decoder
 is to always use EnterMap and EnterArray
 for each array or map. They will error
 if the input CBOR is not the expected
 array or map.  Then use GetInt, GetString
 to get the individual items of of the
 maps and arrays making use of the
 internal error tracking provided by this
 decoder. The only error check needed
 is the call to Finish.
  
 In some CBOR protocols, the type of
 a data item may be variable. Maybe even
 the type of one data item is dependent
 on another. In such designs, GetNext has
 to be used and the internal error checking
 can't be relied upon.
 
 
*/


#define QCBOR_CONVERT_TYPE_INT64     0x01
#define QCBOR_CONVERT_TYPE_UINT64    0x02
#define QCBOR_CONVERT_TYPE_XINT64    0x80 // Type 0 or type 1
#define QCBOR_CONVERT_TYPE_FLOAT     0x04
#define QCBOR_CONVERT_TYPE_DOUBLE    0x40
#define QCBOR_CONVERT_TYPE_BIGFLOAT  0x08
#define QCBOR_CONVERT_TYPE_DECIMAL_FRACTION 0x10
#define QCBOR_CONVERT_TYPE_BIG_NUM  0x20


/* For protocols items that require explicit tags. The item must be explicitly tagged. */
#define QCBOR_TAGSPEC_MATCH_TAG 0
/** For protocol items that must NOT be tagged. The type is known implicitly from the labell, position or some other context. */
#define QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE 1
/** Either of the above two are allowed. This is highly discourged by the CBOR specification. One of the above to should be used instead. */
#define QCBOR_TAGSPEC_MATCH_EITHER 2


/*
 TODO: get rid of this

 int64_t
 uint64_t
 int32_t
 uint32_t
 int16_t
 uint16_t
 int8_t
 uint8_t

 8 types. 12 functions for each --> 96 functions

 7 converter functions

 foreach type
   for each conversion option
      for each fetch option



 */

/**
 @brief Decode next item as a signed 64-bit integer.

 @param[in] pCtx   The decode context
 @param[out] pnValue  64-bit integer with item

 On error, the decoder internal error state is set.

 The CBOR data item to decode must be a positive or negative integer (CBOR type 0 or 1). If not
 @ref QCBOR_ERR_UNEXPECTED_TYPE is set.

 CBOR can represent negative integers further from zero than can be represetned in
 an int64_t. @ref QCBOR_ERR_INT_OVERFLOW is set if such input is encountered.

 See also QCBORDecode_GetInt64Convert() and QCBORDecode_GetInt64ConvertAll().

 */
static void QCBORDecode_GetInt64(QCBORDecodeContext *pCtx, int64_t *pnValue);

static void QCBORDecode_GetInt64InMapN(QCBORDecodeContext *pCtx, int64_t nLabel, int64_t *pnValue);

static void QCBORDecode_GetInt64InMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, int64_t *pnValue);


/**
 @brief Decode next item as a signed 64-bit integer with basic conversions

 @param[in] pCtx   The decode context
 @param[in] uOptions The integer conversion options.
 @param[out] pnValue  64-bit integer with item

 The CBOR data item must be either a positive integer, negative integer or floating-point number.
 \c uOptions is one of XXX and controls which conversions will be performed.

 See also QCBORDecode_GetInt64ConvertAll() which will perform the same conversions
 as this and a lot more at the cost of adding more object code to your executable.

 On error, this sets the decoder last error.  If the data item is of a type that
 can't be decoded by this function, @ref QCBOR_ERR_UNEXPECTED_TYPE is set. If
 the data item can be decode, but the option requesting it is not set, then
 @ref QCBOR_ERR_UNEXPECTED_TYPE will be set. If the data item is too large
 or small to be represented as a 64-bit signed integer, @ref QCBOR_ERR_CONVERSION_UNDER_OVER_FLOW
 us set.

 When converting floating-point values, the integer is rounded to the nearest integer using
 llround(). By default, floating-point suport is enabled for QCBOR. If it is turned off,
 then floating-point conversion is not available and TODO: error will be set.
 
 */
static void QCBORDecode_GetInt64Convert(QCBORDecodeContext *pCtx, uint32_t uOptions, int64_t *pnValue);

static void QCBORDecode_GetInt64ConvertInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, int64_t *pnValue);

static void QCBORDecode_GetInt64ConvertInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, int64_t *pnValue);


/**
 @brief Decode next item as a signed 64-bit integer with conversions

 @param[in] pCtx   The decode context
 @param[in] uOptions The integer conversion options.
 @param[out] pnValue  64-bit integer with item

 This is the same as QCBORDecode_GetInt64Convert() but supports many more conversions at
 the cost of adding more object code to the executable.

 The additiona data item types that are suported are positive and negative bignums,
 decimal fractions and big floats, including decimal fractions and big floats that use bignums.
 Not that all these types can support numbers much larger that can be represented by
 in a 64-bit integer, so @ref QCBOR_ERR_CONVERSION_UNDER_OVER_FLOW may
 often be encountered.

 When converting bignums and decimal fractions @ref QCBOR_ERR_CONVERSION_UNDER_OVER_FLOW
 will be set if the result is below 1, unless the mantissa is zero, in which
 case the coversion is successful and the value of 0 is returned.
 */
void QCBORDecode_GetInt64ConvertAll(QCBORDecodeContext *pCtx, uint32_t uOptions, int64_t *pnValue);

void QCBORDecode_GetInt64ConvertAllInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, int64_t *pnValue);

void QCBORDecode_GetInt64ConvertAllInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, int64_t *pnValue);


/**
 @brief Decode next item as an unsigned 64-bit integer.

 @param[in] pCtx   The decode context
 @param[out] puValue  64-bit integer with item

 The sames as QCBORDecode_GetInt64(), but returns an unsigned integer and thus
 can only decode CBOR positive integers. @ref QCBOR_ERR_NUMBER_SIGN_CONVERSION
 is set if the input is a negative integer.

 See also QCBORDecode_GetUInt64Convert() and QCBORDecode_GetUInt64ConvertAll().
*/
static void QCBORDecode_GetUInt64(QCBORDecodeContext *pCtx, uint64_t *puValue);

static void QCBORDecode_GetUInt64InMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint64_t *puValue);

static void QCBORDecode_GetUInt64InMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint64_t *puValue);

/**
 @brief Decode next item as an unsigned 64-bit integer with basic conversions.

 @param[in] pCtx   The decode context
 @param[out] puValue  64-bit integer with item

 The sames as QCBORDecode_GetInt64Convert(), but returns an unsigned integer and thus
 sets @ref QCBOR_ERR_NUMBER_SIGN_CONVERSION
 is set if the value to be decoded is negatve.

 See also QCBORDecode_GetUInt64Convert() and QCBORDecode_GetUInt64ConvertAll().
*/
static void QCBORDecode_GetUInt64Convert(QCBORDecodeContext *pCtx, uint32_t uOptions, uint64_t *puValue);

static void QCBORDecode_GetUInt64ConvertInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, uint64_t *puValue);

static void QCBORDecode_GetUInt64ConvertInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, uint64_t *puValue);

/**
 @brief Decode next item as an unsigned 64-bit integer with conversions

 @param[in] pCtx   The decode context
 @param[out] puValue  64-bit integer with item

 The sames as QCBORDecode_GetInt64ConvertAll(), but returns an unsigned integer and thus
 sets @ref QCBOR_ERR_NUMBER_SIGN_CONVERSION
 if the value to be decoded is negatve.

 See also QCBORDecode_GetUInt64Convert() and QCBORDecode_GetUInt64ConvertAll().
*/
void QCBORDecode_GetUInt64ConvertAll(QCBORDecodeContext *pCtx, uint32_t uOptions, uint64_t *puValue);

void QCBORDecode_GetUInt64ConvertAllInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, uint64_t *puValue);

void QCBORDecode_GetUInt64ConvertAllInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, uint64_t *puValue);


/**
 @brief Decode next item as a floating-point value.

 @param[in] pCtx   The decode context
 @param[out] pValue  The returned floating-point value.

 On error, the decoder internal error state is set.

 The CBOR data item to decode must be a hafl-precision, single-precision
 or double-precision floating-point value.  If not
 @ref QCBOR_ERR_UNEXPECTED_TYPE is set.

 See also QCBORDecode_GetDoubleConvert() and QCBORDecode_GetDoubleConvertAll().
*/
static void QCBORDecode_GetDouble(QCBORDecodeContext *pCtx, double *pValue);

static void QCBORDecode_GetDoubleInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, double *pdValue);

static void QCBORDecode_GetDoubleInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, double *pdValue);

/**
 @brief Decode next item as a floating-point value with basic conversion.

 @param[in] pCtx   The decode context
 @param[out] pValue  The returned floating-point value.

 On error, the decoder internal error state is set.

 The CBOR data item to decode must be a hafl-precision, single-precision
 or double-precision floating-point value or a positive or negative integer.  If not
 @ref QCBOR_ERR_UNEXPECTED_TYPE is set.

 Positive and negative integers can always be converted to floating-point,
 so this always succeeds.

 Note that a large 64-bit integer can have more precision than even a
 double floating-point value, so there is loss of precision in some conversions.

 See also QCBORDecode_GetDouble() and QCBORDecode_GetDoubleConvertAll().
*/
static void QCBORDecode_GetDoubleConvert(QCBORDecodeContext *pCtx, uint32_t uOptions, double *pValue);

static void QCBORDecode_GetDoubleConvertInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, double *pdValue);

static void QCBORDecode_GetDoubleConvertInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, double *pdValue);


/**
 @brief Decode next item as a floating-point value with conversion.

 @param[in] pCtx   The decode context
 @param[out] pValue  The returned floating-point value.

 On error, the decoder internal error state is set.

 In addition to conversions supported by QCBORDecode_GetDoubleConvert(),
 conversion from positive and negative bignums, decimal fractions and big floats
 are supported.

 Big numbers, decimal fractions and big floats that are too small or too large
 to be reprented as a floating-point number will be returned as plus or minus
 zero or infinity. There is also often loss of precision in the conversion.

 See also QCBORDecode_GetDoubleConvert() and QCBORDecode_GetDoubleConvert().
*/
void QCBORDecode_GetDoubleConvertAll(QCBORDecodeContext *pCtx, uint32_t uOptions, double *pValue);

void QCBORDecode_GetDoubleConvertAllInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, uint32_t uOptions, double *puValue);

void QCBORDecode_GetDoubleConvertAllInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, uint32_t uOptions, double *puValue);



/**
 @brief Decode the next item as a byte string

 @param[in] pCtx   The decode context
 @param[out] pBytes  The decoded byte string

 On error, the decoder internal error state is set. If the next item
 is not a byte string, the @ref QCBOR_ERR_UNEXPECTED_TYPE error is set.
 */
static void QCBORDecode_GetBytes(QCBORDecodeContext *pCtx, UsefulBufC *pBytes);

static void QCBORDecode_GetBytesInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, UsefulBufC *pBytes);

static void QCBORDecode_GetBytesInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, UsefulBufC *pBytes);



static void QCBORDecode_GetText(QCBORDecodeContext *pCtx, UsefulBufC *pText);

static void QCBORDecode_GetTextInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, UsefulBufC *pText);

static void QCBORDecode_GetTextInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, UsefulBufC *pText);


void QCBORDecode_GetBool(QCBORDecodeContext *pCtx, bool *pbBool);

void QCBORDecode_GetBoolInMapN(QCBORDecodeContext *pCtx, int64_t nLabel, bool *pbBool);

void QCBORDecode_GetBoolInMapSZ(QCBORDecodeContext *pCtx, const char *szLabel, bool *pbBool);



/*
@brief Decode the next item as a date string

@param[in] pCtx             The decode context.
@param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
@param[out] pURI            The decoded URI.

Error handling is like QCBORDecode_GetBytes().

See XYZ for discussion on tag requirements.
*/
static void QCBORDecode_GetDateString(QCBORDecodeContext *pCtx, uint8_t uTagRequired, UsefulBufC *pValue);

static void QCBORDecode_GetDateStringInMapN(QCBORDecodeContext *pCtx, uint8_t uTagRequired, int64_t nLabel, UsefulBufC *pValue);

static void QCBORDecode_GetDateStringInMapSZ(QCBORDecodeContext *pCtx,
                                        uint8_t uTagRequired,
                                        const char *szLabel,
                                        UsefulBufC *pValue);



/**
 @brief Decode the next item as an epoch date.

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] puTime            The decoded URI.

 Error handling is like QCBORDecode_GetBytes().

 See XYZ for discussion on tag requirements.
*/
void QCBORDecode_GetEpocDate(QCBORDecodeContext *pCtx, uint8_t uTagRequirement, int64_t *puTime);

static void QCBORDecode_GetEpochDateInMapN(QCBORDecodeContext *pCtx, uint8_t uTagRequirement, int64_t nLabel, int64_t *puTime);

void QCBORDecode_GetEpochDateInMapSZ(QCBORDecodeContext *pCtx, uint8_t uTagRequirement, const char *szLabel, int64_t *puTime);


/*
 @brief Decode the next item as a big number.

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pValue          The returned big number.
 @param[out] pbIsNegative    Is @c true if the big number is negative. This
                             is only valid when @c uTagRequirement is
                             @ref QCBOR_TAGSPEC_MATCH_TAG.

 Error handling is like QCBORDecode_GetBytes().

 The big number is in network byte order. The first byte in
 @c pValue is the most significant byte. There may be leading
 zeros.

 The negative value is computed as -1 - n, where n is the
 postive big number in @C pValue.

 See XYZ for discussion on tag requirements.

 Determination of the sign of the big number depends on the
 tag requirement of the protocol using the big number. If the
 protocol requires tagging, @ref QCBOR_TAGSPEC_MATCH_TAG,
 then the sign indication is in the protocol and @c pbIsNegative
 indicates the sign. If the protocol prohibits tagging,
 @ref QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE, then the
 protocol design must have some way of indicating the sign.
*/
void QCBORDecode_GetBignum(QCBORDecodeContext *pCtx,
                           uint8_t             uTagRequirement,
                           UsefulBufC         *pValue,
                           bool               *pbIsNegative);

void QCBORDecode_GetBignumInMapN(QCBORDecodeContext *pCtx,
                                 int64_t             nLabel,
                                 uint8_t             uTagRequirement,
                                 UsefulBufC         *pValue,
                                 bool               *pbIsNegative);

void QCBORDecode_GetBignumInMapSz(QCBORDecodeContext *pCtx,
                                  const char         *szLabel,
                                  uint8_t             uTagRequirement,
                                  UsefulBufC         *pValue,
                                  bool               *pbIsNegative);



/**
 @brief Decode the next item as a decimal fraction.

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pnMantissa      The mantissa.
 @param[out] pnExponent      The base 10 exponent.

 Error handling is like QCBORDecode_GetBytes().

 You can compute the  value of this by:

     mantissa * ( 10 ** exponent )

 In the encoded CBOR, the mantissa may be a type 0 (unsigned),
 type 1 (signed integer), type 2 tag xx (positive big number) or
 type 2 tag xx (negative big number). This implementation will attempt
 to convert all of these to an int64_t. If the value won't fit,
 @ref QCBOR_ERR_CONVERSION_UNDER_OVER_FLOW
 or QCBOR_ERR_BAD_EXP_AND_MANTISSA will be
 set.

 The encoded CBOR exponent may be a type 0 (unsigned integer)
 or type 1 (signed integer). This implementation will attempt
 to convert all of these to an int64_t. If the value won't fit,
 @ref QCBOR_ERR_CONVERSION_UNDER_OVER_FLOW
 or QCBOR_ERR_BAD_EXP_AND_MANTISSA will be
 set.

 Various format and type issues will result in
 @ref QCBOR_ERR_BAD_EXP_AND_MANTISSA being set.

 See XYZ for discussion on tag requirements.
*/
void QCBORDecode_GetDecimalFraction(QCBORDecodeContext *pCtx,
                                    uint8_t             uTagRequirement,
                                    int64_t             *pnMantissa,
                                    int64_t             *pnExponent);

void QCBORDecode_GetDecimalFractionInMapN(QCBORDecodeContext *pCtx,
                                          uint8_t             uTagRequirement,
                                          int64_t             nLabel,
                                          int64_t             *pnMantissa,
                                          int64_t             *pnExponent);

void QCBORDecode_GetDecimalFractionInMapSZ(QCBORDecodeContext *pMe,
                                           uint8_t             uTagRequirement,
                                           const char         *szLabel,
                                           int64_t             *pnMantissa,
                                           int64_t             *pnExponent);



/**
 @brief Decode the next item as a decimal fraction with a big number mantissa.

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[in] MantissaBuffer The buffer in which to put the mantissa.
 @param[out] pMantissa      The big num mantissa.
 @param[out] pbMantissaIsNegative  Is @c true if @c pMantissa is negative.
 @param[out] pnExponent      The base 10 exponent.

 Error handling is like QCBORDecode_GetBytes().

 You can compute the  value of this by:

     mantissa * ( 10 ** exponent )

 In the encoded CBOR, the mantissa may be a type 0 (unsigned),
 type 1 (signed integer), type 2 tag xx (positive big number) or
 type 2 tag xx (negative big number). This implementation will
 all these to a big number. The limit to this conversion is the
 size of @c MantissaBuffer.

 The exponent is handled the same as for QCBORDecode_GetDecimalFraction().

 See XYZ for discussion on tag requirements.
*/
void QCBORDecode_GetDecimalFractionBig(QCBORDecodeContext *pCtx,
                                       uint8_t             uTagRequirement,
                                       UsefulBuf           MantissaBuffer,
                                       UsefulBufC         *pMantissa,
                                       bool               *pbMantissaIsNegative,
                                       int64_t            *pnExponent);

void QCBORDecode_GetDecimalFractionBigInMapN(QCBORDecodeContext *pCtx,
                                             uint8_t             uTagRequirement,
                                             int64_t             nLabel,
                                             UsefulBuf           MantissaBuffer,
                                             UsefulBufC         *pbMantissaIsNegative,
                                             bool               *pbIsNegative,
                                             int64_t            *pnExponent);

void QCBORDecode_GetDecimalFractionBigInMapSZ(QCBORDecodeContext *pCtx,
                                              uint8_t             uTagRequirement,
                                              const char         *szLabel,
                                              UsefulBuf           MantissaBuffer,
                                              UsefulBufC         *pMantissa,
                                              bool               *pbMantissaIsNegative,
                                              int64_t            *pnExponent);



// TODO: finish these
void QCBORDecode_GetBigFloat(QCBORDecodeContext *pCtx,
                             uint8_t             uTagRequirement,
                             int64_t            *pnMantissa,
                             int64_t            *pnExponent);

void QCBORDecode_GetBigFloatInMapN(QCBORDecodeContext *pCtx,
                                   uint8_t             uTagRequirement,
                                   int64_t             nLabel,
                                   int64_t            *pnMantissa,
                                   int64_t            *pnExponent);

void QCBORDecode_GetBigFloatInMapSZ(QCBORDecodeContext *pCtx,
                                    uint8_t             uTagRequirement,
                                    const char         *szLabel,
                                    int64_t            *pnMantissa,
                                    int64_t            *pnExponent);


// TODO: finish these
void QCBORDecode_GetBigFloatBig(QCBORDecodeContext *pCtx,
                                uint8_t             uTagRequirement,
                                UsefulBuf           MantissaBuffer,
                                UsefulBufC         *pMantissa,
                                bool               *pbMantissaIsNegative,
                                int64_t            *pnExponent);

void QCBORDecode_GetBigFloatBigInMapN(QCBORDecodeContext *pCtx,
                                      uint8_t             uTagRequirement,
                                      int64_t             nLabel,
                                      UsefulBuf           MantissaBuffer,
                                      UsefulBufC         *pMantissa,
                                      bool               *pbMantissaIsNegative,
                                      int64_t            *pnExponent);

void QCBORDecode_GetBigFloatBigInMapSZ(QCBORDecodeContext *pCtx,
                                       uint8_t             uTagRequirement,
                                       const char         *szLabel,
                                       UsefulBuf           MantissaBuffer,
                                       UsefulBufC         *pMantissa,
                                       bool               *pbMantissaIsNegative,
                                       int64_t            *pnExponent);


/*
 @brief Decode the next item as a URI.
 
 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pURI            The decoded URI.
 
 Error handling is like QCBORDecode_GetBytes().
 
 See XYZ for discussion on tag requirements.
 */
static void QCBORDecode_GetURI(QCBORDecodeContext *pCtx,
                               uint8_t             uTagRequirement,
                               UsefulBufC         *pURI);

static void QCBORDecode_GetURIInMapN(QCBORDecodeContext *pCtx,
                                     uint8_t             uTagRequirement,
                                     int64_t             nLabel,
                                     UsefulBufC         *pURI);

static void QCBORDecode_GetURIInMapSZ(QCBORDecodeContext *pCtx,
                                      uint8_t             uTagRequirement,
                                      const char *        szLabel,
                                      UsefulBufC         *pURI);


/*
 @brief Decode the next item as base64 encoded text.

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pRegex          The decoded base64 text.

 Error handling is like QCBORDecode_GetBytes().
 
 See XYZ for discussion on tag requirements.
 
 Note that this doesn not actually remove the base64 encoding.
*/
static void QCBORDecode_GetB64(QCBORDecodeContext *pCtx,
                               uint8_t             uTagRequirement,
                               UsefulBufC         *pB64Text);

static void QCBORDecode_GetB64InMapN(QCBORDecodeContext *pCtx,
                                     uint8_t             uTagRequirement,
                                     int64_t             nLabel,
                                     UsefulBufC         *pB64Text);

static void QCBORDecode_GetB64InMapSZ(QCBORDecodeContext *pCtx,
                                      uint8_t             uTagRequirement,
                                      const char         *szLabel,
                                      UsefulBufC         *pB64Text);

// TODO: docoment these
static void QCBORDecode_GetB64URL(QCBORDecodeContext *pCtx,
                                  uint8_t             uTagRequirement,
                                  UsefulBufC         *pB64Text);

static void QCBORDecode_GetB64URLInMapN(QCBORDecodeContext *pCtx,
                                        uint8_t             uTagRequirement,
                                        int64_t             nLabel,
                                        UsefulBufC         *pB64Text);

static void QCBORDecode_GetB64URLInMapSZ(QCBORDecodeContext *pCtx,
                                         uint8_t             uTagRequirement,
                                         const char         *szLabel,
                                         UsefulBufC         *pB64Text);

/*
 @brief Decode the next item as a regular expression.
 
 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pRegex          The decoded regular expression.
 
 Error handling is like QCBORDecode_GetBytes().
 
 See XYZ for discussion on tag requirements.
 */
static void QCBORDecode_GetRegex(QCBORDecodeContext *pCtx,
                                 uint8_t             uTagRequirement,
                                 UsefulBufC         *pRegex);

static void QCBORDecode_GetRegexInMapN(QCBORDecodeContext *pCtx,
                                       uint8_t             uTagRequirement,
                                       int64_t             nLabel,
                                       UsefulBufC         *pRegex);

static void QCBORDecode_GetRegexInMapSZ(QCBORDecodeContext *pCtx,
                                        uint8_t             uTagRequirement,
                                        const char *        szLabel,
                                        UsefulBufC         *pRegex);


/*
 @brief Decode the next item as a MIME message

 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pMessage        The decoded regular expression.
 @param[out] pbIsNot7Bit     @c true if MIME is binary or 8-bit.

 Error handling is like QCBORDecode_GetBytes().

 See XYZ for discussion on tag requirements.
 
 The MIME message itself is not parsed.
 
 This decodes both tag 36 and 257. If it is tag 257, pbIsNot7Bit
 is @c true. While it is clear that tag 36 can't contain,
 binary or 8-bit MIME, it is probably legal for tag 257
 to contain 7-bit MIME. Hopefully in most uses the
 Content-Transfer-Encoding header is present and the
 contents of pbIsNot7Bit can be ignored. It may be NULL.
*/
static void QCBORDecode_GetMIMEMessage(QCBORDecodeContext *pMe,
                                       uint8_t uTagRequirement,
                                       UsefulBufC *pMessage,
                                       bool *pbIsNot7Bit);

static void QCBORDecode_GetMIMEMessageInMapN(QCBORDecodeContext *pMe,
                                            int64_t             nLabel,
                                            uint8_t             uTagRequirement,
                                            UsefulBufC         *pMessage,
                                            bool               *pbIsNot7Bit);


static void QCBORDecode_GetMIMEMessageInMapSZ(QCBORDecodeContext *pMe,
                                              const char         *szLabel,
                                              uint8_t             uTagRequirement,
                                              UsefulBufC         *pMessage,
                                              bool               *pbIsNot7Bit);

/*
 @brief Decode the next item as a UUID
 
 @param[in] pCtx             The decode context.
 @param[in] uTagRequirement  One of @c QCBOR_TAGSPEC_MATCH_XXX.
 @param[out] pUUID            The decoded UUID
 
 Error handling is like QCBORDecode_GetBytes().
 
 See XYZ for discussion on tag requirements.
 */

static inline void QCBORDecode_GetBinaryUUID(QCBORDecodeContext *pMe,
                                             uint8_t             uTagRequirement,
                                             UsefulBufC         *pUUID);

inline static void QCBORDecode_GetBinaryUUIDInMapN(QCBORDecodeContext *pMe,
                                                   uint8_t             uTagRequirement,
                                                   int64_t             nLabel,
                                                   UsefulBufC         *pUUID);

inline static void QCBORDecode_GetBinaryUUIDInMapSZ(QCBORDecodeContext *pMe,
                                                    uint8_t             uTagRequirement,
                                                    const char         *szLabel,
                                                    UsefulBufC         *pUUID);



/**
 @brief Enter a map for decoding and searching.

 @param[in] pCtx   The decode context

 Next item in the CBOR input must be map or this generates an error.

This puts the decoder in bounded mode which narrows
decoding to the map entered and enables
getting items by label.
 
 Nested maps can be decoded like this by entering
 each map in turn.

  Call QCBORDecode_ExitMap() to exit the current map
 decoding level. When all map decoding layers are exited
 then bounded mode is fully exited.
 
 While in bounded mode, QCBORDecode_GetNext() works as usual on the
 map and the in-order traversal cursor
 is maintained. It starts out at the first item in the map just entered. Attempts to get items off the end of the
 map will give error @ref QCBOR_ERR_NO_MORE_ITEMS rather going to the next
 item after the map as it would when not in bounded
 mode.
 
 TODO: You can rewind the inorder traversal cursor to the
 beginning of the map with RewindMap().
 
 Exiting leaves the pre-order cursor at the
 data item following the last entry in the map or at the end of the input CBOR if there nothing after the map.
 
 Entering and Exiting a map is a way to skip over
 an entire map and its contents. After QCBORDecode_ExitMap(),
 the pre-order traversal cursor will be at the
 first item after the map.

 See also QCBORDecode_EnterArray() and QCBORDecode_EnterBstrWrapped().
 Entering and exiting any nested combination of maps, arrays and bstr-wrapped
 CBOR is supported up to the maximum of @ref QCBOR_MAX_ARRAY_NESTING.
 */
static void QCBORDecode_EnterMap(QCBORDecodeContext *pCtx);

void QCBORDecode_EnterMapFromMapN(QCBORDecodeContext *pCtx, int64_t nLabel);

void QCBORDecode_EnterMapFromMapSZ(QCBORDecodeContext *pCtx, const char *szLabel);

static void QCBORDecode_ExitMap(QCBORDecodeContext *pCtx);


/**
@brief Enter an array for decoding.

@param[in] pCtx   The decode context
*/
static void QCBORDecode_EnterArray(QCBORDecodeContext *pCtx);

void QCBORDecode_EnterArrayFromMapN(QCBORDecodeContext *pMe, int64_t uLabel);

void QCBORDecode_EnterArrayFromMapSZ(QCBORDecodeContext *pMe, const char *szLabel);

static void QCBORDecode_ExitArray(QCBORDecodeContext *pCtx);



/**
 @brief Decode some byte-string wrapped CBOR.

 @param[in] pCtx   The decode context.
 @param[in] uTagRequirement Whether or not the byte string must be tagged.
 @param[out] pBstr  Pointer and length of byte-string wrapped CBOR (optional).

 This is for use on some CBOR that has been wrapped in a
 byte string. There are several ways that this can occur.

 First is tag 24 and tag 63. Tag 24
 wraps a single CBOR data item and 63 a CBOR sequence.
 This implementation doesn't distinguish between the two
 (it would be more code and doesn't seem important).

 The XYZ discussion on the tag requirement applies here
 just the same as any other tag.

 In other cases, CBOR is wrapped in a byte string, but
 it is identified as CBOR by other means. The contents
 of a COSE payload are one example of that. They can
 be identified by the COSE content type, or they can
 be identified as CBOR indirectly by the protocol that
 uses COSE. for example, if a blob of CBOR is identified
 as a CWT, then the COSE payload is CBOR.
 To enter into CBOR of this type use the
 @ref QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE as the \c uTagRequirement argument.

 Note that byte string wrapped CBOR can also be
 decoded by getting the byte string with QCBORDecode_GetItem() or
 QCBORDecode_GetByteString() and feeding it into another
 instance of QCBORDecode. Doing it with this function
 has the advantage of using less memory as another
 instance of QCBORDecode is not necessary.

 When the wrapped CBOR is entered with this function,
 the pre-order traversal and such are bounded to
 the wrapped CBOR. QCBORDecode_ExitBstrWrapped()
 must be called resume processing CBOR outside
 the wrapped CBOR.

 If @c pBstr is not @c NULL the pointer and length of the wrapped
 CBOR will be returned. This is usually not needed, but sometimes
 useful, particularly in the case of verifying signed data like the
 COSE payload. This is usually the pointer and length of the
 data is that is hashed or MACed.
 */
void QCBORDecode_EnterBstrWrapped(QCBORDecodeContext *pCtx,
                                  uint8_t             uTagRequirement,
                                  UsefulBufC         *pBstr);

void QCBORDecode_EnterBstrWrappedFromMapN(QCBORDecodeContext *pCtx,
                                          uint8_t             uTagRequirement,
                                          int64_t             nLabel,
                                          UsefulBufC         *pBstr);

void QCBORDecode_EnterBstrWrappedFromMapSZ(QCBORDecodeContext *pCtx,
                                           uint8_t             uTagRequirement,
                                           const char         *szLabel,
                                           UsefulBufC         *pBstr);

void QCBORDecode_ExitBstrWrapped(QCBORDecodeContext *pCtx);


/*
 TODO: fix this; make it rewind bounded
 Restarts fetching of items in a map to the start of the
 map. This is for GetNext. It has no effect on
 GetByLabel (which always searches from the start).
 */
void QCBORDecode_RewindMap(QCBORDecodeContext *pCtxt);


/**
 @brief Indicate if decoder is in bound mode.
 @param[in] pCtx   The decode context.

 @return true is returned if a map, array or bstr wrapped
 CBOR has been entered. This only returns false
 if all maps, arrays and bst wrapped CBOR levels
 have been exited.
 */
bool QCBORDecode_InBoundedMode(QCBORDecodeContext *pCtx);


/**
 @brief Get an item in map by label and type.

 @param[in] pCtx   The decode context.
 @param[in] nLabel The integer label.
 @param[in] uQcborType  The QCBOR type. One of QCBOR_TYPE_XXX.
 @param[out] pItem  The returned item.

 A map must have been entered to use this. If not @ref xxx is set.

 The map is searched for an item of the requested label and type.
 @ref QCBOR_TYPE_ANY can be given to search for the label without
 matching the type.

 This will always search the entire map. This will always perform
  duplicate label detection, setting @ref QCBOR_ERR_DUPLICATE_LABEL if there is more than
 one occurance of the label being searched for.

 This performs a full decode of every item in the map
 being searched, which involves a full pre-order traversal
 of every item. For  maps with little nesting, this
 is of little consequence, but 

 Get an item out of a map.
 
 Decoding must be in bounded for this to work.
*/
void QCBORDecode_GetItemInMapN(QCBORDecodeContext *pCtx,
                               int64_t             nLabel,
                               uint8_t             uQcborType,
                               QCBORItem          *pItem);

void QCBORDecode_GetItemInMapSZ(QCBORDecodeContext *pCtx,
                                const char         *szLabel,
                                uint8_t             uQcborType,
                                QCBORItem          *pItem);


/**
 @brief Get a group of labeled items all at once from a map

 @param[in] pCtx   The decode context.
 @param[in,out] pItemList  On input the items to search for. On output the returned items.
 
 This gets several labeled items out of a map.
 
 @c pItemList is an array of items terminated by an item
 with @c uLabelType @ref QCBOR_TYPE_NONE.

 On input the labels to search for are in the @c uLabelType and
 label fields in the items in @c pItemList.

 Also on input are the requested QCBOR types in the field @c uDataType.
 To match any type, searching just by label, @c uDataType
 can be @ref QCBOR_TYPE_ANY.

 This is a CPU-efficient way to decode a bunch of items in a map. It
 is more efficient than scanning each individually because the map
 only needs to be traversed once.
 
 If any duplicate labels are detected, this returns @ref QCBOR_ERR_DUPLICATE_LABEL.
 
 This will return maps and arrays that are in the map, but
 provides no way to descend into and decode them. Use
 QCBORDecode_EnterMapinMapN(), QCBORDecode_EnterArrayInMapN()
 and such to descend into and process maps and arrays.
 */
QCBORError QCBORDecode_GetItemsInMap(QCBORDecodeContext *pCtx, QCBORItem *pItemList);


/**
 @brief Per-item callback for map searching.

 @param[in] pCallbackCtx  Pointer to the caller-defined context for the callback
 @param[in] pItem  The item from the map.

 @return  The return value is intended for QCBOR errors, not general protocol decoding
 errors. If this returns other than @ref QCBOR_SUCCESS, the search will stop and
 the value it returns will be set in QCBORDecode_GetItemsInMapWithCallback(). The
 special error, @ref QCBOR_ERR_CALLBACK_FAIL, can be returned to indicate some
 protocol processing error that is not a CBOR error. The specific details of the protocol
  processing error can be returned the call back context.
 */
typedef QCBORError (*QCBORItemCallback)(void *pCallbackCtx, const QCBORItem *pItem);


/**
 @brief Get a group of labeled items all at once from a map with a callback

 @param[in] pCtx   The decode context.
 @param[in,out] pItemList  On input the items to search for. On output the returned items.
 @param[in,out] pCallbackCtx Pointer to a context structure for @ref QCBORItemCallback
 @param[in] pfCB pointer to function of type @ref QCBORItemCallback that is called on unmatched items.

 This searchs a map like QCBORDecode_GetItemsInMap(), but calls a callback on items not
 matched rather than ignoring them. If @c pItemList is empty, the call back will be called
 on every item in the map.

 LIke QCBORDecode_GetItemsInMap(), this only matches and calls back on the items at the
 top level of the map entered. Items in nested maps/arrays skipped over and not candidate for
 matching or the callback.

 See QCBORItemCallback() for error handling.
 */
QCBORError QCBORDecode_GetItemsInMapWithCallback(QCBORDecodeContext *pCtx,
                                                 QCBORItem          *pItemList,
                                                 void               *pCallbackCtx,
                                                 QCBORItemCallback   pfCB);









/*
 Normally decoding is just in-order traversal. You can get next
 of any type, get next of a particular type including conversions.
 
 If the cursor is at a map and you enter it, then you can use
 methods that Get things by label, either numeric or string.
 
 These methods work only at the particular level in the map.
 To go into a map nested in a map call the special method
 to enter a map by label.
 
 When in a map, the GetNext methods work too, but only
 to the end of the map. You can't traverse off the end of the
 map.
 
 You can rewind to the start of the map and traverse it again
 with the MapRestart method.
 
 The exit map method will leave the traversal cursor at the first itme after
 the map.
 
 
  The beginning of each map must be recorded so the scan can be done
 through the whole map.
 
 
 
 
 
 
 
 
 
 
 
 */



/**
 @brief Gets the next item including full list of tags for item.

 @param[in]  pCtx          The decoder context.
 @param[out] pDecodedItem  Holds the CBOR item just decoded.
 @param[in,out] pTagList   On input array to put tags in; on output
 the tags on this item. See
 @ref QCBORTagListOut.

 @return See return values for QCBORDecode_GetNext().

 @retval QCBOR_ERR_TOO_MANY_TAGS  The size of @c pTagList is too small.

 This works the same as QCBORDecode_GetNext() except that it also
 returns the full list of tags for the data item. This function should
 only be needed when parsing CBOR to print it out or convert it to
 some other format. It should not be needed to implement a CBOR-based
 protocol.  See QCBORDecode_GetNext() for the main description of tag
 decoding.

 Tags will be returned here whether or not they are in the built-in or
 caller-configured tag lists.

 CBOR has no upper bound of limit on the number of tags that can be
 associated with a data item though in practice the number of tags on
 an item will usually be small, perhaps less than five. This will
 return @ref QCBOR_ERR_TOO_MANY_TAGS if the array in @c pTagList is
 too small to hold all the tags for the item.

 (This function is separate from QCBORDecode_GetNext() so as to not
 have to make @ref QCBORItem large enough to be able to hold a full
 list of tags. Even a list of five tags would nearly double its size
 because tags can be a @c uint64_t ).
 */
QCBORError QCBORDecode_GetNextWithTags(QCBORDecodeContext *pCtx, QCBORItem *pDecodedItem, QCBORTagListOut *pTagList);





// Semi-private
void QCBORDecode_EnterBoundedMapOrArray(QCBORDecodeContext *pMe, uint8_t uType);


inline static void QCBORDecode_EnterMap(QCBORDecodeContext *pMe) {
   QCBORDecode_EnterBoundedMapOrArray(pMe, QCBOR_TYPE_MAP);
}


inline static void QCBORDecode_EnterArray(QCBORDecodeContext *pMe) {
   QCBORDecode_EnterBoundedMapOrArray(pMe, QCBOR_TYPE_ARRAY);
}

// Semi-private
void QCBORDecode_ExitBoundedMapOrArray(QCBORDecodeContext *pMe, uint8_t uType);


static inline void QCBORDecode_ExitArray(QCBORDecodeContext *pMe)
{
   QCBORDecode_ExitBoundedMapOrArray(pMe, QCBOR_TYPE_ARRAY);
}

static inline void QCBORDecode_ExitMap(QCBORDecodeContext *pMe)
{
   QCBORDecode_ExitBoundedMapOrArray(pMe, QCBOR_TYPE_MAP);
}


// Semi-private
void QCBORDecode_GetInt64ConvertInternal(QCBORDecodeContext *pMe, uint32_t uOptions, int64_t *pnValue, QCBORItem *pItem);

void QCBORDecode_GetInt64ConvertInternalInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, int64_t *pnValue, QCBORItem *pItem);

void QCBORDecode_GetInt64ConvertInternalInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, int64_t *pnValue, QCBORItem *pItem);



inline static void QCBORDecode_GetInt64Convert(QCBORDecodeContext *pMe, uint32_t uOptions, int64_t *pnValue)
{
    QCBORItem Item;
    QCBORDecode_GetInt64ConvertInternal(pMe, uOptions, pnValue, &Item);
}

inline static void QCBORDecode_GetInt64ConvertInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, int64_t *pnValue)
{
   QCBORItem Item;
   QCBORDecode_GetInt64ConvertInternalInMapN(pMe, nLabel, uOptions, pnValue, &Item);
}

inline static void QCBORDecode_GetInt64ConvertInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, int64_t *pnValue)
{
   QCBORItem Item;
   QCBORDecode_GetInt64ConvertInternalInMapSZ(pMe, szLabel, uOptions, pnValue, &Item);
}

inline static void QCBORDecode_GetInt64(QCBORDecodeContext *pMe, int64_t *pnValue)
{
    QCBORDecode_GetInt64Convert(pMe, QCBOR_CONVERT_TYPE_XINT64, pnValue);
}

inline static void QCBORDecode_GetInt64InMapN(QCBORDecodeContext *pMe, int64_t nLabel, int64_t *pnValue)
{
   QCBORDecode_GetInt64ConvertInMapN(pMe, nLabel, QCBOR_CONVERT_TYPE_XINT64, pnValue);
}

inline static void QCBORDecode_GetInt64InMapSZ(QCBORDecodeContext *pMe, const char *szLabel, int64_t *pnValue)
{
   QCBORDecode_GetInt64ConvertInMapSZ(pMe, szLabel, QCBOR_CONVERT_TYPE_XINT64, pnValue);
}






// Semi-private
void QCBORDecode_GetUInt64ConvertInternal(QCBORDecodeContext *pMe, uint32_t uOptions, uint64_t *puValue, QCBORItem *pItem);

void QCBORDecode_GetUInt64ConvertInternalInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, uint64_t *puValue, QCBORItem *pItem);

void QCBORDecode_GetUInt64ConvertInternalInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, uint64_t *puValue, QCBORItem *pItem);



void QCBORDecode_GetUInt64Convert(QCBORDecodeContext *pMe, uint32_t uOptions, uint64_t *puValue)
{
    QCBORItem Item;
    QCBORDecode_GetUInt64ConvertInternal(pMe, uOptions, puValue, &Item);
}

inline static void QCBORDecode_GetUInt64ConvertInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, uint64_t *puValue)
{
   QCBORItem Item;
   QCBORDecode_GetUInt64ConvertInternalInMapN(pMe, nLabel, uOptions, puValue, &Item);
}

inline static void QCBORDecode_GetUInt64ConvertInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, uint64_t *puValue)
{
   QCBORItem Item;
   QCBORDecode_GetUInt64ConvertInternalInMapSZ(pMe, szLabel, uOptions, puValue, &Item);
}

static inline void QCBORDecode_GetUInt64(QCBORDecodeContext *pMe, uint64_t *puValue)
{
    QCBORDecode_GetUInt64Convert(pMe, QCBOR_CONVERT_TYPE_XINT64, puValue);
}


inline static void QCBORDecode_GetUInt64InMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint64_t *puValue)
{
   QCBORDecode_GetUInt64ConvertInMapN(pMe, nLabel, QCBOR_CONVERT_TYPE_XINT64, puValue);
}

inline static void QCBORDecode_GetUInt64InMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint64_t *puValue)
{
   QCBORDecode_GetUInt64ConvertInMapSZ(pMe, szLabel, QCBOR_CONVERT_TYPE_XINT64, puValue);
}




void QCBORDecode_GetDoubleConvertInternal(QCBORDecodeContext *pMe, uint32_t uOptions, double *pValue, QCBORItem *pItem);

void QCBORDecode_GetDoubleConvertInternalInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, double *pdValue, QCBORItem *pItem);

void QCBORDecode_GetDoubleConvertInternalInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, double *pdValue, QCBORItem *pItem);


inline static void QCBORDecode_GetDoubleConvert(QCBORDecodeContext *pMe, uint32_t uOptions, double *pValue)
{
    QCBORItem Item;
    QCBORDecode_GetDoubleConvertInternal(pMe, uOptions, pValue, &Item);
}

inline static void QCBORDecode_GetDoubleConvertInMapN(QCBORDecodeContext *pMe, int64_t nLabel, uint32_t uOptions, double *pdValue)
{
   QCBORItem Item;
   QCBORDecode_GetDoubleConvertInternalInMapN(pMe, nLabel, uOptions, pdValue, &Item);
}

inline static void QCBORDecode_GetDoubleConvertInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, uint32_t uOptions, double *pdValue)
{
   QCBORItem Item;
   QCBORDecode_GetDoubleConvertInternalInMapSZ(pMe, szLabel, uOptions, pdValue, &Item);
}

inline static void QCBORDecode_GetDouble(QCBORDecodeContext *pMe, double *pValue)
{
    QCBORDecode_GetDoubleConvert(pMe, QCBOR_CONVERT_TYPE_FLOAT, pValue);
}

inline static void QCBORDecode_GetDoubleInMapN(QCBORDecodeContext *pMe, int64_t nLabel, double *pdValue)
{
   QCBORDecode_GetDoubleConvertInMapN(pMe, nLabel, QCBOR_CONVERT_TYPE_FLOAT, pdValue);
}

inline static void QCBORDecode_GetDoubleInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, double *pdValue)
{
   QCBORDecode_GetDoubleConvertInMapSZ(pMe, szLabel, QCBOR_CONVERT_TYPE_FLOAT, pdValue);
}

// Semi private

#define QCBOR_TAGSPEC_MATCH_TAG 0
#define QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE 1 // When the tag type is known from the context of the protocol
#define QCBOR_TAGSPEC_MATCH_EITHER 2 // CBOR protocols that need this are designed against recommended tag use !!

#define QCBOR_TAGSPEC_NUM_TYPES 3
/* This structure can probably be rearrange so the initialization
 of it takes much less code. */
typedef struct {
   /* One of QCBOR_TAGSPEC_MATCH_xxx */
   uint8_t uTagRequirement;
   /* The tagged type translated into QCBOR_TYPE_XXX. Used to match explicit tagging */
   uint8_t uTaggedTypes[QCBOR_TAGSPEC_NUM_TYPES];
   /* The types of the content, which are used to match implicit tagging */
   uint8_t uAllowedContentTypes[QCBOR_TAGSPEC_NUM_TYPES];
} TagSpecification;

// Semi private

void QCBORDecode_GetTaggedStringInternal(QCBORDecodeContext *pMe, TagSpecification TagSpec, UsefulBufC *pBstr);



// Semi private

void QCBORDecode_GetTaggedItemInMapN(QCBORDecodeContext *pMe,
                                     int64_t             nLabel,
                                     TagSpecification    TagSpec,
                                     QCBORItem          *pItem);

void QCBORDecode_GetTaggedItemInMapSZ(QCBORDecodeContext *pMe,
                                      const char *        szLabel,
                                      TagSpecification    TagSpec,
                                      QCBORItem          *pItem);

void QCBORDecode_GetTaggedStringInMapN(QCBORDecodeContext *pMe,
                                       int64_t             nLabel,
                                       TagSpecification    TagSpec,
                                       UsefulBufC          *pString);

void QCBORDecode_GetTaggedStringInMapSZ(QCBORDecodeContext *pMe,
                                        const char *        szLabel,
                                        TagSpecification    TagSpec,
                                        UsefulBufC          *pString);

QCBORError FarfMIME(uint8_t uTagRequirement, const QCBORItem *pItem, UsefulBufC *pMessage, bool *pbIsNot7Bit);



static inline void QCBORDecode_GetBytes(QCBORDecodeContext *pMe,  UsefulBufC *pValue)
{
   // Complier should make this just 64-bit integer parameter
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pValue);
}

inline static void QCBORDecode_GetBytesInMapN(QCBORDecodeContext *pMe, int64_t nLabel, UsefulBufC *pBstr)
{
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };
   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pBstr);
}

inline static void QCBORDecode_GetBytesInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, UsefulBufC *pBstr)
{
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pBstr);
}

static inline void QCBORDecode_GetText(QCBORDecodeContext *pMe,  UsefulBufC *pValue)
{
   // Complier should make this just 64-bit integer parameter
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pValue);
}

inline static void QCBORDecode_GetTextInMapN(QCBORDecodeContext *pMe, int64_t nLabel, UsefulBufC *pText)
{
   // This TagSpec only matches text strings; it also should optimize down to passing a 64-bit integer
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };;

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pText);
}


inline static void QCBORDecode_GetTextInMapSZ(QCBORDecodeContext *pMe, const char *szLabel, UsefulBufC *pText)
{
   const TagSpecification TagSpec = {QCBOR_TAGSPEC_MATCH_TAG_CONTENT_TYPE,
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pText);
}


static inline void QCBORDecode_GetDateString(QCBORDecodeContext *pMe, uint8_t uTagRequirement, UsefulBufC *pValue)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_DATE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pValue);
}


inline static void QCBORDecode_GetDateStringInMapN(QCBORDecodeContext *pMe,
                                                   uint8_t uTagRequirement,
                                                   int64_t nLabel,
                                                   UsefulBufC *pText)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_DATE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pText);
}

inline static void QCBORDecode_GetDateStringInMapSZ(QCBORDecodeContext *pMe,
                                                    uint8_t             uTagRequirement,
                                                    const char         *szLabel,
                                                    UsefulBufC         *pText)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_DATE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pText);
}




static inline void QCBORDecode_GetURI(QCBORDecodeContext *pMe,
                                      uint8_t             uTagRequirement,
                                      UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_URI, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pUUID);
}


inline static void QCBORDecode_GetURIInMapN(QCBORDecodeContext *pMe,
                                            uint8_t             uTagRequirement,
                                            int64_t             nLabel,
                                            UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_URI, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pUUID);
}

inline static void QCBORDecode_GetURIInMapSZ(QCBORDecodeContext *pMe,
                                             uint8_t             uTagRequirement,
                                             const char         *szLabel,
                                             UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_URI, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pUUID);
}



static inline void QCBORDecode_GetB64(QCBORDecodeContext *pMe,
                                      uint8_t             uTagRequirement,
                                      UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pB64Text);
}


inline static void QCBORDecode_GetB64InMapN(QCBORDecodeContext *pMe,
                                            uint8_t             uTagRequirement,
                                            int64_t             nLabel,
                                            UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pB64Text);
}

inline static void QCBORDecode_GetB64InMapSZ(QCBORDecodeContext *pMe,
                                             uint8_t             uTagRequirement,
                                             const char         *szLabel,
                                             UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };
   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pB64Text);
}


static inline void QCBORDecode_GetB64URL(QCBORDecodeContext *pMe,
                                      uint8_t             uTagRequirement,
                                      UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64URL, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pB64Text);
}


inline static void QCBORDecode_GetB64URLInMapN(QCBORDecodeContext *pMe,
                                            uint8_t             uTagRequirement,
                                            int64_t             nLabel,
                                            UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64URL, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pB64Text);
}

inline static void QCBORDecode_GetB64URLInMapSZ(QCBORDecodeContext *pMe,
                                             uint8_t             uTagRequirement,
                                             const char         *szLabel,
                                             UsefulBufC         *pB64Text)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_BASE64URL, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };
   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pB64Text);
}


static inline void QCBORDecode_GetRegex(QCBORDecodeContext *pMe,
                                        uint8_t             uTagRequirement,
                                        UsefulBufC         *pRegex)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_REGEX, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pRegex);
}

static inline void QCBORDecode_GetRegexInMapN(QCBORDecodeContext *pMe,
                                              uint8_t             uTagRequirement,
                                              int64_t             nLabel,
                                              UsefulBufC         *pRegex)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_REGEX, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };
   
   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pRegex);
}

static inline void QCBORDecode_GetRegexInMapSZ(QCBORDecodeContext *pMe,
                                               uint8_t             uTagRequirement,
                                               const char *        szLabel,
                                               UsefulBufC         *pRegex)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_REGEX, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_TEXT_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };
   
   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pRegex);
}


static inline void QCBORDecode_GetMIMEMessage(QCBORDecodeContext *pMe,
                                              uint8_t uTagRequirement,
                                              UsefulBufC *pMessage,
                                              bool *pbIsNot7Bit)
{
   if(pMe->uLastError != QCBOR_SUCCESS) {
      // Already in error state, do nothing
      return;
   }

   QCBORItem  Item;
   QCBORError uError = QCBORDecode_GetNext(pMe, &Item);
   if(uError != QCBOR_SUCCESS) {
      pMe->uLastError = (uint8_t)uError;
      return;
   }

   pMe->uLastError = (uint8_t)FarfMIME(uTagRequirement, &Item, pMessage, pbIsNot7Bit);
}

static inline void QCBORDecode_GetMIMEMessageInMapN(QCBORDecodeContext *pMe,
                                      int64_t             nLabel,
                                      uint8_t             uTagRequirement,
                                      UsefulBufC         *pMessage,
                                      bool               *pbIsNot7Bit)
{
   QCBORItem Item;
   QCBORDecode_GetItemInMapN(pMe, nLabel, QCBOR_TYPE_ANY, &Item);

   pMe->uLastError = (uint8_t)FarfMIME(uTagRequirement, &Item, pMessage, pbIsNot7Bit);
}

static inline void QCBORDecode_GetMIMEMessageInMapSZ(QCBORDecodeContext *pMe,
                                       const char         *szLabel,
                                       uint8_t             uTagRequirement,
                                       UsefulBufC         *pMessage,
                                       bool               *pbIsNot7Bit)
{
   QCBORItem Item;
   QCBORDecode_GetItemInMapSZ(pMe, szLabel, QCBOR_TYPE_ANY, &Item);

   pMe->uLastError = (uint8_t)FarfMIME(uTagRequirement, &Item, pMessage, pbIsNot7Bit);
}



static inline void QCBORDecode_GetBinaryUUID(QCBORDecodeContext *pMe,
                                             uint8_t             uTagRequirement,
                                             UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_UUID, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInternal(pMe, TagSpec, pUUID);
}


inline static void QCBORDecode_GetBinaryUUIDInMapN(QCBORDecodeContext *pMe,
                                                   uint8_t             uTagRequirement,
                                                   int64_t             nLabel,
                                                   UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_UUID, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapN(pMe, nLabel, TagSpec, pUUID);
}

inline static void QCBORDecode_GetBinaryUUIDInMapSZ(QCBORDecodeContext *pMe,
                                                    uint8_t             uTagRequirement,
                                                    const char         *szLabel,
                                                    UsefulBufC         *pUUID)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_UUID, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_BYTE_STRING, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE}
                                    };

   QCBORDecode_GetTaggedStringInMapSZ(pMe, szLabel, TagSpec, pUUID);
}


inline static void QCBORDecode_GetEpochDateInMapN(QCBORDecodeContext *pMe,
                                                  uint8_t             uTagRequirement,
                                                  int64_t             nLabel,
                                                  int64_t            *puTime)
{
   const TagSpecification TagSpec = {uTagRequirement,
                                     {QCBOR_TYPE_DATE_EPOCH, QCBOR_TYPE_NONE, QCBOR_TYPE_NONE},
                                     {QCBOR_TYPE_INT64, QCBOR_TYPE_DOUBLE, QCBOR_TYPE_NONE}
                                    };

   QCBORItem Item;
   QCBORDecode_GetTaggedItemInMapN(pMe, nLabel, TagSpec, &Item);
   *puTime = Item.val.int64; // TODO: lots of work to do here to handle the variety of date types
   // This can't stay as an inline function. May have to rewrite date handling
}



#ifdef __cplusplus
}
#endif 

#endif /* qcbor_spiffy_decode_h */
