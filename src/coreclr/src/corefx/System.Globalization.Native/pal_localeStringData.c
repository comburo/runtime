// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pal_localeStringData.h"

/*
Function:
GetLocaleInfoDecimalFormatSymbol

Obtains the value of a DecimalFormatSymbols
*/
UErrorCode
GetLocaleInfoDecimalFormatSymbol(const char* locale, UNumberFormatSymbol symbol, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    UNumberFormat* pFormat = unum_open(UNUM_DECIMAL, NULL, 0, locale, NULL, &status);

    if (U_FAILURE(status))
        return status;

    unum_getSymbol(pFormat, symbol, value, valueLength, &status);

    unum_close(pFormat);
    return status;
}

/*
Function:
GetDigitSymbol

Obtains the value of a Digit DecimalFormatSymbols
*/
UErrorCode GetDigitSymbol(const char* locale,
                          UErrorCode previousStatus,
                          UNumberFormatSymbol symbol,
                          int digit,
                          UChar* value,
                          int32_t valueLength)
{
    if (U_FAILURE(previousStatus))
    {
        return previousStatus;
    }

    return GetLocaleInfoDecimalFormatSymbol(locale, symbol, value + digit, valueLength - digit);
}

/*
Function:
GetLocaleInfoAmPm

Obtains the value of the AM or PM string for a locale.
*/
UErrorCode GetLocaleInfoAmPm(const char* locale, bool am, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    UDateFormat* pFormat = udat_open(UDAT_DEFAULT, UDAT_DEFAULT, locale, NULL, 0, NULL, 0, &status);

    if (U_FAILURE(status))
        return status;

    udat_getSymbols(pFormat, UDAT_AM_PMS, am ? 0 : 1, value, valueLength, &status);

    udat_close(pFormat);
    return status;
}

/*
Function:
GetLocaleIso639LanguageTwoLetterName

Gets the language name for a locale (via uloc_getLanguage) and converts the result to UChars
*/
UErrorCode GetLocaleIso639LanguageTwoLetterName(const char* locale, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t length = uloc_getLanguage(locale, NULL, 0, &status) + 1;

    char* buf = calloc(length, sizeof(char));
    if (buf == NULL)
    {
        return U_MEMORY_ALLOCATION_ERROR;
    }

    status = U_ZERO_ERROR;

    uloc_getLanguage(locale, buf, length, &status);

    if (U_SUCCESS(status))
    {
        status = u_charsToUChars_safe(buf, value, valueLength);
    }

    free(buf);
    return status;
}

/*
Function:
GetLocaleIso639LanguageThreeLetterName

Gets the language name for a locale (via uloc_getISO3Language) and converts the result to UChars
*/
UErrorCode GetLocaleIso639LanguageThreeLetterName(const char* locale, UChar* value, int32_t valueLength)
{
    const char *isoLanguage = uloc_getISO3Language(locale);
    if (isoLanguage[0] == 0)
    {
        return U_ILLEGAL_ARGUMENT_ERROR;
    }

    return u_charsToUChars_safe(isoLanguage, value, valueLength);
}

/*
Function:
GetLocaleIso3166CountryName

Gets the country name for a locale (via uloc_getCountry) and converts the result to UChars
*/
UErrorCode GetLocaleIso3166CountryName(const char* locale, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t length = uloc_getCountry(locale, NULL, 0, &status) + 1;

    char* buf = calloc(length, sizeof(char));
    if (buf == NULL)
    {
        return U_MEMORY_ALLOCATION_ERROR;
    }

    status = U_ZERO_ERROR;

    uloc_getCountry(locale, buf, length, &status);

    if (U_SUCCESS(status))
    {
        status = u_charsToUChars_safe(buf, value, valueLength);
    }

    free(buf);

    return status;
}

/*
Function:
GetLocaleIso3166CountryCode

Gets the 3 letter country code for a locale (via uloc_getISO3Country) and converts the result to UChars
*/
UErrorCode GetLocaleIso3166CountryCode(const char* locale, UChar* value, int32_t valueLength)
{
    const char *pIsoCountryName = uloc_getISO3Country(locale);
    int len = strlen(pIsoCountryName);

    if (len == 0)
    {
        return U_ILLEGAL_ARGUMENT_ERROR;
    }

    return u_charsToUChars_safe(pIsoCountryName, value, valueLength);
}

/*
Function:
GetLocaleCurrencyName

Gets the locale currency English or native name and convert the result to UChars
*/
UErrorCode GetLocaleCurrencyName(const char* locale, bool nativeName, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    
    UChar currencyThreeLettersName[4]; // 3 letters currency iso name + NULL
    ucurr_forLocale(locale, currencyThreeLettersName, 4, &status);
    if (!U_SUCCESS(status))
    {
        return status;
    }
    
    int32_t len;
    UBool formatChoice;
    const UChar *pCurrencyLongName = ucurr_getName(
                                        currencyThreeLettersName, 
                                        nativeName ? locale : ULOC_US, 
                                        UCURR_LONG_NAME, 
                                        &formatChoice, 
                                        &len, 
                                        &status);
    if (!U_SUCCESS(status))
    {
        return status;
    }
    
    if (len >= valueLength) // we need to have room for NULL too
    {
        return U_BUFFER_OVERFLOW_ERROR;
    }
    u_strncpy(value, pCurrencyLongName, len);
    value[len] = 0;
    
    return status;
}

/*
PAL Function:
GetLocaleInfoString

Obtains string locale information.
Returns 1 for success, 0 otherwise
*/
int32_t GlobalizationNative_GetLocaleInfoString(
    const UChar* localeName, LocaleStringData localeStringData, UChar* value, int32_t valueLength)
{
    UErrorCode status = U_ZERO_ERROR;
    char locale[ULOC_FULLNAME_CAPACITY];
    GetLocale(localeName, locale, ULOC_FULLNAME_CAPACITY, false, &status);

    if (U_FAILURE(status))
    {
        return UErrorCodeToBool(U_ILLEGAL_ARGUMENT_ERROR);
    }

    switch (localeStringData)
    {
        case LocalizedDisplayName:
            uloc_getDisplayName(locale, DetectDefaultLocaleName(), value, valueLength, &status);
            break;
        case EnglishDisplayName:
            uloc_getDisplayName(locale, ULOC_ENGLISH, value, valueLength, &status);
            break;
        case NativeDisplayName:
            uloc_getDisplayName(locale, locale, value, valueLength, &status);
            break;
        case LocalizedLanguageName:
            uloc_getDisplayLanguage(locale, DetectDefaultLocaleName(), value, valueLength, &status);
            break;
        case EnglishLanguageName:
            uloc_getDisplayLanguage(locale, ULOC_ENGLISH, value, valueLength, &status);
            break;
        case NativeLanguageName:
            uloc_getDisplayLanguage(locale, locale, value, valueLength, &status);
            break;
        case EnglishCountryName:
            uloc_getDisplayCountry(locale, ULOC_ENGLISH, value, valueLength, &status);
            break;
        case NativeCountryName:
            uloc_getDisplayCountry(locale, locale, value, valueLength, &status);
            break;
        case ListSeparator:
        // fall through
        case ThousandSeparator:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_GROUPING_SEPARATOR_SYMBOL, value, valueLength);
            break;
        case DecimalSeparator:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_DECIMAL_SEPARATOR_SYMBOL, value, valueLength);
            break;
        case Digits:
            status = GetDigitSymbol(locale, status, UNUM_ZERO_DIGIT_SYMBOL, 0, value, valueLength);
            // symbols UNUM_ONE_DIGIT to UNUM_NINE_DIGIT are contiguous
            for (int32_t symbol = UNUM_ONE_DIGIT_SYMBOL; symbol <= UNUM_NINE_DIGIT_SYMBOL; symbol++)
            {
                int charIndex = symbol - UNUM_ONE_DIGIT_SYMBOL + 1;
                status = GetDigitSymbol(
                    locale, status, (UNumberFormatSymbol)symbol, charIndex, value, valueLength);
            }
            break;
        case MonetarySymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_CURRENCY_SYMBOL, value, valueLength);
            break;
        case Iso4217MonetarySymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_INTL_CURRENCY_SYMBOL, value, valueLength);
            break;
        case CurrencyEnglishName:
            status = GetLocaleCurrencyName(locale, false, value, valueLength);
            break;
        case CurrencyNativeName:
            status = GetLocaleCurrencyName(locale, true, value, valueLength);
            break;
        case MonetaryDecimalSeparator:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_MONETARY_SEPARATOR_SYMBOL, value, valueLength);
            break;
        case MonetaryThousandSeparator:
            status =
                GetLocaleInfoDecimalFormatSymbol(locale, UNUM_MONETARY_GROUPING_SEPARATOR_SYMBOL, value, valueLength);
            break;
        case AMDesignator:
            status = GetLocaleInfoAmPm(locale, true, value, valueLength);
            break;
        case PMDesignator:
            status = GetLocaleInfoAmPm(locale, false, value, valueLength);
            break;
        case PositiveSign:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_PLUS_SIGN_SYMBOL, value, valueLength);
            break;
        case NegativeSign:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_MINUS_SIGN_SYMBOL, value, valueLength);
            break;
        case Iso639LanguageTwoLetterName:
            status = GetLocaleIso639LanguageTwoLetterName(locale, value, valueLength);
            break;
        case Iso639LanguageThreeLetterName:
            status = GetLocaleIso639LanguageThreeLetterName(locale, value, valueLength);
            break;
        case Iso3166CountryName:
            status = GetLocaleIso3166CountryName(locale, value, valueLength);
            break;
        case Iso3166CountryName2:
            status = GetLocaleIso3166CountryCode(locale, value, valueLength);
            break;
        case NaNSymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_NAN_SYMBOL, value, valueLength);
            break;
        case PositiveInfinitySymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_INFINITY_SYMBOL, value, valueLength);
            break;
        case ParentName:
        {
            // ICU supports lang[-script][-region][-variant] so up to 4 parents
            // including invariant locale
            char localeNameTemp[ULOC_FULLNAME_CAPACITY];

            uloc_getParent(locale, localeNameTemp, ULOC_FULLNAME_CAPACITY, &status);
            if (U_SUCCESS(status))
            {
                status = u_charsToUChars_safe(localeNameTemp, value, valueLength);
                if (U_SUCCESS(status))
                {
                    FixupLocaleName(value, valueLength);
                }
            }
            break;
        }
        case PercentSymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_PERCENT_SYMBOL, value, valueLength);
            break;
        case PerMilleSymbol:
            status = GetLocaleInfoDecimalFormatSymbol(locale, UNUM_PERMILL_SYMBOL, value, valueLength);
            break;
        default:
            status = U_UNSUPPORTED_ERROR;
            break;
    };

    return UErrorCodeToBool(status);
}

/*
PAL Function:
GetLocaleTimeFormat

Obtains time format information (in ICU format, it needs to be coverted to .NET Format).
Returns 1 for success, 0 otherwise
*/
int32_t GlobalizationNative_GetLocaleTimeFormat(
    const UChar* localeName, int shortFormat, UChar* value, int32_t valueLength)
{
    UErrorCode err = U_ZERO_ERROR;
    char locale[ULOC_FULLNAME_CAPACITY];
    GetLocale(localeName, locale, ULOC_FULLNAME_CAPACITY, false, &err);

    if (U_FAILURE(err))
    {
        return UErrorCodeToBool(U_ILLEGAL_ARGUMENT_ERROR);
    }

    UDateFormatStyle style = (shortFormat != 0) ? UDAT_SHORT : UDAT_MEDIUM;
    UDateFormat* pFormat = udat_open(style, UDAT_NONE, locale, NULL, 0, NULL, 0, &err);

    if (U_FAILURE(err))
        return UErrorCodeToBool(err);

    udat_toPattern(pFormat, false, value, valueLength, &err);

    udat_close(pFormat);
    return UErrorCodeToBool(err);
}
