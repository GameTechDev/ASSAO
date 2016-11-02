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

#pragma once

#include "vaCore.h"

#include "vaSTL.h"

namespace VertexAsylum
{
   struct vaCmdLineParam
   {
      wstring        Switch;
      wstring        Value;
   };

   class vaStringTools
   {
   public:
      static wstring            Format( const wchar_t * fmtString, ... );
      static string             Format( const char * fmtString, ... );
                                             
      static wstring            Format( const wchar_t * fmtString, va_list args );
      static string             Format( const char * fmtString, va_list args );

      static wstring            FormatArray( const int intArr[], int count );
      static wstring            FormatArray( const float intArr[], int count );

      static wstring            SimpleWiden( const string & s );
      static string             SimpleNarrow( const wstring & s );

      static wstring            ToLower( const wstring & str );
      static string             ToLower( const string & str );

      static wstring            ToUpper( const wstring & str );
      static string             ToUpper( const string & str );

      static int                CompareNoCase( const wstring & left, const wstring & right );

      static void               SplitPath( const string & inFullPath, string * outDirectory, string * outFileName, string * outFileExt );
      static void               SplitPath( const wstring & inFullPath, wstring * outDirectory, wstring * outFileName, wstring * outFileExt );

      static std::vector<vaCmdLineParam> 
                                SplitCmdLineParams( const wstring & cmdLine );

      static wstring            Trim( const wchar_t * inputStr, const wchar_t * trimCharStr );
      static string             Trim( const char * inputStr, const char * trimCharStr );
      //static wstring          TrimLeft( const wchar_t * trimCharsStr );
      //static wstring          TrimRight( const wchar_t * trimCharsStr );

      static std::vector<wstring>
                                Tokenize( const wchar_t * inputStr, const wchar_t * separatorStr, const wchar_t * trimCharsStr = NULL );
      static std::vector<string>
                                Tokenize( const char * inputStr, const char * separatorStr, const char * trimCharsStr = NULL );

      static float              StringToFloat( const char * inputStr );
      static float              StringToFloat( const wchar_t * inputStr );

      static wstring            FromGUID( const GUID & id );

      static void               ReplaceAll( string & inoutStr, const string & searchStr, const string & replaceStr );

      static bool               WriteTextFile( const wstring & filePath, const string & textData );
   };
}