/************************************************************************/
/**
 * @file LogImp.h
 * @brief Log engine core implementation.
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2006-05-10
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006 GSI GridTeam. All rights reserved.
************************************************************************/
#ifndef CLOGIMP_H
#define CLOGIMP_H

// STD
#include <stdexcept>

// OUR
#include "Log.h"

namespace MiscCommon
{
    /**
     * @brief It is a supporting macro, which declares GetModuleName method. Needed by MiscCommon::CLogImp
     **/
#define REGISTER_LOG_MODULE(name)             std::string GetModuleName() const  {  return name; }

    /**
     * @brief It represents logbook as a singleton.
     * @brief ofstream specialization of CLog
     **/
    class CLogSinglton
    {
            typedef std::auto_ptr<CFileLog> CFileLogPtr;

            CLogSinglton()
            {}
            ~CLogSinglton()
            {}

        public:
            int Init( const std::string &_LogFileName, bool _CreateNew = false )
            {
                if ( m_log.get() )
                    throw std::logic_error("CLogSinglton error: singleton class has been already initialized.");

                m_log = CFileLogPtr( new CFileLog( _LogFileName, _CreateNew ) );
                push( LOG_SEVERITY_INFO, 0, "LOG singleton", "LOG singleton has been initialized." );
                return 0;
            }
            static CLogSinglton &Instance()
            {
                static CLogSinglton log;
                return log;
            }
            CFileLog::stream_type &push( LOG_SEVERITY _Severity, unsigned long _ErrorCode, const std::string &_Module, const std::string &_Message )
            {
                if ( !m_log.get() )
                {
                    return reinterpret_cast<CFileLog::stream_type &>(std::cerr << _Message << std::endl);
                }
                return m_log->push( _Severity, _ErrorCode, _Module, _Message );
            }
            bool IsReady()
            {
                return ( NULL != m_log.get() );
            }

        private:
            CFileLogPtr m_log;
    };

    /**
     * @brief Template class. High-end helper implementation of CLog, its ofstream specialization.
     * @note: a REGISTER_LOG_MODULE(module name) must be be declared in a child class body.
     * an example:
     * @code
     * class CFoo: public MiscCommon::CLogImp<CAgentServer>
     * {
     *   public:
     *          CFoo();
     *          ~CFoo();
     *          REGISTER_LOG_MODULE( Foo );
     * }; 
     * @endcode
     **/ 
    // TODO: Add comment to doxygen about REGISTER_LOG_MODULE(name), which must be declared in a child class
    template <typename _T>
    class CLogImp
    {
        public:
            CLogImp()
            {
                if ( CLogSinglton::Instance().IsReady() )
                    CLogSinglton::Instance().push( LOG_SEVERITY_INFO, 0, g_cszMODULENAME_CORE, "Bringing >>> " + GetModuleName() + " <<< to life..." );
            }
            ~CLogImp()
            {
                CLogSinglton::Instance().push( LOG_SEVERITY_INFO, 0, g_cszMODULENAME_CORE, "Shutting down >>> " + GetModuleName() + " <<<" );
            }
            CFileLog::stream_type &InfoLog( unsigned long _ErrorCode, const std::string &_Message )
            {
                return CLogSinglton::Instance().push( LOG_SEVERITY_INFO, _ErrorCode, GetModuleName(), _Message );
            }
            CFileLog::stream_type &WarningLog( unsigned long _ErrorCode, const std::string &_Message )
            {
                return CLogSinglton::Instance().push( LOG_SEVERITY_WARNING, _ErrorCode, GetModuleName(), _Message );
            }
            CFileLog::stream_type &FaultLog( unsigned long _ErrorCode, const std::string &_Message )
            {
                return CLogSinglton::Instance().push( LOG_SEVERITY_FAULT, _ErrorCode, GetModuleName(), _Message );
            }
            CFileLog::stream_type &CriticalErrLog( unsigned long _ErrorCode, const std::string &_Message )
            {
                return CLogSinglton::Instance().push( LOG_SEVERITY_CRITICAL_ERROR, _ErrorCode, GetModuleName(), _Message );
            }
            CFileLog::stream_type &DebugLog( unsigned long _ErrorCode, const std::string &_Message )
            {
                return CLogSinglton::Instance().push( LOG_SEVERITY_DEBUG, _ErrorCode, GetModuleName(), _Message );
            }

        private:
            std::string GetModuleName()
            {
                _T *pT = reinterpret_cast<_T*>(this);
                return pT->GetModuleName();
            }
    };

};
#endif
