///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File changes (yyyy-mm-dd)
// 2016-09-07: filip.strugar@intel.com: first commit (extracted from VertexAsylum codebase, 2006-2016 by Filip Strugar)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "vaStringTools.h"

#include "System/vaFileStream.h"

#include <stdarg.h>
#include <assert.h>
#include <algorithm>

using namespace VertexAsylum;

#pragma warning (disable : 4996)
#pragma warning (disable : 4995)

wstring vaStringTools::Format(const wchar_t * fmtString, ...)
{
   va_list args;
   va_start(args, fmtString);
   wstring ret = Format( fmtString, args );
   va_end(args);
   return ret;
}

string vaStringTools::Format(const char * fmtString, ...)
{
   va_list args;
   va_start(args, fmtString);
   string ret = Format( fmtString, args );
   va_end(args);
   return ret;
}

wstring vaStringTools::Format(const wchar_t * fmtString, va_list args)
{
   int nBuf;
   wchar_t szBuffer[4096];

   nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer) / sizeof(wchar_t), fmtString, args);
   assert(nBuf < sizeof(szBuffer));//Output truncated as it was > sizeof(szBuffer)

   return wstring(szBuffer);
}

string vaStringTools::Format(const char * fmtString, va_list args)
{
   int nBuf;
   char szBuffer[4096];

   nBuf = _vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(char), fmtString, args);
   assert(nBuf < sizeof(szBuffer));//Output truncated as it was > sizeof(szBuffer)

   return string(szBuffer);
}


wstring vaStringTools::SimpleWiden( const string & s )
{
   wstring ws;
   ws.resize(s.size());
   for( size_t i = 0; i < s.size(); i++ ) ws[i] = s[i];
   return ws;
}

string vaStringTools::SimpleNarrow( const wstring & s )
{
   string ws;
   ws.resize(s.size());
   for( size_t i = 0; i < s.size(); i++ ) ws[i] = (char)s[i];
   return ws;
}

void vaStringTools::SplitPath( const string & inFullPath, string * outDirectory, string * outFileName, string * outFileExt )
{
   char buffDrive[32];
   char buffDir[4096];
   char buffName[4096];
   char buffExt[4096];

   _splitpath_s( inFullPath.c_str(), buffDrive, _countof(buffDrive), 
      buffDir, _countof(buffDir), buffName, _countof(buffName), buffExt, _countof(buffExt) );

   if( outDirectory != NULL ) *outDirectory  = string(buffDrive) + string(buffDir);
   if( outFileName != NULL )  *outFileName   = buffName;
   if( outFileExt != NULL )   *outFileExt    = buffExt;
}

void vaStringTools::SplitPath( const wstring & inFullPath, wstring * outDirectory, wstring * outFileName, wstring * outFileExt )
{
   wchar_t buffDrive[32];
   wchar_t buffDir[4096];
   wchar_t buffName[4096];
   wchar_t buffExt[4096];

   //assert( !((outDirectory != NULL) && ( (outDirectory != outFileName) || (outDirectory != outFileExt) )) );
   //assert( !((outFileName != NULL) && (outFileName != outFileExt)) );

   _wsplitpath_s( inFullPath.c_str(), buffDrive, _countof(buffDrive), 
      buffDir, _countof(buffDir), buffName, _countof(buffName), buffExt, _countof(buffExt) );

   if( outDirectory != NULL ) *outDirectory = wstring(buffDrive) + wstring(buffDir);
   if( outFileName != NULL )  *outFileName = buffName;
   if( outFileExt != NULL )   *outFileExt = buffExt;
}


// probably better trim version
//static inline cString & TrimSpacesLeft(cString &s)
//{
//    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
//    return s;
//}
//static inline cString & TrimSpacesRight(cString &s)
//{
//    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
//    return s;
//}
//static inline cString & TrimSpaces(cString &s) 
//{
//    return TrimSpacesLeft(TrimSpacesRight(s));
//}
//static std::vector<cString> Tokenize( const cString & inputStr, wchar_t delim = L',' )
//{
//    std::vector<cString> ret;
//    std::wstringstream ss( inputStr );
//    std::wstring item;
//    while(std::getline(ss, item, delim)) 
//    {
//        item = TrimSpaces( item );
//        if( item.length() > 0 )
//            ret.push_back( item );
//    }
//    return ret;
//}


template< class StrTypeChar, class StrType >
static StrType TrimTempl( const StrTypeChar * _inputStr, const StrTypeChar * trimCharsStr )
{
   // if optimizing, see better version above ^
   StrType trimChars = trimCharsStr;
   StrType inputStr = _inputStr;

   int left = 0; int right = (int)inputStr.size()-1;
   for( ; (left <= right) && (trimChars.find(inputStr[left]) != wstring::npos); left++ );
   for( ; (right >= left) && (trimChars.find(inputStr[right]) != wstring::npos); right-- );

   return StrType( &inputStr[left], right-left+1 );
}

template< class StrTypeChar, class StrType >
static std::vector<StrType> TokenizeTempl( const StrTypeChar * _inputStr, const StrTypeChar * separatorStr, const StrTypeChar * trimCharsStr )
{
   std::vector<StrType> ret;

   StrType inputStr = _inputStr;
   
   if( trimCharsStr != NULL )
      inputStr = vaStringTools::Trim( inputStr.c_str(), trimCharsStr );

   wstring::size_type prevSep = 0;
   wstring::size_type nextSep = wstring::npos;
   while( ( nextSep = inputStr.find( separatorStr, prevSep ) ) != wstring::npos )
   {
      StrType subStr = inputStr.substr( prevSep, nextSep - prevSep );
      if( trimCharsStr != NULL )
         subStr = vaStringTools::Trim( subStr.c_str(), trimCharsStr );
      ret.push_back( subStr );
      prevSep = nextSep+1;
   }
   nextSep = inputStr.size();
   if( nextSep > (prevSep-1) )
   {
      StrType subStr = inputStr.substr( prevSep, nextSep - prevSep );
      if( trimCharsStr != NULL )
         subStr = vaStringTools::Trim( subStr.c_str(), trimCharsStr );
      ret.push_back( subStr );
   }

   return ret;
}

wstring vaStringTools::Trim( const wchar_t * inputStr, const wchar_t * trimCharsStr )
{
   return TrimTempl<wchar_t, wstring>( inputStr, trimCharsStr );
}

string vaStringTools::Trim( const char * inputStr, const char * trimCharsStr )
{
   return TrimTempl<char, string>( inputStr, trimCharsStr );
}

std::vector<wstring> vaStringTools::Tokenize( const wchar_t * _inputStr, const wchar_t * separatorStr, const wchar_t * trimCharsStr )
{
   return TokenizeTempl<wchar_t, wstring>( _inputStr, separatorStr, trimCharsStr );
}

std::vector<string> vaStringTools::Tokenize( const char * _inputStr, const char * separatorStr, const char * trimCharsStr )
{
   return TokenizeTempl<char, string>( _inputStr, separatorStr, trimCharsStr );
}

std::vector<vaCmdLineParam> vaStringTools::SplitCmdLineParams( const wstring & cmdLine )
{
   std::vector<vaCmdLineParam> outCmdParams;

   std::vector<wstring> splitStrings;
   wstring line = cmdLine;

   bool inQuotes = false;
   wstring::size_type a = 0;
   for( wstring::size_type i = 0; i < cmdLine.size(); i++ )
   {
      if( cmdLine[i] == L'\"' )
      {
         inQuotes = !inQuotes;
         continue;
      }
      if( inQuotes )
         continue;

      if( cmdLine[i] == L' ' )
      {
         if( i - a == 0 )
         {
            a = i+1;
            continue;
         }
         else
         {
            splitStrings.push_back( cmdLine.substr(a, i-a) );
            a = i+1;
         }
      }
   }

   if( a != line.size() )
      splitStrings.push_back( line.substr(a) );

   for( size_t i = 0; i < splitStrings.size(); i++ )
   {
      if( splitStrings[i][0] == L'\"' ) splitStrings[i] = splitStrings[i].substr(1);
      if( splitStrings[i][splitStrings[i].size()-1] == L'\"' ) splitStrings[i] = splitStrings[i].substr(0, splitStrings[i].size()-1);
   }

   vaCmdLineParam currParam;
   currParam.Switch = L"";
   for( int i = 0; i < (int)splitStrings.size(); i++ )
   {
      if( splitStrings[i][0] == L'-' )
      {
         if( currParam.Switch != L"" )
            outCmdParams.push_back( currParam );
         
         currParam.Switch = splitStrings[i].substr( 1 );
         currParam.Value = L"";
      }
      else
      {
         if( currParam.Value.size() != 0 ) currParam.Value += L" ";
         currParam.Value += splitStrings[i];
      }
   }
   outCmdParams.push_back( currParam );

   return outCmdParams;
}

wstring vaStringTools::FormatArray( const int intArr[], int count )
{
   wstring outStr = L"";

   if( count == 0 )
      return outStr;

   for( int i = 0; i < count-1; i++ )
   {
      outStr += vaStringTools::Format(L"%d: %03d, ", i, intArr[i]);
   }
   outStr += vaStringTools::Format(L"%d: %03d", count-1, intArr[count-1]);
   
   return outStr;
}

wstring vaStringTools::FormatArray( const float intArr[], int count )
{
   wstring outStr = L"";

   if( count == 0 )
      return outStr;

   for( int i = 0; i < count-1; i++ )
   {
      outStr += vaStringTools::Format(L"%f: %03d, ", i, intArr[i]);
   }
   outStr += vaStringTools::Format(L"%f: %03d", count-1, intArr[count-1]);
   
   return outStr;
}

wstring vaStringTools::ToLower( const wstring & str )
{
   wstring ret = str;
   std::transform( ret.begin(), ret.end(), ret.begin(), ::tolower );
   return ret;
}

string  vaStringTools::ToLower( const string & str )
{
   string ret = str;
   std::transform( ret.begin(), ret.end(), ret.begin(), ::tolower );
   return ret;
}

             
wstring vaStringTools::ToUpper( const wstring & str )
{
   wstring ret = str;
   std::transform( ret.begin(), ret.end(), ret.begin(), ::toupper );
   return ret;
}

string  vaStringTools::ToUpper( const string & str )
{
   string ret = str;
   std::transform( ret.begin(), ret.end(), ret.begin(), ::toupper );
   return ret;
}

float vaStringTools::StringToFloat( const char * inputStr )
{
   return (float)atof( inputStr );
}

float vaStringTools::StringToFloat( const wchar_t * inputStr )
{
   string inputStrNarrow = vaStringTools::SimpleNarrow( inputStr );
   return StringToFloat( inputStrNarrow.c_str() );
}

int vaStringTools::CompareNoCase( const wstring & left, const wstring & right )
{
   wstring ul = ToLower( left );
   wstring ur = ToLower( right );

   return ul.compare( ur );
}

void vaStringTools::ReplaceAll( string & inoutStr, const string & searchStr, const string & replaceStr )
{
    size_t searchStrLen = searchStr.length();
    size_t replaceStrLen = replaceStr.length( );
    size_t startPos = 0;
    while( ( startPos = inoutStr.find( searchStr, startPos ) ) != std::string::npos ) 
    {
        inoutStr.replace( startPos, searchStrLen, replaceStr );
        startPos += replaceStrLen;
    }
}

bool vaStringTools::WriteTextFile( const wstring & filePath, const string & textData )
{
    // output results!
    vaFileStream outFile;
    if( outFile.Open( filePath, FileCreationMode::Create ) )
    {
        outFile.WriteTXT( textData );
        return true;
    }
    return false;
}
