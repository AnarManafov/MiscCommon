/************************************************************************/
/**
 * @file Log.h
 * @brief Log engine core.
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision:1599 $
        created by:         Anar Manafov
                            2006-05-07
        last changed by:    $LastChangedBy:manafov $ $LastChangedDate:2008-01-02 15:12:05 +0100 (Wed, 02 Jan 2008) $

        Copyright (c) 2006 GSI GridTeam. All rights reserved.
************************************************************************/
#ifndef CLOG_H
#define CLOG_H

// API
#include <sys/time.h>
// STD
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
// STL
#include <string>
// MiscCommon
#include "Res.h"
#include "def.h"
#include "SysHelper.h"

namespace MiscCommon
{
    /**
     *
     * @brief Log's severity's constants.
     *
     */
    typedef enum ESeverity
    {
        LOG_SEVERITY_INFO = 0x1,
        LOG_SEVERITY_WARNING = 0x2,
        LOG_SEVERITY_FAULT = 0x4,
        LOG_SEVERITY_CRITICAL_ERROR = 0x8,
        LOG_SEVERITY_DEBUG = 0x10
    }LOG_SEVERITY;
    enum
    {
        e_FieldSeparator = 0x20,
        e_WhiteSpace = 0x20
    };
    /**
     *
     * @brief A simple template class which represents the Log engine of library.
     * @brief Current Log schema:
     * @brief [DATE/TIME]  [SEVERITY]  [MODULE NAME]   [Message]
     *
     */
    template <typename _T>
    class CLog
    {
        public:
            CLog( _T *_stream ) :
                    m_stream( _stream )
            {}

            _T &push( LOG_SEVERITY _Severity, unsigned long _ErrorCode, const std::string &_Module, const std::string &_Message )
            {
                // Thread ID
                pid_t tid = gettid();
                std::string _DTBuff;
                std::stringstream strMsg;
                strMsg
                << GetCurTimeString( &_DTBuff ) << char( e_FieldSeparator )
                << GetSeverityString( _Severity ) << char( e_FieldSeparator )
                << GetErrorCode( _ErrorCode ) << char( e_FieldSeparator )
                << "[" << _Module << ":thread-" << tid << "]" << char( e_FieldSeparator )
                << _Message;

                smart_mutex m( m_mutex );
                if ( m_stream && m_stream->good() )
                {
                    *m_stream << strMsg.str() << std::endl;
                    m_stream->flush();
                }
                else
                {
                    std::cout << strMsg.str() << std::endl;
                }
                return *m_stream;
            }

        private:
            std::string &GetCurTimeString( std::string *_Buf )
            {
                if ( !_Buf )
                    return * _Buf;

                // Obtain the time of day, and convert it to a tm struct.
                timeval tv;
                gettimeofday (&tv, NULL);
                tm *tm_now( localtime( &tv.tv_sec ) );
                CHARVector_t buff( LOG_DATETIME_BUFF_LEN );
                strftime ( &buff[ 0 ], sizeof( char ) * LOG_DATETIME_BUFF_LEN, g_cszLOG_DATETIME_FRMT, tm_now );
                const long milliseconds = tv.tv_usec / 1000;
                *_Buf = &buff[ 0 ];
                std::stringstream ss;
                ss << "." << milliseconds;
                *_Buf += ss.str();
                return *_Buf;
            }

            const std::string GetSeverityString( LOG_SEVERITY _Severity ) const
            {
                switch ( _Severity )
                {
                    case LOG_SEVERITY_INFO:
                        return g_cszLOG_SEVERITY_INFO;
                    case LOG_SEVERITY_WARNING:
                        return g_cszLOG_SEVERITY_WARNING;
                    case LOG_SEVERITY_FAULT:
                        return g_cszLOG_SEVERITY_FAULT;
                    case LOG_SEVERITY_CRITICAL_ERROR:
                        return g_cszLOG_SEVERITY_CRITICAL_ERROR;
                    default:
                        return g_cszLOG_SEVERITY_DEBUG;
                }
            }

            std::string GetErrorCode( unsigned long _ErrorCode ) const
            {
                std::stringstream sErrCode;
                sErrCode << _ErrorCode;
                return sErrCode.str();
            }

        private:
            _T *m_stream;
            CMutex m_mutex;
    };
    /**
     *
     * @brief ostream specialization of CLog.
     *
     */
    typedef CLog<std::ostream> CSTDOutLog;
    /**
     *
     * @brief Logging to a file.
     * @brief ofstream specialization of CLog.
     *
     */
    class CFileLog: public CLog<std::ofstream>
    {
        public:
            typedef std::ofstream stream_type;

        public:
            CFileLog( const std::string &_LogFileName, bool _CreateNew = false ) : CLog<stream_type>( &m_log_file ),
                    m_log_file( _LogFileName.c_str(), ( _CreateNew ? std::ios::trunc : std::ios::app ) | std::ios::out )
            {}

        private:
            stream_type m_log_file;
    };
};
#endif
